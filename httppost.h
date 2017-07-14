#ifndef HTTPPOST_H
#define HTTPPOST_H
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QObject>

class httpPost : public QObject
{
    Q_OBJECT
public:
    httpPost();
    void setAuthenticationURL(QString URL);
    void setWebHookUrl(QString URL);
    void setDescriptiom(QString Description);

public slots:
    void packetReceived(QString input);
    void getAuthenticationToken();
    void httpPostToURL(QString URL, QJsonObject postPayload);
    void tokenReplyFinished(QNetworkReply*);
    void webhookReplyFinished(QNetworkReply*);
    void onSslError(QNetworkReply*, QList<QSslError>);
private:
    QNetworkAccessManager *authenticationNam;
    QNetworkAccessManager *webhookNam;
    QNetworkReply* reply;
    QString authenticationURL;
    QString webHookURL;
    QByteArray authenticationToken;
    QJsonObject parseTcpdumpOutput(QString tcpdumpOutputString);
    QString getHostname();
    QString description;
};

#endif // HTTPPOST_H
