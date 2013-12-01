#include <QMetaObject>
#include <QMetaClassInfo>
#include <QDebug>

#include "qjsonrpcservice_p.h"
#include "qjsonrpcservice.h"
#include "qjsonrpcsocket.h"
#include "qjsonrpcabstractserver_p.h"
#include "qjsonrpcabstractserver.h"

QJsonRpcServiceProvider::QJsonRpcServiceProvider()
    : d_ptr(new QJsonRpcServiceProviderPrivate)
{
}

QJsonRpcServiceProvider::~QJsonRpcServiceProvider()
{
}

QByteArray QJsonRpcServiceProviderPrivate::serviceName(QJsonRpcService *service)
{
    const QMetaObject *mo = service->metaObject();
    for (int i = 0; i < mo->classInfoCount(); i++) {
        const QMetaClassInfo mci = mo->classInfo(i);
        if (mci.name() == QLatin1String("serviceName"))
            return mci.value();
    }

    return QByteArray(mo->className()).toLower();
}

bool QJsonRpcServiceProvider::addService(QJsonRpcService *service)
{
    Q_D(QJsonRpcServiceProvider);
    QByteArray serviceName = d->serviceName(service);
    if (serviceName.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "service added without serviceName classinfo, aborting";
        return false;
    }

    if (d->services.contains(serviceName)) {
        qDebug() << Q_FUNC_INFO << "service with name " << serviceName << " already exist";
        return false;
    }

    service->d_ptr->cacheInvokableInfo();
    d->services.insert(serviceName, service);
    if (!service->parent())
        d->cleanupHandler.add(service);
    return true;
}

bool QJsonRpcServiceProvider::removeService(QJsonRpcService *service)
{
    Q_D(QJsonRpcServiceProvider);
    QByteArray serviceName = d->serviceName(service);
    if (!d->services.contains(serviceName)) {
        qDebug() << Q_FUNC_INFO << "can nof find service with name " << serviceName;
        return false;
    }

    d->cleanupHandler.remove(d->services.value(serviceName));
    d->services.remove(serviceName);
    return true;
}

void QJsonRpcServiceProvider::processMessage(QJsonRpcSocket *socket, const QJsonRpcMessage &message)
{
    Q_D(QJsonRpcServiceProvider);
    switch (message.type()) {
        case QJsonRpcMessage::Request:
        case QJsonRpcMessage::Notification: {
            QByteArray serviceName = message.method().section(".", 0, -2).toLatin1();
            if (serviceName.isEmpty() || !d->services.contains(serviceName)) {
                if (message.type() == QJsonRpcMessage::Request) {
                    QJsonRpcMessage error =
                        message.createErrorResponse(QJsonRpc::MethodNotFound,
                            QString("service '%1' not found").arg(serviceName.constData()));
                    socket->notify(error);
                }
            } else {
                QJsonRpcService *service = d->services.value(serviceName);
                service->d_ptr->socket = socket;
                if (message.type() == QJsonRpcMessage::Request)
                    QObject::connect(service, SIGNAL(result(QJsonRpcMessage)),
                                      socket, SLOT(notify(QJsonRpcMessage)));
                service->dispatch(message);
            }
        }
        break;

        case QJsonRpcMessage::Response:
            // we don't handle responses in the provider
            break;

        default: {
            QJsonRpcMessage error =
                message.createErrorResponse(QJsonRpc::InvalidRequest, QString("invalid request"));
            socket->notify(error);
            break;
        }
    };
}

QJsonRpcAbstractServer::QJsonRpcAbstractServer(QJsonRpcAbstractServerPrivate *dd, QObject *parent)
    : QObject(parent),
      d_ptr(dd)
{
}

QJsonRpcAbstractServer::~QJsonRpcAbstractServer()
{
    Q_D(QJsonRpcAbstractServer);
     foreach (QJsonRpcSocket *client, d->clients)
        client->deleteLater();
    d->clients.clear();
}

bool QJsonRpcAbstractServer::addService(QJsonRpcService *service)
{
    if (!QJsonRpcServiceProvider::addService(service))
        return false;

    connect(service, SIGNAL(notifyConnectedClients(QJsonRpcMessage)),
               this, SLOT(notifyConnectedClients(QJsonRpcMessage)));
    connect(service, SIGNAL(notifyConnectedClients(QString,QVariantList)),
               this, SLOT(notifyConnectedClients(QString,QVariantList)));
    return true;
}

bool QJsonRpcAbstractServer::removeService(QJsonRpcService *service)
{
    if (!QJsonRpcServiceProvider::removeService(service))
        return false;

    disconnect(service, SIGNAL(notifyConnectedClients(QJsonRpcMessage)),
               this, SLOT(notifyConnectedClients(QJsonRpcMessage)));
    disconnect(service, SIGNAL(notifyConnectedClients(QString,QVariantList)),
               this, SLOT(notifyConnectedClients(QString,QVariantList)));
    return true;
}

#if QT_VERSION >= 0x050100 || QT_VERSION <= 0x050000
QJsonDocument::JsonFormat QJsonRpcAbstractServer::wireFormat() const
{
    Q_D(const QJsonRpcAbstractServer);
    return d->format;
}

void QJsonRpcAbstractServer::setWireFormat(QJsonDocument::JsonFormat format)
{
    Q_D(QJsonRpcAbstractServer);
    d->format = format;
}
#endif

void QJsonRpcAbstractServer::notifyConnectedClients(const QString &method, const QVariantList &params)
{
    QJsonRpcMessage notification = QJsonRpcMessage::createNotification(method, params);
    notifyConnectedClients(notification);
}

void QJsonRpcAbstractServer::notifyConnectedClients(const QJsonRpcMessage &message)
{
    Q_D(QJsonRpcAbstractServer);
    for (int i = 0; i < d->clients.size(); ++i)
        d->clients[i]->notify(message);
}

void QJsonRpcAbstractServer::processMessage(const QJsonRpcMessage &message)
{
    QJsonRpcSocket *socket = static_cast<QJsonRpcSocket*>(sender());
    if (!socket) {
        qDebug() << Q_FUNC_INFO << "called without service socket";
        return;
    }

    QJsonRpcServiceProvider::processMessage(socket, message);
}
