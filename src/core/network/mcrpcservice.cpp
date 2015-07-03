#include "mcrpcservice.h"

#include <QDebug>
#include <QTimer>
#include <QEventLoop>
#include <QThreadPool>
#include <QRunnable>


MCRPCService::MCRPCService(QObject *parent)
    : QJsonRpcService(parent)
{
    QThreadPool::globalInstance()->setMaxThreadCount(10);
}


void MCRPCService::testMethod()
{
    qDebug() << Q_FUNC_INFO << "called" << endl;
}


