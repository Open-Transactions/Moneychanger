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

#include "qjsonrpcmessage.h"

class TestQJsonRpcMessage: public QObject
{
    Q_OBJECT  
private slots:
    void testInvalidData();
    void testInvalidDataResponseWithId();
    void testInvalidDataResponseWithoutId();
    void testResponseSameId();
    void testNotificationNoId();
    void testMessageTypes();
    void testPositionalParameters();
    void testEquivalence();
    void testWithVariantListArgs();
};

void TestQJsonRpcMessage::testInvalidData()
{
    QJsonObject invalidData;
    QJsonRpcMessage message(invalidData);
    QCOMPARE(message.type(), QJsonRpcMessage::Invalid);
}

void TestQJsonRpcMessage::testInvalidDataResponseWithId()
{
    // invalid with id
    const char *invalid = "{\"jsonrpc\": \"2.0\", \"params\": [], \"id\": 666}";
    QJsonRpcMessage request(invalid);
    QJsonRpcMessage error    = request.createErrorResponse(QJsonRpc::NoError, QString());
    QJsonRpcMessage response = request.createResponse(QString());
    QCOMPARE(request.type(), QJsonRpcMessage::Invalid);
    QCOMPARE(response.id(), request.id());
    QCOMPARE(error.type(), QJsonRpcMessage::Error);
}

void TestQJsonRpcMessage::testInvalidDataResponseWithoutId()
{
    // invalid without id
    const char *invalid = "{\"jsonrpc\": \"2.0\", \"params\": []}";
    QJsonRpcMessage request(invalid);
    QJsonRpcMessage error    = request.createErrorResponse(QJsonRpc::NoError, QString());
    QJsonRpcMessage response = request.createResponse(QString());
    QCOMPARE(request.type(), QJsonRpcMessage::Invalid);
    QCOMPARE(response.type(), QJsonRpcMessage::Invalid);    
    QCOMPARE(error.id(), 0);
}

void TestQJsonRpcMessage::testResponseSameId()
{
    QJsonRpcMessage request = QJsonRpcMessage::createRequest("testMethod");
    QJsonRpcMessage response = request.createResponse("testResponse");
    QCOMPARE(response.id(), request.id());
}

void TestQJsonRpcMessage::testNotificationNoId()
{
    QJsonRpcMessage notification = QJsonRpcMessage::createNotification("testNotification");
    QCOMPARE(notification.id(), -1);
}

void TestQJsonRpcMessage::testMessageTypes()
{
    QJsonRpcMessage invalid;
    QCOMPARE(invalid.type(), QJsonRpcMessage::Invalid);

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("testMethod");
    QCOMPARE(request.type(), QJsonRpcMessage::Request);

    QJsonRpcMessage response = request.createResponse("testResponse");
    QCOMPARE(response.type(), QJsonRpcMessage::Response);

    QJsonRpcMessage error = request.createErrorResponse(QJsonRpc::NoError);
    QCOMPARE(error.type(), QJsonRpcMessage::Error);

    QJsonRpcMessage notification = QJsonRpcMessage::createNotification("testNotification");
    QCOMPARE(notification.type(), QJsonRpcMessage::Notification);
}

// this is from the spec, I don't think it proves much..
void TestQJsonRpcMessage::testPositionalParameters()
{
    const char *first = "{\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [42, 23], \"id\": 1}";
    QJsonObject firstObject = QJsonDocument::fromJson(first).object();
    const char *second = "{\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [23, 42], \"id\": 2}";
    QJsonObject secondObject = QJsonDocument::fromJson(second).object();
    QVERIFY2(firstObject.value("params").toArray() != secondObject.value("params").toArray(), "params should maintain order");
}

void TestQJsonRpcMessage::testEquivalence()
{
    // request (same as error)
    QJsonRpcMessage firstRequest = QJsonRpcMessage::createRequest("testRequest");
    QJsonRpcMessage secondRequest(firstRequest);
    QJsonRpcMessage thirdRequest = QJsonRpcMessage::createRequest("testRequest", QVariantList() << "with" << "parameters");
    QJsonRpcMessage fourthRequest = thirdRequest;
    QCOMPARE(firstRequest, secondRequest);
    QVERIFY(secondRequest != thirdRequest);
    QCOMPARE(thirdRequest, fourthRequest);

    // notification (no id)
    QJsonRpcMessage firstNotification = QJsonRpcMessage::createNotification("testNotification");
    QJsonRpcMessage secondNotification = QJsonRpcMessage::createNotification("testNotification");
    QJsonRpcMessage thirdNotification = QJsonRpcMessage::createNotification("testNotification", QVariantList() << "first");
    QJsonRpcMessage fourthNotification = QJsonRpcMessage::createNotification("testNotification", QVariantList() << "first");
    QCOMPARE(firstNotification, secondNotification);
    QVERIFY(firstNotification != thirdNotification);
    QCOMPARE(thirdNotification, fourthNotification);

    QJsonRpcMessage invalid;
    QVERIFY(firstRequest != invalid);
    QVERIFY(secondRequest != invalid);
    QVERIFY(thirdRequest != invalid);
    QVERIFY(fourthRequest != invalid);
    QVERIFY(firstNotification != invalid);
    QVERIFY(secondNotification != invalid);
    QVERIFY(thirdNotification != invalid);
    QVERIFY(fourthNotification != invalid);
}

void TestQJsonRpcMessage::testWithVariantListArgs()
{
    const char *varListArgsFormat = "{ " \
            "\"id\": %1, " \
            "\"jsonrpc\": \"2.0\", " \
            "\"method\": \"service.variantListParameter\", " \
            "\"params\": [[ 1, 20, \"hello\", false ]] " \
            "}";

    QVariantList firstParameter;
    firstParameter << 1 << 20 << "hello" << false;
    QJsonRpcMessage requestFromQJsonRpc =
        QJsonRpcMessage::createRequest("service.variantListParameter", QVariant(firstParameter));

    // QJsonRpcMessage::createRequest is creating objects with an unique id,
    // and to allow a random test execution order - json data must have the same id
    int id = requestFromQJsonRpc.id();
    QByteArray varListArgs = QString(varListArgsFormat).arg(id).toLatin1();

    QJsonRpcMessage requestFromData(varListArgs);
    QCOMPARE(requestFromQJsonRpc, requestFromData);
}

QTEST_MAIN(TestQJsonRpcMessage)
#include "tst_qjsonrpcmessage.moc"
