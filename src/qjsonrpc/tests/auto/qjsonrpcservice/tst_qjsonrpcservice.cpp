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
#include <QtCore/QVariant>
#include <QtTest/QtTest>

#if QT_VERSION >= 0x050000
#include <QJsonDocument>
#else
#include "json/qjsondocument.h"
#endif

#include "qjsonrpcabstractserver.h"
#include "qjsonrpcmessage.h"
#include "qjsonrpcservice.h"

class TestQJsonRpcService: public QObject
{
    Q_OBJECT  
private slots:
    void testDispatch();
    void testSignals();

};

class TestService : public QJsonRpcService
{
    Q_OBJECT
    Q_CLASSINFO("serviceName", "service")
public:
    TestService(QObject *parent = 0)
        : QJsonRpcService(parent)
    {}

    bool testDispatch(const QJsonRpcMessage &message) {
        return QJsonRpcService::dispatch(message);
    }

Q_SIGNALS:
    void testSignal();
    void testSignalWithParameter(const QString &param);

public Q_SLOTS:
    QString testMethod(const QString &string) const {
        return string;
    }

};

class TestServiceProvider : public QJsonRpcServiceProvider
{
public:
    TestServiceProvider() {}
};

void TestQJsonRpcService::testDispatch()
{
    TestServiceProvider provider;
    TestService service;
    provider.addService(&service);

    QJsonRpcMessage validRequestDispatch =
        QJsonRpcMessage::createRequest("service.testMethod", QString("testParam"));
    QVERIFY(service.testDispatch(validRequestDispatch));

    QJsonRpcMessage validNotificationDispatch =
        QJsonRpcMessage::createNotification("service.testMethod", "testParam");
    QVERIFY(service.testDispatch(validNotificationDispatch));

    QJsonRpcMessage invalidResponseDispatch =
        validRequestDispatch.createResponse("testResult");
    QVERIFY(!service.testDispatch(invalidResponseDispatch));

    QJsonRpcMessage invalidDispatch;
    QVERIFY(!service.testDispatch(invalidDispatch));
}

void TestQJsonRpcService::testSignals()
{
    TestServiceProvider provider;
    TestService service;
    provider.addService(&service);

    QJsonRpcMessage validRequestSignalDispatch =
        QJsonRpcMessage::createRequest("service.testSignal");
    QVERIFY(service.testDispatch(validRequestSignalDispatch));

    QJsonRpcMessage validRequestSignalWithParamDispatch =
        QJsonRpcMessage::createRequest("service.testSignalWithParameter", "testParam");
    QVERIFY(service.testDispatch(validRequestSignalWithParamDispatch));

    QJsonRpcMessage invalidRequestSignalDispatch =
        QJsonRpcMessage::createRequest("service.testSignal", "testParam");
    QCOMPARE(service.testDispatch(invalidRequestSignalDispatch), false);
}

QTEST_MAIN(TestQJsonRpcService)
#include "tst_qjsonrpcservice.moc"
