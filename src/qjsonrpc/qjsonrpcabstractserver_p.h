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
#ifndef QJSONRPCABSTRACTSERVER_P_H
#define QJSONRPCABSTRACTSERVER_P_H

#include <QObjectCleanupHandler>

#if QT_VERSION >= 0x050000
#include <QJsonDocument>
#else
#include "json/qjsondocument.h"
#endif

class QJsonRpcService;
class QJsonRpcServiceProviderPrivate
{
public:
    QByteArray serviceName(QJsonRpcService *service);

    QHash<QByteArray, QJsonRpcService*> services;
    QObjectCleanupHandler cleanupHandler;

};

class QJsonRpcSocket;
class QJsonRpcAbstractServerPrivate
{
public:
#if QT_VERSION >= 0x050100 || QT_VERSION <= 0x050000
    QJsonDocument::JsonFormat format;
    QJsonRpcAbstractServerPrivate() : format(QJsonDocument::Compact) {}
#else
    QJsonRpcAbstractServerPrivate() {}
#endif

    QList<QJsonRpcSocket*> clients;

};

#endif
