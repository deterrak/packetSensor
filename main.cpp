#include <QCoreApplication>
#include <QObject>
#include <QDebug>
#include "tcpdumpThread.h"
#include "httppost.h"
#include <QThread>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "main: setup some objects";
    tcpdumpThread capturePackets;

    httpPost postToWorkFlowComposer;

    postToWorkFlowComposer.setDescriptiom(capturePackets.getDescription());
    postToWorkFlowComposer.setWebHookUrl(capturePackets.getWfcUrl());
    postToWorkFlowComposer.setAuthenticationURL(capturePackets.getWfcAuthUrl());


    qDebug() << "main: setup the signal/slots connections";
    QObject::connect(&capturePackets, SIGNAL(packetReceived(QString)), &postToWorkFlowComposer, SLOT(packetReceived(QString)));

    qDebug() << "main: start the capturePackets timer";

    QTimer packetCaptureTtimer;
    QObject::connect(&packetCaptureTtimer, SIGNAL(timeout()), &capturePackets, SLOT(start()));
    packetCaptureTtimer.start(5000);

    //get the initial authentication
    QTimer::singleShot(50, &postToWorkFlowComposer, SLOT(getAuthenticationToken()));

    //refresh the authentication token
    int hourInMilliSeconds = 60*60*1000; // 60 minutes in hour * 60 seconds in minute * 1000 milliseconds in sec
    QTimer authenticationTokenTimer;
    QObject::connect(&authenticationTokenTimer, SIGNAL(timeout()), &postToWorkFlowComposer, SLOT(getAuthenticationToken()));
    authenticationTokenTimer.start(3 * hourInMilliSeconds);

    qDebug() << "main: manually start the capturePactes the first time";
    capturePackets.start();

    return a.exec();
}
