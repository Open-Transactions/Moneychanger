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
#ifndef QJSONRPCSERVICEREPLY_H
#define QJSONRPCSERVICEREPLY_H

#include <QObject>
#include <QNetworkReply>

#include "qjsonrpc_export.h"
#include "qjsonrpcmessage.h"

class QJsonRpcServiceReplyPrivate;
class QJSONRPC_EXPORT QJsonRpcServiceReply : public QObject
{
    Q_OBJECT
public:
    explicit QJsonRpcServiceReply(QObject *parent = 0);
    virtual ~QJsonRpcServiceReply();

    QJsonRpcMessage response() const;

Q_SIGNALS:
    void finished();

protected:
    QJsonRpcServiceReply(QJsonRpcServiceReplyPrivate *dd, QObject *parent = 0);

private:
    Q_DISABLE_COPY(QJsonRpcServiceReply)
    Q_DECLARE_PRIVATE(QJsonRpcServiceReply)
    QScopedPointer<QJsonRpcServiceReplyPrivate> d_ptr;
    friend class QJsonRpcSocket;

};

class QJsonRpcHttpReplyPrivate;
class QJSONRPC_EXPORT QJsonRpcHttpReply : public QJsonRpcServiceReply
{
    Q_OBJECT
public:
    explicit QJsonRpcHttpReply(const QJsonRpcMessage &request, QNetworkReply *reply,
                               QObject *parent = 0);
    virtual ~QJsonRpcHttpReply();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyerror(QNetworkReply::NetworkError code);

private:
    Q_DISABLE_COPY(QJsonRpcHttpReply)
    Q_DECLARE_PRIVATE(QJsonRpcHttpReply)
    QScopedPointer<QJsonRpcHttpReplyPrivate> d_ptr;

};

#endif // QJSONRPCSERVICEREPLY_H
