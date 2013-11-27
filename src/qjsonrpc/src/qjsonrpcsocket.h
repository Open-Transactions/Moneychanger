/*
 * Copyright (C) 2012-2013 Matt Broadstone
 * Contact: http://bitbucket.org/devonit/qjsonrpc
 *
 * This file is part of the QJsonRpc Library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */
#ifndef QJSONRPCSOCKET_H
#define QJSONRPCSOCKET_H

#include <QObject>
#include <QScopedPointer>
#include <QIODevice>

#include "qjsonrpcabstractserver.h"
#include "qjsonrpcservice.h"
#include "qjsonrpcmessage.h"
#include "qjsonrpc_export.h"

class QJsonRpcSocketPrivate;
class QJsonRpcServiceReply;
class QJSONRPC_EXPORT QJsonRpcSocket : public QObject
{
    Q_OBJECT
public:
    explicit QJsonRpcSocket(QIODevice *device, QObject *parent = 0);
    ~QJsonRpcSocket();

#if QT_VERSION >= 0x050100 || QT_VERSION <= 0x050000
    QJsonDocument::JsonFormat wireFormat() const;
    void setWireFormat(QJsonDocument::JsonFormat format);
#endif

    bool isValid() const;

public Q_SLOTS:
    virtual void notify(const QJsonRpcMessage &message);
    QJsonRpcMessage sendMessageBlocking(const QJsonRpcMessage &message, int msecs = 30000);
    QJsonRpcServiceReply *sendMessage(const QJsonRpcMessage &message);
//  void sendMessage(const QList<QJsonRpcMessage> &bulk);
    QJsonRpcMessage invokeRemoteMethodBlocking(const QString &method, const QVariant &arg1 = QVariant(),
                                               const QVariant &arg2 = QVariant(), const QVariant &arg3 = QVariant(),
                                               const QVariant &arg4 = QVariant(), const QVariant &arg5 = QVariant(),
                                               const QVariant &arg6 = QVariant(), const QVariant &arg7 = QVariant(),
                                               const QVariant &arg8 = QVariant(), const QVariant &arg9 = QVariant(),
                                               const QVariant &arg10 = QVariant());
    QJsonRpcServiceReply *invokeRemoteMethod(const QString &method, const QVariant &arg1 = QVariant(),
                                             const QVariant &arg2 = QVariant(), const QVariant &arg3 = QVariant(),
                                             const QVariant &arg4 = QVariant(), const QVariant &arg5 = QVariant(),
                                             const QVariant &arg6 = QVariant(), const QVariant &arg7 = QVariant(),
                                             const QVariant &arg8 = QVariant(), const QVariant &arg9 = QVariant(),
                                             const QVariant &arg10 = QVariant());

Q_SIGNALS:
    void messageReceived(const QJsonRpcMessage &message);

protected Q_SLOTS:
    virtual void processIncomingData();

protected:
    virtual void processRequestMessage(const QJsonRpcMessage &message);

private:
    Q_DECLARE_PRIVATE(QJsonRpcSocket)
    QScopedPointer<QJsonRpcSocketPrivate> d_ptr;

};

class QJSONRPC_EXPORT QJsonRpcServiceSocket : public QJsonRpcSocket,
                                              public QJsonRpcServiceProvider
{
    Q_OBJECT
public:
    explicit QJsonRpcServiceSocket(QIODevice *device, QObject *parent = 0);
    ~QJsonRpcServiceSocket();

private:
    virtual void processRequestMessage(const QJsonRpcMessage &message);

};

#endif
