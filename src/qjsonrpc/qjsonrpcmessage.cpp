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
#   include <QJsonDocument>
#else
#   include "json/qjsondocument.h"
#endif

#include "qjsonrpcmessage.h"

class QJsonRpcMessagePrivate : public QSharedData
{
public:
    QJsonRpcMessagePrivate();
    ~QJsonRpcMessagePrivate();

    void initializeWithObject(const QJsonObject &message);
    static QJsonRpcMessage createBasicRequest(const QString &method, const QVariantList &params);
    QJsonRpcMessage::Type type;
    QJsonObject *object;

    static int uniqueRequestCounter;

};

int QJsonRpcMessagePrivate::uniqueRequestCounter = 0;
QJsonRpcMessagePrivate::QJsonRpcMessagePrivate()
    : type(QJsonRpcMessage::Invalid),
      object(0)
{
}

void QJsonRpcMessagePrivate::initializeWithObject(const QJsonObject &message)
{
    object = new QJsonObject(message);
    if (message.contains("id")) {
        if (message.contains("result") || message.contains("error")) {
            if (message.contains("error"))
                type = QJsonRpcMessage::Error;
            else
                type = QJsonRpcMessage::Response;
        } else if (message.contains("method")) {
            type = QJsonRpcMessage::Request;
        }
    } else {
        if (message.contains("method")) {
            type = QJsonRpcMessage::Notification;
        }
    }
}


QJsonRpcMessagePrivate::~QJsonRpcMessagePrivate()
{
    if (object)
        delete object;
}

QJsonRpcMessage::QJsonRpcMessage()
    : d(new QJsonRpcMessagePrivate)
{
    d->object = new QJsonObject;
}

QJsonRpcMessage::QJsonRpcMessage(const QJsonRpcMessage &other)
    : d(other.d)
{
}

QJsonRpcMessage::~QJsonRpcMessage()
{
}

QJsonRpcMessage &QJsonRpcMessage::operator=(const QJsonRpcMessage &other)
{
    d = other.d;
    return *this;
}

bool QJsonRpcMessage::operator==(const QJsonRpcMessage &message) const
{
    if (message.d == d)
        return true;

    if (message.type() == type()) {
        if (message.type() == QJsonRpcMessage::Error) {
            return (message.errorCode() == errorCode() &&
                    message.errorMessage() == errorMessage() &&
                    message.errorData() == errorData());
        } else {
            if (message.type() == QJsonRpcMessage::Notification) {
                return (message.method() == method() &&
                        message.params() == params());
            } else {
                return (message.id() == id() &&
                        message.method() == method() &&
                        message.params() == params());
            }
        }
    }

    return false;
}

QJsonRpcMessage::QJsonRpcMessage(const QByteArray &message)
    : d(new QJsonRpcMessagePrivate)
{
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(message, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << Q_FUNC_INFO << error.errorString();
        return;
    }

    if (!document.isObject()) {
        qWarning() << Q_FUNC_INFO << "invalid message: " << message;
        return;
    }

    d->initializeWithObject(document.object());
}

QJsonRpcMessage::QJsonRpcMessage(const QJsonObject &message)
    : d(new QJsonRpcMessagePrivate)
{
    d->initializeWithObject(message);
}

QJsonObject QJsonRpcMessage::toObject() const
{
    if (d->object)
        return QJsonObject(*d->object);
    return QJsonObject();
}

bool QJsonRpcMessage::isValid() const
{
    return d->type != QJsonRpcMessage::Invalid;
}

QJsonRpcMessage::Type QJsonRpcMessage::type() const
{
    return d->type;
}

QJsonRpcMessage QJsonRpcMessagePrivate::createBasicRequest(const QString &method, const QVariantList &params)
{
    QJsonRpcMessage request;
    request.d->object = new QJsonObject;
    request.d->object->insert("jsonrpc", QLatin1String("2.0"));
    request.d->object->insert("method", method);
    if (!params.isEmpty())
        request.d->object->insert("params", QJsonArray::fromVariantList(params));
    return request;
}

QJsonRpcMessage QJsonRpcMessage::createRequest(const QString &method, const QVariantList &params)
{
    QJsonRpcMessage request = QJsonRpcMessagePrivate::createBasicRequest(method, params);
    request.d->type = QJsonRpcMessage::Request;
    QJsonRpcMessagePrivate::uniqueRequestCounter++;
    request.d->object->insert("id", QJsonRpcMessagePrivate::uniqueRequestCounter);
    return request;
}

QJsonRpcMessage QJsonRpcMessage::createRequest(const QString &method, const QVariant &param)
{
    return createRequest(method, QVariantList() << param);
}

