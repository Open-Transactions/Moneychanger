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
#include <QDebug>
#if QT_VERSION >= 0x050000
#include <QJsonDocument>
#else
#include "json/qjsondocument.h"
#endif

#include "qjsonrpcservicereply_p.h"
#include "qjsonrpchttpclient.h"

QJsonRpcHttpReply::QJsonRpcHttpReply(const QJsonRpcMessage &request,
                                     QNetworkReply *reply, QObject *parent)
    : QJsonRpcServiceReply(parent),
      d_ptr(new QJsonRpcHttpReplyPrivate)
{
    Q_D(QJsonRpcHttpReply);
    d->request = request;
    d->reply = reply;
    connect(d->reply, SIGNAL(finished()), this, SLOT(networkReplyFinished()));
    connect(d->reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(networkReplyerror(QNetworkReply::NetworkError)));
}

QJsonRpcHttpReply::~QJsonRpcHttpReply()
{
}

void QJsonRpcHttpReply::networkReplyFinished()
{
    Q_D(QJsonRpcHttpReply);
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        qDebug() << Q_FUNC_INFO << "invalid reply";
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        // this should be handled by the networkReplyError slot
    } else {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isEmpty() || doc.isNull() || !doc.isObject()) {
            d->response =
                d->request.createErrorResponse(QJsonRpc::ParseError,
                                               "unable to process incoming JSON data",
                                               data);
        } else {
            if (qgetenv("QJSONRPC_DEBUG").toInt())
                qDebug() << "received: " << doc.toJson();

            QJsonRpcMessage response = QJsonRpcMessage(doc.object());
            if (d->request.type() == QJsonRpcMessage::Request &&
                d->request.id() != response.id()) {
                d->response =
                    d->request.createErrorResponse(QJsonRpc::InternalError,
                                                   "invalid response id", data);
            } else {
                d->response = response;
            }
        }
    }

    Q_EMIT finished();
}

void QJsonRpcHttpReply::networkReplyerror(QNetworkReply::NetworkError code)
{
    Q_D(QJsonRpcHttpReply);
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        qDebug() << Q_FUNC_INFO << "invalid reply";
        return;
    }

    if (code == QNetworkReply::NoError)
        return;

    d->response = d->request.createErrorResponse(QJsonRpc::InternalError,
                                   "error with http request",
                                   reply->errorString());
    Q_EMIT finished();
}

class QJsonRpcHttpClientPrivate
{
public:
    QNetworkReply *writeMessage(const QJsonRpcMessage &message) {
        QNetworkRequest request(endPoint);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QByteArray data = QJsonDocument(message.toObject()).toJson();
        if (qgetenv("QJSONRPC_DEBUG").toInt())
            qDebug() << "sending: " << data;
        return networkAccessManager->post(request, data);
    }

    QUrl endPoint;
    QNetworkAccessManager *networkAccessManager;
};

QJsonRpcHttpClient::QJsonRpcHttpClient(QObject *parent)
    : QObject(parent),
      d_ptr(new QJsonRpcHttpClientPrivate)
{
    Q_D(QJsonRpcHttpClient);
    d->networkAccessManager = new QNetworkAccessManager(this);
    connect(d->networkAccessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(handleAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
    connect(d->networkAccessManager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
            this, SLOT(handleSslErrors(QNetworkReply*,QList<QSslError>)));
}

QJsonRpcHttpClient::QJsonRpcHttpClient(QNetworkAccessManager *manager, QObject *parent)
    : QObject(parent),
      d_ptr(new QJsonRpcHttpClientPrivate)
{
    Q_D(QJsonRpcHttpClient);
    d->networkAccessManager = manager;
    connect(d->networkAccessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(handleAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
    connect(d->networkAccessManager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
            this, SLOT(handleSslErrors(QNetworkReply*,QList<QSslError>)));

}

QJsonRpcHttpClient::~QJsonRpcHttpClient()
{
}

QUrl QJsonRpcHttpClient::endPoint() const
{
    Q_D(const QJsonRpcHttpClient);
    return d->endPoint;
}

void QJsonRpcHttpClient::setEndPoint(const QUrl &endPoint)
{
    Q_D(QJsonRpcHttpClient);
    d->endPoint = endPoint;
}

void QJsonRpcHttpClient::setEndPoint(const QString &endPoint)
{
    Q_D(QJsonRpcHttpClient);
    d->endPoint = QUrl::fromUserInput(endPoint);
}

QNetworkAccessManager *QJsonRpcHttpClient::networkAccessManager()
{
    Q_D(QJsonRpcHttpClient);
    return d->networkAccessManager;
}

void QJsonRpcHttpClient::notify(const QJsonRpcMessage &message)
{
    Q_D(QJsonRpcHttpClient);
    if (d->endPoint.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "invalid endpoint specified";
        return;
    }

    QNetworkReply *reply = d->writeMessage(message);
    connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));

    // NOTE: we might want to connect this to a local slot to track errors
    //       for debugging later?
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), reply, SLOT(deleteLater()));
}

QJsonRpcServiceReply *QJsonRpcHttpClient::sendMessage(const QJsonRpcMessage &message)
{
    Q_D(QJsonRpcHttpClient);
    if (d->endPoint.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "invalid endpoint specified";
        return 0;
    }

    QNetworkReply *reply = d->writeMessage(message);
    return new QJsonRpcHttpReply(message, reply);
}

QJsonRpcMessage QJsonRpcHttpClient::sendMessageBlocking(const QJsonRpcMessage &message, int msecs)
{
    Q_UNUSED(message)
    Q_UNUSED(msecs)
    // TODO

    return QJsonRpcMessage();
}



void QJsonRpcHttpClient::handleAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    Q_UNUSED(reply)
    Q_UNUSED(authenticator)
}

void QJsonRpcHttpClient::handleSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    Q_UNUSED(errors)
    reply->ignoreSslErrors();
}

