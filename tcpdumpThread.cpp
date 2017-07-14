#include "tcpdumpThread.h"
#include <QProcess>
#include <QtCore>

tcpdumpThread::tcpdumpThread()
{   
    // load the configuration items
    getConfiguration();
}

bool tcpdumpThread::Stop = false;

void tcpdumpThread::run()
{
    // Get the configuration Information
    QString command("/usr/sbin/tcpdump ");
    //command.append("-c 1 -n -i wlp4s0 ip host 50.56.24.179");
    command.append("-c 1 -n -i " + mInterface + " ");

    if (mProtocol.contains("icmp"))
        command.append("icmp and ip ");
    else
    {

        if (mProtocol.contains("udp"))
            command.append("udp port " + mPort + " and ip ");

        if (mProtocol.contains("tcp"))
            command.append("tcp port " + mPort + " and ip ");
    }

    if (mProtocol.contains("ip"))
        command.append("ip ");

    if (!mIp.isEmpty())
        command.append("net " + mIp);

    QProcess P;
    qDebug() << "tcpdumpThread::run() : Starting packet capture: " << command;

    // Start TCPDUMP
    P.start(command);
    P.waitForFinished(-1);

    // Emit signals if hysteriesisis satisfied
    QString output(P.readAllStandardOutput());

    emit packetReceived(output);
    qDebug() << "tcpdumpthread: packet received " << output;

}

QString tcpdumpThread::getWfcUrl()
{
    return mWfc_webhook_url;
}

QString tcpdumpThread::getWfcAuthUrl()
{
    return mWfc_auth_url;
}

QString tcpdumpThread::getDescription()
{
    return mDescription;
}

int tcpdumpThread::getConfiguration()
{
    // open a file for reading
    QFile file("config.json");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        qDebug() << "configuration file was opened!";
    else
        qDebug() << "configuration file was not opened";

    // read the contents
    QByteArray contents;
    contents = file.readAll();
    file.close();

    qDebug() << "--- file conetents are ---";
    qDebug() << contents;
    qDebug() << "---";

    // convert to JSON
    QJsonParseError *error = new QJsonParseError();
    QJsonDocument jsonDoc;
    jsonDoc = QJsonDocument::fromJson(contents,  error);

    QJsonObject jsonObj = jsonDoc.object();

    if (!error->errorString().isEmpty())
    {
        qDebug() << "json parse erros string is -> " << error->errorString();
    }
    free(error);

    // Parse the JSON values and store in private variables
    qDebug() << "configuration is ---> " << jsonDoc;
    mIp = jsonObj.value("ip").toString();
    mInterface = jsonObj.value("interface").toString();
    mPort = jsonObj.value("port").toString();
    mProtocol = jsonObj.value("protocol").toString();
    mWfc_webhook_url = jsonObj.value("wfcWebhookUrl").toString();
    mWfc_auth_url = jsonObj.value("wfcAuthenticationUrl").toString();
    mDescription = jsonObj.value("description").toString();

    return 1;
}
