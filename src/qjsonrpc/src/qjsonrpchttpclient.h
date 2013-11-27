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
#ifndef QJSONRPCHTTPCLIENT_H
#define QJSONRPCHTTPCLIENT_H

#include <QObject>
#include <QNetworkReply>

#include "qjsonrpcmessage.h"
#include "qjsonrpcservicereply.h"

class QNetwokReply;
class QAuthenticator;
class QSslError;
class QNetworkAccessManager;
class QJsonRpcHttpClientPrivate;
class QJsonRpcHttpClient : public QObject
{
    Q_OBJECT
public:
    QJsonRpcHttpClient(QObject *parent = 0);
    QJsonRpcHttpClient(QNetworkAccessManager *manager, QObject *parent = 0);
    ~QJsonRpcHttpClient();

    QUrl endPoint() const;
    void setEndPoint(const QUrl &endPoint);
    void setEndPoint(const QString &endPoint);

    QNetworkAccessManager *networkAccessManager();

public Q_SLOTS:
    virtual void notify(const QJsonRpcMessage &message);
    QJsonRpcMessage sendMessageBlocking(const QJsonRpcMessage &message, int msecs = 30000);
    QJsonRpcServiceReply *sendMessage(const QJsonRpcMessage &message);

private Q_SLOTS:
    virtual void handleAuthenticationRequired(QNetworkReply *reply, QAuthenticator * authenticator);
    virtual void handleSslErrors( QNetworkReply * reply, const QList<QSslError> &errors);

private:
    Q_DISABLE_COPY(QJsonRpcHttpClient)
    Q_DECLARE_PRIVATE(QJsonRpcHttpClient)
    QScopedPointer<QJsonRpcHttpClientPrivate> d_ptr;

};

#endif
