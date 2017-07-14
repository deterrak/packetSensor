#ifndef tcpdumpThread_H
#define tcpdumpThread_H

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QThread>
#include <QtCore>

class tcpdumpThread : public QThread
{
    Q_OBJECT
public:
    tcpdumpThread();
    void run();
    int ID; //Want to be able to identify each thread
    static bool Stop;
    QString getWfcUrl();
    QString getWfcAuthUrl();
    QString getDescription();

signals:
    void packetReceived(QString output);
private:
    int getConfiguration();
    QString mIp;
    QString mProtocol;
    QString mPort;
    QString mInterface;
    QString mWfc_webhook_url;
    QString mWfc_auth_url;
    QString mDescription;
};

#endif // tcpdumpThread_H
