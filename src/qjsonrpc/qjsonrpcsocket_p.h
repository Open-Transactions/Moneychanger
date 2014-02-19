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
#ifndef QJSONRPCSOCKET_P_H
#define QJSONRPCSOCKET_P_H

#include <QPointer>
#include <QHash>
#include <QIODevice>

#if QT_VERSION >= 0x050000
#include <QJsonDocument>
#else
#include "json/qjsondocument.h"
#endif

#include "qjsonrpcmessage.h"
#include "qjsonrpc_export.h"

class QJsonRpcServiceReply;
class QJSONRPC_EXPORT QJsonRpcSocketPrivate
{
public:
#if QT_VERSION >= 0x050100 || QT_VERSION <= 0x050000
    QJsonDocument::JsonFormat format;
    QJsonRpcSocketPrivate() : format(QJsonDocument::Compact) {}
#else
    QJsonRpcSocketPrivate() {}
#endif

    int findJsonDocumentEnd(const QByteArray &jsonData);
    void writeData(const QJsonRpcMessage &message);

    QPointer<QIODevice> device;
    QByteArray buffer;
    QHash<int, QPointer<QJsonRpcServiceReply> > replies;

};

#endif
