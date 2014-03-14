#include <QLocalServer>
#include <QLocalSocket>

#include "qjsonrpcsocket.h"
#include "qjsonrpcabstractserver_p.h"
#include "qjsonrpclocalserver.h"

class QJsonRpcLocalServerPrivate : public QJsonRpcAbstractServerPrivate
{
public:
    QJsonRpcLocalServerPrivate() : server(0) {}
    QLocalServer *server;
    QHash<QLocalSocket*, QJsonRpcSocket*> socketLookup;
};

QJsonRpcLocalServer::QJsonRpcLocalServer(QObject *parent)
    : QJsonRpcAbstractServer(new QJsonRpcLocalServerPrivate, parent)
{
}

QJsonRpcLocalServer::~QJsonRpcLocalServer()
{
    Q_D(QJsonRpcLocalServer);
    foreach (QLocalSocket *socket, d->socketLookup.keys())
        socket->deleteLater();
    d->socketLookup.clear();
}

bool QJsonRpcLocalServer::listen(const QString &service)
{
    Q_D(QJsonRpcLocalServer);
    if (!d->server) {
        d->server = new QLocalServer(this);
        connect(d->server, SIGNAL(newConnection()), this, SLOT(processIncomingConnection()));
    }

    return d->server->listen(service);
}

void QJsonRpcLocalServer::processIncomingConnection()
{
    Q_D(QJsonRpcLocalServer);
    QLocalSocket *localSocket = d->server->nextPendingConnection();
    if (!localSocket) {
        qDebug() << Q_FUNC_INFO << "nextPendingConnection is null";
        return;
    }

    QIODevice *device = qobject_cast<QIODevice*>(localSocket);
    QJsonRpcSocket *socket = new QJsonRpcSocket(device, this);
#if QT_VERSION >= 0x050100 || QT_VERSION <= 0x050000
    socket->setWireFormat(d->format);
#endif

    connect(socket, SIGNAL(messageReceived(QJsonRpcMessage)), this, SLOT(processMessage(QJsonRpcMessage)));
    d->clients.append(socket);
    connect(localSocket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    d->socketLookup.insert(localSocket, socket);
}

void QJsonRpcLocalServer::clientDisconnected()
{
    Q_D(QJsonRpcLocalServer);
    QLocalSocket *localSocket = static_cast<QLocalSocket*>(sender());
    if (localSocket) {
        if (d->socketLookup.contains(localSocket)) {
            QJsonRpcSocket *socket = d->socketLookup.take(localSocket);
            d->clients.removeAll(socket);
            socket->deleteLater();
        }

        localSocket->deleteLater();
    }
}

QString QJsonRpcLocalServer::errorString() const
{
    Q_D(const QJsonRpcLocalServer);
    return d->server->errorString();
}
