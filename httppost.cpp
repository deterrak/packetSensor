#include "httppost.h"
#include <QProcess>
#include <QRegExp>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

httpPost::httpPost()
{
    authenticationNam = new QNetworkAccessManager();
    webhookNam = new QNetworkAccessManager();
}

void httpPost::setAuthenticationURL(QString URL)
{
    authenticationURL.clear();
    authenticationURL.append(URL);
}

void httpPost::setWebHookUrl(QString URL)
{
    webHookURL.clear();
    webHookURL = URL;
}

void httpPost::setDescriptiom(QString Description)
{
    description.clear();
    description.append(Description);
}

void httpPost::packetReceived(QString input)
{
    // this is the SLOT that we will receive the packet cont ents from the tcpdumThread
    qDebug() << "=============================================";
    qDebug() << "httpPost:packetReceived :: received the signal: " << input;
    qDebug() << "=============================================";
    QJsonObject json = parseTcpdumpOutput(input);

    if (!json["error"].toString().contains("error", Qt::CaseInsensitive))
    {
        // there was no error in parsing the packet contents. proceed to trigger the webhook
        qDebug() << "httpPost::packetReceived : sending " << json << " to: " <<  webHookURL;
        this->httpPostToURL(webHookURL, json);
    }
    else
    {
        qDebug() << "parse error ----> not trigggering a webhook...";
    }
}

void httpPost::getAuthenticationToken()
{
    // This SLOT is periodically triggered. It retrives authentication token and saves it in authenticationToken
    QByteArray postData("{\"username\": \"st2admin\", \"password\": \"password\", \"auth_url\": \"https://localhost/auth/v1/tokens\"}");
    postData.clear(); //postData has to be empty it seems... not sure why.

    QNetworkAccessManager *authenticationNam = new QNetworkAccessManager();
    connect(authenticationNam, SIGNAL(finished(QNetworkReply*)), this, SLOT(tokenReplyFinished(QNetworkReply*)));
    connect(authenticationNam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(onSslError(QNetworkReply*, QList<QSslError>)));

    QUrl url;
    url.setUrl(authenticationURL);
    QByteArray userPass("st2admin:password");
    qDebug() << userPass << "base64 is -> " << userPass.toBase64();

    QByteArray auth_array;
    auth_array.append("Basic ");
    auth_array.append(userPass.toBase64());

    QNetworkRequest netRequest(url);
    netRequest.attribute(QNetworkRequest::FollowRedirectsAttribute);
    netRequest.setRawHeader("Authorization", auth_array);
    netRequest.setHeader(QNetworkRequest::UserAgentHeader, "python-openstackclient");
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "*/*");

    reply = authenticationNam->post(netRequest,postData); //get the authentication token
}

void httpPost::httpPostToURL(QString WFCURL, QJsonObject postPayload)
{
    qDebug() << "httpPost::httpPostToURL : " << postPayload;

    QNetworkAccessManager *webhookNam = new QNetworkAccessManager();
    // goto tokenReplyFinished
    connect(webhookNam, SIGNAL(finished(QNetworkReply*)), this, SLOT(webhookReplyFinished(QNetworkReply*)));
    connect(webhookNam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(onSslError(QNetworkReply*, QList<QSslError>)));

    // we will post the JSON structure of the packet captured to the workflow composer
    QJsonDocument doc(postPayload);
    QByteArray postData;
    postData.append(doc.toJson());

    qDebug() << authenticationNam->supportedSchemes();

    QUrl url;
    url.setUrl(WFCURL);
    qDebug() << "httpPost::httpPostToURL : URL is => " << url.toString();

    QNetworkRequest URL(url);
    QByteArray x_auth_token("X-Auth-Token");

    URL.setRawHeader(x_auth_token, authenticationToken);
    URL.setHeader(QNetworkRequest::UserAgentHeader, "User-Agent: python-openstackclient");
    URL.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    URL.setRawHeader("Accept", "application/json");

    qDebug() << "httpPost::httpPostToURL : " << postData << " to URL-> " << url.toString();
    reply = webhookNam->post(URL,postData);
}

void httpPost::tokenReplyFinished(QNetworkReply *reply)
{
    QString output((QString)reply->readAll());

    qDebug() << "httpPost::getAuthenticationToken() : peer response-> " << output;
#if 0
    if (reply->NoError)
    {
        qDebug() << "no reply error detected";
    }
    else
    {
        qDebug() << "reply error detected " << reply->error();
    }
    QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    qDebug() << "connection is encrypted? " << reply->attribute((QNetworkRequest::ConnectionEncryptedAttribute)).toBool();
    qDebug() << "redirect to " << possibleRedirectUrl.toString();
#endif

    if (reply->error() == QNetworkReply::NoError) {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode == 301) {
            QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            qDebug() << "redirect URL is " << redirectUrl.toString();
        }
        else
        {
            //qDebug() << "url error code is =" << statusCode;
        }
    }

#if 1
    qDebug() << "httpPost::getAuthenticationToken() : URL -> " << authenticationURL;
    qDebug() << "httpPost::getAuthenticationToken() : ca certs->" << reply->sslConfiguration().caCertificates();
    qDebug() << "httpPost::getAuthenticationToken() : peer error->" << reply->errorString();
    qDebug() << "httpPost::getAuthenticationToken() : peer cert->" << reply->sslConfiguration().peerCertificateChain();