QJsonRpcMessage QJsonRpcMessage::createNotification(const QString &method, const QVariantList &params)
{
    QJsonRpcMessage notification = QJsonRpcMessagePrivate::createBasicRequest(method, params);
    notification.d->type = QJsonRpcMessage::Notification;
    return notification;
}

QJsonRpcMessage QJsonRpcMessage::createNotification(const QString &method, const QVariant &param)
{
    return createNotification(method, QVariantList() << param);
}

QJsonRpcMessage QJsonRpcMessage::createResponse(const QVariant &result) const
{
    QJsonRpcMessage response;
    if (d->object->contains("id")) {
        QJsonObject *object = new QJsonObject;
        object->insert("jsonrpc", QLatin1String("2.0"));
        object->insert("id", d->object->value("id"));
        object->insert("result", QJsonValue::fromVariant(result));
        response.d->type = QJsonRpcMessage::Response;
        response.d->object = object;
    }

    return response;
}

QJsonRpcMessage QJsonRpcMessage::createErrorResponse(QJsonRpc::ErrorCode code, const QString &message, const QVariant &data) const
{
    QJsonRpcMessage response;
    QJsonObject error;
    error.insert("code", code);
    if (!message.isEmpty())
        error.insert("message", message);
    if (data.isValid())
        error.insert("data", QJsonValue::fromVariant(data));

    response.d->type = QJsonRpcMessage::Error;
    QJsonObject *object = new QJsonObject;
    object->insert("jsonrpc", QLatin1String("2.0"));

    if (d->object->contains("id"))
        object->insert("id", d->object->value("id"));
    else
        object->insert("id", 0);
    object->insert("error", error);
    response.d->object = object;
    return response;
}

int QJsonRpcMessage::id() const
{
    if (d->type == QJsonRpcMessage::Notification || !d->object)
        return -1;
    return d->object->value("id").toVariant().toInt();
}

QString QJsonRpcMessage::method() const
{
    if (d->type == QJsonRpcMessage::Response || !d->object)
        return QString();

    return d->object->value("method").toString();
}

QVariantList QJsonRpcMessage::params() const
{
    if (d->type == QJsonRpcMessage::Response || d->type == QJsonRpcMessage::Error)
        return QVariantList();
    if (!d->object)
        return QVariantList();

    return d->object->value("params").toVariant().toList();
}

QVariant QJsonRpcMessage::result() const
{
    if (d->type != QJsonRpcMessage::Response || !d->object)
        return QVariant();

    return d->object->value("result").toVariant();
}

int QJsonRpcMessage::errorCode() const
{
    if (d->type != QJsonRpcMessage::Error || !d->object)
        return 0;

    QJsonObject error = d->object->value("error").toObject();
    return error.value("code").toVariant().toInt();
}

QString QJsonRpcMessage::errorMessage() const
{
    if (d->type != QJsonRpcMessage::Error || !d->object)
        return QString();

    QJsonObject error = d->object->value("error").toObject();
    return error.value("message").toString();
}

QVariant QJsonRpcMessage::errorData() const
{
    if (d->type != QJsonRpcMessage::Error || !d->object)
        return QVariant();

    QJsonObject error = d->object->value("error").toObject();
    return error.value("data").toVariant();
}

static QDebug operator<<(QDebug dbg, QJsonRpcMessage::Type type)
{
    switch (type) {
    case QJsonRpcMessage::Request:
        return dbg << "QJsonRpcMessage::Request";
    case QJsonRpcMessage::Response:
        return dbg << "QJsonRpcMessage::Response";
    case QJsonRpcMessage::Notification:
        return dbg << "QJsonRpcMessage::Notification";
    case QJsonRpcMessage::Error:
        return dbg << "QJsonRpcMessage::Error";
    default:
        return dbg << "QJsonRpcMessage::Invalid";
    }
}

QDebug operator<<(QDebug dbg, const QJsonRpcMessage &msg)
{
    dbg.nospace() << "QJsonRpcMessage(type=" << msg.type();
    if (msg.type() != QJsonRpcMessage::Notification) {
        dbg.nospace() << ", id=" << msg.id();
    }

    if (msg.type() == QJsonRpcMessage::Request ||
        msg.type() == QJsonRpcMessage::Notification) {
        dbg.nospace() << ", method=" << msg.method()
                      << ", params=" << msg.params();
    } else if (msg.type() == QJsonRpcMessage::Response) {
        dbg.nospace() << ", result=" << msg.result();
    } else if (msg.type() == QJsonRpcMessage::Error) {
        dbg.nospace() << ", code=" << msg.errorCode()
                      << ", message=" << msg.errorMessage()
                      << ", data=" << msg.errorData();
    }
    dbg.nospace() << ")";
    return dbg.space();
}