#endif

    // Example Token:
    // "{\"user\": \"st2admin\", \"token\": \"2a188d4ebe4a43bc87eb46e53a6e87fd\", \"expiry\": \"2016-12-14T19:11:35.620520Z\", \"id\": \"585047e78121922fb61fd530\", \"metadata\": {}}"
    // Parse the JSON output to get the token
    QJsonDocument tokenDocument = QJsonDocument::fromJson(output.toUtf8());
    QJsonObject jsonObject = tokenDocument.object();

    // Store the token in a private variable
    authenticationToken.clear();
    authenticationToken.append(jsonObject.value("token").toString());
}

void httpPost::webhookReplyFinished(QNetworkReply *reply)
{
    qDebug() << "httpPost::webhookReplyFinished : readall";
    QString output((QString)reply->readAll());
    qDebug() << "HTTP POST-> " << output;
}

void httpPost::onSslError(QNetworkReply *reply, QList<QSslError>)
{
    // If SSL is using a self signed certificate, ignore the error that is generated
    reply->ignoreSslErrors();
    qDebug() << "ignoring SSL error";
}

QJsonObject httpPost::parseTcpdumpOutput(QString tcpdumpOutputString)
{
    /* Possible Inputs are:
     * IP:
     * 16:27:27.971356 IP 192.168.0.103 > 56.56.24.179: ICMP echo request, id 16261, seq 2360, length 64
     *
     * TCP:
     * 16:27:58.783521 IP 192.168.0.103.47438 > 50.56.24.179.22: Flags [S], seq 4136178741, win 29200, options [mss 1460,sackOK,TS val 16529358 ecr 0,nop,wscale 7], length 0
     *
     * UDP:
     * 16:30:10.088488 IP 192.168.0.1.57881 > 255.255.255.255.7437: UDP, length 173
     *
     */
    bool ICMP = false;
    QRegExp rx;
    if (tcpdumpOutputString.contains("ICMP"))
    {
        // we have an ICMP packet
        ICMP = true;
        rx.setPattern("(\\d+:\\d+:\\d+.\\d+)\\W+IP\\W+(\\d+.\\d+.\\d+.\\d+)\\W+(\\d+.\\d+.\\d+.\\d+):\\W+(\\w+)");
    }
    else
    {
        // We  have either TCP or UDP
        rx.setPattern("(\\d+:\\d+:\\d+.\\d+)\\W+IP\\W+(\\d+.\\d+.\\d+.\\d+).(\\d+)\\W+(\\d+.\\d+.\\d+.\\d+).(\\d+):\\W+(\\w+)");
    }
    tcpdumpOutputString.replace("Flags", "TCP");

    int pos = rx.indexIn(tcpdumpOutputString);

    QJsonObject pktJsonObject;
    if (pos != -1)
    {
        // format the parsedOutput into JSON
        QString EOL(" \r\n");
        QString json;
        pktJsonObject["hostname"] = getHostname();
        //json.append("hostname:" + getHostname() + EOL);

        pktJsonObject["timestamp"] = rx.cap(1);
        //json.append("timestamp: " + rx.cap(1) + EOL);
        if (ICMP)
        {
            pktJsonObject["sourceIp"] = rx.cap(2);
            //json.append("sourceIp: " + rx.cap(2) + EOL);
            pktJsonObject["sourcePort"] = "not applicable";
            //json.append("sourcePort: not applicable" + EOL);
        }
        else
        {
            pktJsonObject["sourceIp"] = rx.cap(2);
            //json.append("sourceIp: " + rx.cap(2) + EOL);
            pktJsonObject["sourcePort"] = rx.cap(3);
            //json.append("sourcePort: " + rx.cap(3) + EOL);
        }
        if (ICMP)
        {
            pktJsonObject["destinationIp"] = rx.cap(3);
            //json.append("destinationIp: " + rx.cap(3) + EOL);
            pktJsonObject["destinationPort"] = "not applicable";
            //json.append("destinationPort: not applicable" + EOL);
        }
        else
        {
            pktJsonObject["destinationIp"] = rx.cap(4);
            //json.append("destinationIp: " + rx.cap(4) + EOL);
            pktJsonObject["destinationPort"] = rx.cap(5);
            //json.append("destinationPort: " + rx.cap(5) + EOL);
        }
        if (ICMP)
        {
            pktJsonObject["protocol"] = rx.cap(4);
            //json.append("protocol: " + rx.cap(4) + EOL);
        }
        else
        {
            pktJsonObject["protocol"] = rx.cap(6);
            //json.append("protocol: " + rx.cap(6) + EOL);
        }
        qDebug() << "httpPost:parseTcpdumpOutput :: returning json = " << pktJsonObject;
        return  pktJsonObject;
    }
    else
    {
        qDebug() << "Packet Parse Error :: I only decode ICMP, TCP and UDP right now...";
        pktJsonObject["error"] = "error parsing";
        return pktJsonObject;
    }
}

QString httpPost::getHostname()
{
    // Set the hostname
    QProcess P;
    P.start("/bin/hostname");
    P.waitForFinished(1000);
    QString hostname(P.readAll());
    return(hostname);
}
