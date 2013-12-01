/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest>
#include "json/qjsonarray.h"
#include "json/qjsonobject.h"
#include "json/qjsonvalue.h"
#include "json/qjsondocument.h"

#define INVALID_UNICODE "\357\277\277" // "\uffff"
#define UNICODE_DJE "\320\202" // Character from the Serbian Cyrillic alphabet

class tst_QtJson: public QObject
{
    Q_OBJECT
public:
    tst_QtJson(QObject *parent = 0);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testValueSimple();
    void testNumbers();

    void testObjectSimple();
    void testObjectSmallKeys();
    void testArraySimple();
    void testValueObject();
    void testValueArray();
    void testObjectNested();
    void testArrayNested();
    void testArrayNestedEmpty();
    void testObjectNestedEmpty();

    void testValueRef();
    void testObjectIteration();
    void testArrayIteration();

    void testObjectFind();

    void testDocument();

    void nullValues();
    void nullArrays();
    void nullObject();

    void keySorting();

    void undefinedValues();

    void fromVariantMap();
    void toVariantMap();
    void toVariantList();

    void toJson();
    void fromJson();
    void fromJsonErrors();
    void fromBinary();
    void toAndFromBinary_data();
    void toAndFromBinary();
    void parseNumbers();
    void parseStrings();
    void parseDuplicateKeys();
    void testParser();

    void compactArray();
    void compactObject();

    void validation();

    void assignToDocument();

    void testDuplicateKeys();
    void testCompaction();
    void testDebugStream();
    void testCompactionError();

    void parseUnicodeEscapes();

    void assignObjects();
    void assignArrays();

    void testTrailingComma();
    void testDetachBug();

    void valueEquals();

    void bom();
    void nesting();
};

tst_QtJson::tst_QtJson(QObject *parent) : QObject(parent)
{
}

void tst_QtJson::initTestCase()
{
}

void tst_QtJson::cleanupTestCase()
{
}

void tst_QtJson::init()
{
}

void tst_QtJson::cleanup()
{
}

void tst_QtJson::testValueSimple()
{
    QJsonObject object;
    object.insert("number", 999.);
    QJsonArray array;
    for (int i = 0; i < 10; ++i)
        array.append((double)i);

    QJsonValue value(true);
    QCOMPARE(value.type(), QJsonValue::Bool);
    QCOMPARE(value.toDouble(), 0.);
    QCOMPARE(value.toString(), QString());
    QCOMPARE(value.toBool(), true);
    QCOMPARE(value.toObject(), QJsonObject());
    QCOMPARE(value.toArray(), QJsonArray());
    QCOMPARE(value.toDouble(99.), 99.);
    QCOMPARE(value.toString(QString("test")), QString("test"));
    QCOMPARE(value.toObject(object), object);
    QCOMPARE(value.toArray(array), array);

    value = 999.;
    QCOMPARE(value.type(), QJsonValue::Double);
    QCOMPARE(value.toDouble(), 999.);
    QCOMPARE(value.toString(), QString());
    QCOMPARE(value.toBool(), false);
    QCOMPARE(value.toBool(true), true);
    QCOMPARE(value.toObject(), QJsonObject());
    QCOMPARE(value.toArray(), QJsonArray());

    value = QLatin1String("test");
    QCOMPARE(value.toDouble(), 0.);
    QCOMPARE(value.toString(), QLatin1String("test"));
    QCOMPARE(value.toBool(), false);
    QCOMPARE(value.toObject(), QJsonObject());
    QCOMPARE(value.toArray(), QJsonArray());

    value = true;
    QCOMPARE(value.toDouble(), 0.);
    QCOMPARE(value.toString(), QString());
    QCOMPARE(value.toBool(), true);
    QCOMPARE(value.toObject(), QJsonObject());
    QCOMPARE(value.toArray(), QJsonArray());

    value = 999.;
    QCOMPARE(value.toDouble(), 999.);
    QCOMPARE(value.toString(), QString());
    QCOMPARE(value.toBool(), false);
    QCOMPARE(value.toObject(), QJsonObject());
    QCOMPARE(value.toArray(), QJsonArray());
}

void tst_QtJson::testNumbers()
{
    {
        int numbers[] = {
            0,
            -1,
            1,
            (1<<26),
            (1<<27),
            (1<<28),
            -(1<<26),
            -(1<<27),
            -(1<<28),
            (1<<26) - 1,
            (1<<27) - 1,
            (1<<28) - 1,
            -((1<<26) - 1),
            -((1<<27) - 1),
            -((1<<28) - 1)
        };
        int n = sizeof(numbers)/sizeof(int);

        QJsonArray array;
        for (int i = 0; i < n; ++i)
            array.append((double)numbers[i]);
        for (int i = 0; i < array.size(); ++i) {
            QCOMPARE(array.at(i).type(), QJsonValue::Double);
            QCOMPARE(array.at(i).toDouble(), (double)numbers[i]);
        }
    }

    {
        double numbers[] = {
            0,
            -1,
            1,
            (1<<26),
            (1<<27),
            (1<<28),
            -(1<<26),
            -(1<<27),
            -(1<<28),
            (1<<26) - 1,
            (1<<27) - 1,
            (1<<28) - 1,
            -((1<<26) - 1),
            -((1<<27) - 1),
            -((1<<28) - 1),
            1.1,
            0.1,
            -0.1,
            -1.1,
            1e200,
            -1e200
        };
        int n = sizeof(numbers)/sizeof(double);

        QJsonArray array;
        for (int i = 0; i < n; ++i)
            array.append(numbers[i]);
        for (int i = 0; i < array.size(); ++i) {
            QCOMPARE(array.at(i).type(), QJsonValue::Double);
            QCOMPARE(array.at(i).toDouble(), numbers[i]);
        }
    }

}

void tst_QtJson::testObjectSimple()
{
    QJsonObject object;
    object.insert("number", 999.);
    QCOMPARE(object.value("number").type(), QJsonValue::Double);
    QCOMPARE(object.value("number").toDouble(), 999.);
    object.insert("string", QString::fromLatin1("test"));
    QCOMPARE(object.value("string").type(), QJsonValue::String);
    QCOMPARE(object.value("string").toString(), QString("test"));
    object.insert("boolean", true);
    QCOMPARE(object.value("boolean").toBool(), true);

    QStringList keys = object.keys();
    QVERIFY2(keys.contains("number"), "key number not found");
    QVERIFY2(keys.contains("string"), "key string not found");
    QVERIFY2(keys.contains("boolean"), "key boolean not found");

    // if we put a JsonValue into the JsonObject and retrieve
    // it, it should be identical.
    QJsonValue value(QLatin1String("foo"));
    object.insert("value", value);
    QCOMPARE(object.value("value"), value);

    int size = object.size();
    object.remove("boolean");
    QCOMPARE(object.size(), size - 1);
    QVERIFY2(!object.contains("boolean"), "key boolean should have been removed");

    QJsonValue taken = object.take("value");
    QCOMPARE(taken, value);
    QVERIFY2(!object.contains("value"), "key value should have been removed");

    QString before = object.value("string").toString();
    object.insert("string", QString::fromLatin1("foo"));
    QVERIFY2(object.value("string").toString() != before, "value should have been updated");

    size = object.size();
    QJsonObject subobject;
    subobject.insert("number", 42);
    subobject.insert(QLatin1String("string"), QLatin1String("foobar"));
    object.insert("subobject", subobject);
    QCOMPARE(object.size(), size+1);
    QJsonValue subvalue = object.take(QLatin1String("subobject"));
    QCOMPARE(object.size(), size);
    QCOMPARE(subvalue.toObject(), subobject);
    // make object detach by modifying it many times
    for (int i = 0; i < 64; ++i)
        object.insert(QLatin1String("string"), QLatin1String("bar"));
    QCOMPARE(object.size(), size);
    QCOMPARE(subvalue.toObject(), subobject);
}

void tst_QtJson::testObjectSmallKeys()
{
    QJsonObject data1;
    data1.insert(QLatin1String("1"), 123);
    QVERIFY(data1.contains(QLatin1String("1")));
    QCOMPARE(data1.value(QLatin1String("1")).toDouble(), (double)123);
    data1.insert(QLatin1String("12"), 133);
    QCOMPARE(data1.value(QLatin1String("12")).toDouble(), (double)133);
    QVERIFY(data1.contains(QLatin1String("12")));
    data1.insert(QLatin1String("123"), 323);
    QCOMPARE(data1.value(QLatin1String("12")).toDouble(), (double)133);
    QVERIFY(data1.contains(QLatin1String("123")));
    QCOMPARE(data1.value(QLatin1String("123")).type(), QJsonValue::Double);
    QCOMPARE(data1.value(QLatin1String("123")).toDouble(), (double)323);
}

void tst_QtJson::testArraySimple()
{
    QJsonArray array;
    array.append(999.);
    array.append(QString::fromLatin1("test"));
    array.append(true);

    QJsonValue val = array.at(0);
    QCOMPARE(array.at(0).toDouble(), 999.);
    QCOMPARE(array.at(1).toString(), QString("test"));
    QCOMPARE(array.at(2).toBool(), true);
    QCOMPARE(array.size(), 3);

    // if we put a JsonValue into the JsonArray and retrieve
    // it, it should be identical.
    QJsonValue value(QLatin1String("foo"));
    array.append(value);
    QCOMPARE(array.at(3), value);

    int size = array.size();
    array.removeAt(2);
    --size;
    QCOMPARE(array.size(), size);

    QJsonValue taken = array.takeAt(0);
    --size;
    QCOMPARE(taken.toDouble(), 999.);
    QCOMPARE(array.size(), size);

    // check whether null values work
    array.append(QJsonValue());
    ++size;
    QCOMPARE(array.size(), size);
    QCOMPARE(array.last().type(), QJsonValue::Null);
    QCOMPARE(array.last(), QJsonValue());

    QCOMPARE(array.first().type(), QJsonValue::String);
    QCOMPARE(array.first(), QJsonValue(QLatin1String("test")));

    array.prepend(false);
    QCOMPARE(array.first().type(), QJsonValue::Bool);
    QCOMPARE(array.first(), QJsonValue(false));

    QCOMPARE(array.at(-1), QJsonValue(QJsonValue::Undefined));
    QCOMPARE(array.at(array.size()), QJsonValue(QJsonValue::Undefined));

    array.replace(0, -555.);
    QCOMPARE(array.first().type(), QJsonValue::Double);
    QCOMPARE(array.first(), QJsonValue(-555.));
    QCOMPARE(array.at(1).type(), QJsonValue::String);
    QCOMPARE(array.at(1), QJsonValue(QLatin1String("test")));
}

void tst_QtJson::testValueObject()
{
    QJsonObject object;
    object.insert("number", 999.);
    object.insert("string", QLatin1String("test"));
    object.insert("boolean", true);

    QJsonValue value(object);

    // if we don't modify the original JsonObject, toObject()
    // on the JsonValue should return the same object (non-detached).
    QCOMPARE(value.toObject(), object);

    // if we modify the original object, it should detach
    object.insert("test", QJsonValue(QLatin1String("test")));
    QVERIFY2(value.toObject() != object, "object should have detached");
}

void tst_QtJson::testValueArray()
{
    QJsonArray array;
    array.append(999.);
    array.append(QLatin1String("test"));
    array.append(true);

    QJsonValue value(array);

    // if we don't modify the original JsonArray, toArray()
    // on the JsonValue should return the same object (non-detached).
    QCOMPARE(value.toArray(), array);

    // if we modify the original array, it should detach
    array.append(QLatin1String("test"));
    QVERIFY2(value.toArray() != array, "array should have detached");
}

void tst_QtJson::testObjectNested()
{
    QJsonObject inner, outer;
    inner.insert("number", 999.);
    outer.insert("nested", inner);

    // if we don't modify the original JsonObject, value()
    // should return the same object (non-detached).
    QJsonObject value = outer.value("nested").toObject();
    QCOMPARE(value, inner);
    QCOMPARE(value.value("number").toDouble(), 999.);

    // if we modify the original object, it should detach and not
    // affect the nested object
    inner.insert("number", 555.);
    value = outer.value("nested").toObject();
    QVERIFY2(inner.value("number").toDouble() != value.value("number").toDouble(),
             "object should have detached");

    // array in object
    QJsonArray array;
    array.append(123.);
    array.append(456.);
    outer.insert("array", array);
    QCOMPARE(outer.value("array").toArray(), array);
    QCOMPARE(outer.value("array").toArray().at(1).toDouble(), 456.);

    // two deep objects
    QJsonObject twoDeep;
    twoDeep.insert("boolean", true);
    inner.insert("nested", twoDeep);
    outer.insert("nested", inner);
    QCOMPARE(outer.value("nested").toObject().value("nested").toObject(), twoDeep);
    QCOMPARE(outer.value("nested").toObject().value("nested").toObject().value("boolean").toBool(),
             true);
}

void tst_QtJson::testArrayNested()
{
    QJsonArray inner, outer;
    inner.append(999.);
    outer.append(inner);

    // if we don't modify the original JsonArray, value()
    // should return the same array (non-detached).
    QJsonArray value = outer.at(0).toArray();
    QCOMPARE(value, inner);
    QCOMPARE(value.at(0).toDouble(), 999.);

    // if we modify the original array, it should detach and not
    // affect the nested array
    inner.append(555.);
    value = outer.at(0).toArray();
    QVERIFY2(inner.size() != value.size(), "array should have detached");

    // objects in arrays
    QJsonObject object;
    object.insert("boolean", true);
    outer.append(object);
    QCOMPARE(outer.last().toObject(), object);
    QCOMPARE(outer.last().toObject().value("boolean").toBool(), true);

    // two deep arrays
    QJsonArray twoDeep;
    twoDeep.append(QJsonValue(QString::fromLatin1("nested")));
    inner.append(twoDeep);
    outer.append(inner);
    QCOMPARE(outer.last().toArray().last().toArray(), twoDeep);
    QCOMPARE(outer.last().toArray().last().toArray().at(0).toString(), QString("nested"));
}

void tst_QtJson::testArrayNestedEmpty()
{
    QJsonObject object;
    QJsonArray inner;
    object.insert("inner", inner);
    QJsonValue val = object.value("inner");
    QJsonArray value = object.value("inner").toArray();
    QCOMPARE(value.size(), 0);
    QCOMPARE(value, inner);
    QCOMPARE(value.size(), 0);
    object.insert("count", 0.);
    QCOMPARE(object.value("inner").toArray().size(), 0);
    QVERIFY(object.value("inner").toArray().isEmpty());
    QJsonDocument(object).toBinaryData();
    QCOMPARE(object.value("inner").toArray().size(), 0);
}

void tst_QtJson::testObjectNestedEmpty()
{
    QJsonObject object;
    QJsonObject inner;
    QJsonObject inner2;
    object.insert("inner", inner);
    object.insert("inner2", inner2);
    QJsonObject value = object.value("inner").toObject();
    QCOMPARE(value.size(), 0);
    QCOMPARE(value, inner);
    QCOMPARE(value.size(), 0);
    object.insert("count", 0.);
    QCOMPARE(object.value("inner").toObject().size(), 0);
    QCOMPARE(object.value("inner").type(), QJsonValue::Object);
    QJsonDocument(object).toBinaryData();
    QVERIFY(object.value("inner").toObject().isEmpty());
    QVERIFY(object.value("inner2").toObject().isEmpty());
    QJsonDocument doc = QJsonDocument::fromBinaryData(QJsonDocument(object).toBinaryData());
    QVERIFY(!doc.isNull());
    QJsonObject reconstituted(doc.object());
    QCOMPARE(reconstituted.value("inner").toObject().size(), 0);
    QCOMPARE(reconstituted.value("inner").type(), QJsonValue::Object);
    QCOMPARE(reconstituted.value("inner2").type(), QJsonValue::Object);
}

void tst_QtJson::testValueRef()
{
    QJsonArray array;
    array.append(1.);
    array.append(2.);
    array.append(3.);
    array[1] = false;

    QCOMPARE(array.size(), 3);
    QCOMPARE(array.at(0).toDouble(), 1.);
    QCOMPARE(array.at(2).toDouble(), 3.);
    QCOMPARE(array.at(1).type(), QJsonValue::Bool);
    QCOMPARE(array.at(1).toBool(), false);

    QJsonObject object;
    object[QLatin1String("key")] = true;
    QCOMPARE(object.size(), 1);
    object.insert(QLatin1String("null"), QJsonValue());
    QCOMPARE(object.value(QLatin1String("null")), QJsonValue());
    object[QLatin1String("null")] = 100.;
    QCOMPARE(object.value(QLatin1String("null")).type(), QJsonValue::Double);
    QJsonValue val = object[QLatin1String("null")];
    QCOMPARE(val.toDouble(), 100.);
    QCOMPARE(object.size(), 2);

    array[1] = array[2] = object[QLatin1String("key")] = 42;
    QCOMPARE(array[1], array[2]);
    QCOMPARE(array[2], object[QLatin1String("key")]);
    QCOMPARE(object.value(QLatin1String("key")), QJsonValue(42));
}

void tst_QtJson::testObjectIteration()
{
    QJsonObject object;
    for (int i = 0; i < 10; ++i)
        object[QString::number(i)] = (double)i;

    QCOMPARE(object.size(), 10);

    for (QJsonObject::iterator it = object.begin(); it != object.end(); ++it) {
        QJsonValue value = it.value();
        QCOMPARE((double)it.key().toInt(), value.toDouble());
    }

    {
        QJsonObject object2 = object;
        QVERIFY(object == object2);

        QJsonValue val = *object2.begin();
        object2.erase(object2.begin());
        QCOMPARE(object.size(), 10);
        QCOMPARE(object2.size(), 9);

        for (QJsonObject::const_iterator it = object2.constBegin(); it != object2.constEnd(); ++it) {
            QJsonValue value = it.value();
            QVERIFY(it.value() != val);
            QCOMPARE((double)it.key().toInt(), value.toDouble());
        }
    }

    {
        QJsonObject object2 = object;
        QVERIFY(object == object2);

        QJsonObject::iterator it = object2.find(QString::number(5));
        object2.erase(it);
        QCOMPARE(object.size(), 10);
        QCOMPARE(object2.size(), 9);
    }

    {
        QJsonObject::Iterator it = object.begin();
        it += 5;
        QCOMPARE(QJsonValue(it.value()).toDouble(), 5.);
        it -= 3;
        QCOMPARE(QJsonValue(it.value()).toDouble(), 2.);
        QJsonObject::Iterator it2 = it + 5;
        QCOMPARE(QJsonValue(it2.value()).toDouble(), 7.);
        it2 = it - 1;
        QCOMPARE(QJsonValue(it2.value()).toDouble(), 1.);
    }

    {
        QJsonObject::ConstIterator it = object.constBegin();
        it += 5;
        QCOMPARE(QJsonValue(it.value()).toDouble(), 5.);
        it -= 3;
        QCOMPARE(QJsonValue(it.value()).toDouble(), 2.);
        QJsonObject::ConstIterator it2 = it + 5;
        QCOMPARE(QJsonValue(it2.value()).toDouble(), 7.);
        it2 = it - 1;
        QCOMPARE(QJsonValue(it2.value()).toDouble(), 1.);
    }

    QJsonObject::Iterator it = object.begin();
    while (!object.isEmpty())
        it = object.erase(it);
    QCOMPARE(object.size() , 0);
    QVERIFY(it == object.end());
}

void tst_QtJson::testArrayIteration()
{
    QJsonArray array;
    for (int i = 0; i < 10; ++i)
        array.append(i);

    QCOMPARE(array.size(), 10);

    int i = 0;
    for (QJsonArray::iterator it = array.begin(); it != array.end(); ++it, ++i) {
        QJsonValue value = (*it);
        QCOMPARE((double)i, value.toDouble());
    }

    {
        QJsonArray array2 = array;
        QVERIFY(array == array2);

        QJsonValue val = *array2.begin();
        array2.erase(array2.begin());
        QCOMPARE(array.size(), 10);
        QCOMPARE(array2.size(), 9);

        i = 1;
        for (QJsonArray::const_iterator it = array2.constBegin(); it != array2.constEnd(); ++it, ++i) {
            QJsonValue value = (*it);
            QCOMPARE((double)i, value.toDouble());
        }
    }

    {
        QJsonArray::Iterator it = array.begin();
        it += 5;
        QCOMPARE(QJsonValue((*it)).toDouble(), 5.);
        it -= 3;
        QCOMPARE(QJsonValue((*it)).toDouble(), 2.);
        QJsonArray::Iterator it2 = it + 5;
        QCOMPARE(QJsonValue(*it2).toDouble(), 7.);
        it2 = it - 1;
        QCOMPARE(QJsonValue(*it2).toDouble(), 1.);
    }

    {
        QJsonArray::ConstIterator it = array.constBegin();
        it += 5;
        QCOMPARE(QJsonValue((*it)).toDouble(), 5.);
        it -= 3;
        QCOMPARE(QJsonValue((*it)).toDouble(), 2.);
        QJsonArray::ConstIterator it2 = it + 5;
        QCOMPARE(QJsonValue(*it2).toDouble(), 7.);
        it2 = it - 1;
        QCOMPARE(QJsonValue(*it2).toDouble(), 1.);
    }

    QJsonArray::Iterator it = array.begin();
    while (!array.isEmpty())
        it = array.erase(it);
    QCOMPARE(array.size() , 0);
    QVERIFY(it == array.end());
}

void tst_QtJson::testObjectFind()
{
    QJsonObject object;
    for (int i = 0; i < 10; ++i)
        object[QString::number(i)] = i;

    QCOMPARE(object.size(), 10);

    QJsonObject::iterator it = object.find(QLatin1String("1"));
    QCOMPARE((*it).toDouble(), 1.);
    it = object.find(QLatin1String("11"));
    QVERIFY((*it).type() == QJsonValue::Undefined);
    QVERIFY(it == object.end());

    QJsonObject::const_iterator cit = object.constFind(QLatin1String("1"));
    QCOMPARE((*cit).toDouble(), 1.);
    cit = object.constFind(QLatin1String("11"));
    QVERIFY((*it).type() == QJsonValue::Undefined);
    QVERIFY(it == object.end());
}

void tst_QtJson::testDocument()
{
    QJsonDocument doc;
    QCOMPARE(doc.isEmpty(), true);
    QCOMPARE(doc.isArray(), false);
    QCOMPARE(doc.isObject(), false);

    QJsonObject object;
    doc.setObject(object);
    QCOMPARE(doc.isEmpty(), false);
    QCOMPARE(doc.isArray(), false);
    QCOMPARE(doc.isObject(), true);

    object.insert(QLatin1String("Key"), QLatin1String("Value"));
    doc.setObject(object);
    QCOMPARE(doc.isEmpty(), false);
    QCOMPARE(doc.isArray(), false);
    QCOMPARE(doc.isObject(), true);
    QVERIFY(doc.object() == object);
    QVERIFY(doc.array() == QJsonArray());

    doc = QJsonDocument();
    QCOMPARE(doc.isEmpty(), true);
    QCOMPARE(doc.isArray(), false);
    QCOMPARE(doc.isObject(), false);

    QJsonArray array;
    doc.setArray(array);
    QCOMPARE(doc.isEmpty(), false);
    QCOMPARE(doc.isArray(), true);
    QCOMPARE(doc.isObject(), false);

    array.append(QLatin1String("Value"));
    doc.setArray(array);
    QCOMPARE(doc.isEmpty(), false);
    QCOMPARE(doc.isArray(), true);
    QCOMPARE(doc.isObject(), false);
    QVERIFY(doc.array() == array);
    QVERIFY(doc.object() == QJsonObject());

    QJsonObject outer;
    outer.insert(QLatin1String("outerKey"), 22);
    QJsonObject inner;
    inner.insert(QLatin1String("innerKey"), 42);
    outer.insert(QLatin1String("innter"), inner);
    QJsonArray innerArray;
    innerArray.append(23);
    outer.insert(QLatin1String("innterArray"), innerArray);

    QJsonDocument doc2(outer.value(QLatin1String("innter")).toObject());
    QVERIFY(doc2.object().contains(QLatin1String("innerKey")));
    QCOMPARE(doc2.object().value(QLatin1String("innerKey")), QJsonValue(42));

    QJsonDocument doc3;
    doc3.setObject(outer.value(QLatin1String("innter")).toObject());
    QCOMPARE(doc3.isArray(), false);
    QCOMPARE(doc3.isObject(), true);
    QVERIFY(doc3.object().contains(QLatin1String("innerKey")));
    QCOMPARE(doc3.object().value(QLatin1String("innerKey")), QJsonValue(42));

    QJsonDocument doc4(outer.value(QLatin1String("innterArray")).toArray());
    QCOMPARE(doc4.isArray(), true);
    QCOMPARE(doc4.isObject(), false);
    QCOMPARE(doc4.array().size(), 1);
    QCOMPARE(doc4.array().at(0), QJsonValue(23));

    QJsonDocument doc5;
    doc5.setArray(outer.value(QLatin1String("innterArray")).toArray());
    QCOMPARE(doc5.isArray(), true);
    QCOMPARE(doc5.isObject(), false);
    QCOMPARE(doc5.array().size(), 1);
    QCOMPARE(doc5.array().at(0), QJsonValue(23));
}

void tst_QtJson::nullValues()
{
    QJsonArray array;
    array.append(QJsonValue());

    QCOMPARE(array.size(), 1);
    QCOMPARE(array.at(0), QJsonValue());

    QJsonObject object;
    object.insert(QString("key"), QJsonValue());
    QCOMPARE(object.contains("key"), true);
    QCOMPARE(object.size(), 1);
    QCOMPARE(object.value("key"), QJsonValue());
}

void tst_QtJson::nullArrays()
{
    QJsonArray nullArray;
    QJsonArray nonNull;
    nonNull.append(QLatin1String("bar"));

    QCOMPARE(nullArray, QJsonArray());
    QVERIFY(nullArray != nonNull);
    QVERIFY(nonNull != nullArray);

    QCOMPARE(nullArray.size(), 0);
    QCOMPARE(nullArray.takeAt(0), QJsonValue(QJsonValue::Undefined));
    QCOMPARE(nullArray.first(), QJsonValue(QJsonValue::Undefined));
    QCOMPARE(nullArray.last(), QJsonValue(QJsonValue::Undefined));
    nullArray.removeAt(0);
    nullArray.removeAt(-1);

    nullArray.append(QString("bar"));
    nullArray.removeAt(0);

    QCOMPARE(nullArray.size(), 0);
    QCOMPARE(nullArray.takeAt(0), QJsonValue(QJsonValue::Undefined));
    QCOMPARE(nullArray.first(), QJsonValue(QJsonValue::Undefined));
    QCOMPARE(nullArray.last(), QJsonValue(QJsonValue::Undefined));
    nullArray.removeAt(0);
    nullArray.removeAt(-1);
}

void tst_QtJson::nullObject()
{
    QJsonObject nullObject;
    QJsonObject nonNull;
    nonNull.insert(QLatin1String("foo"), QLatin1String("bar"));

    QCOMPARE(nullObject, QJsonObject());
    QVERIFY(nullObject != nonNull);
    QVERIFY(nonNull != nullObject);

    QCOMPARE(nullObject.size(), 0);
    QCOMPARE(nullObject.keys(), QStringList());
    nullObject.remove("foo");
    QCOMPARE(nullObject, QJsonObject());
    QCOMPARE(nullObject.take("foo"), QJsonValue(QJsonValue::Undefined));
    QCOMPARE(nullObject.contains("foo"), false);

    nullObject.insert("foo", QString("bar"));
    nullObject.remove("foo");

    QCOMPARE(nullObject.size(), 0);
    QCOMPARE(nullObject.keys(), QStringList());
    nullObject.remove("foo");
    QCOMPARE(nullObject, QJsonObject());
    QCOMPARE(nullObject.take("foo"), QJsonValue(QJsonValue::Undefined));
    QCOMPARE(nullObject.contains("foo"), false);
}

void tst_QtJson::keySorting()
{
    const char *json = "{ \"B\": true, \"A\": false }";
    QJsonDocument doc = QJsonDocument::fromJson(json);

    QCOMPARE(doc.isObject(), true);

    QJsonObject o = doc.object();
    QCOMPARE(o.size(), 2);
    QJsonObject::const_iterator it = o.constBegin();
    QCOMPARE(it.key(), QLatin1String("A"));
    ++it;
    QCOMPARE(it.key(), QLatin1String("B"));
}

void tst_QtJson::undefinedValues()
{
    QJsonObject object;
    object.insert("Key", QJsonValue(QJsonValue::Undefined));
    QCOMPARE(object.size(), 0);

    object.insert("Key", QLatin1String("Value"));
    QCOMPARE(object.size(), 1);
    QCOMPARE(object.value("Key").type(), QJsonValue::String);
    QCOMPARE(object.value("foo").type(), QJsonValue::Undefined);
    object.insert("Key", QJsonValue(QJsonValue::Undefined));
    QCOMPARE(object.size(), 0);
    QCOMPARE(object.value("Key").type(), QJsonValue::Undefined);

    QJsonArray array;
    array.append(QJsonValue(QJsonValue::Undefined));
    QCOMPARE(array.size(), 1);
    QCOMPARE(array.at(0).type(), QJsonValue::Null);

    QCOMPARE(array.at(1).type(), QJsonValue::Undefined);
    QCOMPARE(array.at(-1).type(), QJsonValue::Undefined);
}


void tst_QtJson::fromVariantMap()
{
    QVariantMap map;
    map.insert(QLatin1String("key1"), QLatin1String("value1"));
    map.insert(QLatin1String("key2"), QLatin1String("value2"));
    QJsonObject object = QJsonObject::fromVariantMap(map);
    QCOMPARE(object.size(), 2);
    QCOMPARE(object.value(QLatin1String("key1")), QJsonValue(QLatin1String("value1")));
    QCOMPARE(object.value(QLatin1String("key2")), QJsonValue(QLatin1String("value2")));

    QVariantList list;
    list.append(true);
    list.append(QVariant());
    list.append(999.);
    list.append(QLatin1String("foo"));
    map.insert("list", list);
    object = QJsonObject::fromVariantMap(map);
    QCOMPARE(object.size(), 3);
    QCOMPARE(object.value(QLatin1String("key1")), QJsonValue(QLatin1String("value1")));
    QCOMPARE(object.value(QLatin1String("key2")), QJsonValue(QLatin1String("value2")));
    QCOMPARE(object.value(QLatin1String("list")).type(), QJsonValue::Array);
    QJsonArray array = object.value(QLatin1String("list")).toArray();
    QCOMPARE(array.size(), 4);
    QCOMPARE(array.at(0).type(), QJsonValue::Bool);
    QCOMPARE(array.at(0).toBool(), true);
    QCOMPARE(array.at(1).type(), QJsonValue::Null);
    QCOMPARE(array.at(2).type(), QJsonValue::Double);
    QCOMPARE(array.at(2).toDouble(), 999.);
    QCOMPARE(array.at(3).type(), QJsonValue::String);
    QCOMPARE(array.at(3).toString(), QLatin1String("foo"));
}

void tst_QtJson::toVariantMap()
{
    QJsonObject object;
    QVariantMap map = object.toVariantMap();
    QVERIFY(map.isEmpty());

    object.insert("Key", QString("Value"));
    object.insert("null", QJsonValue());
    QJsonArray array;
    array.append(true);
    array.append(999.);
    array.append(QLatin1String("string"));
    array.append(QJsonValue());
    object.insert("Array", array);

    map = object.toVariantMap();

    QCOMPARE(map.size(), 3);
    QCOMPARE(map.value("Key"), QVariant(QString("Value")));
    QCOMPARE(map.value("null"), QVariant());
    QCOMPARE(map.value("Array").type(), QVariant::List);
    QVariantList list = map.value("Array").toList();
    QCOMPARE(list.size(), 4);
    QCOMPARE(list.at(0), QVariant(true));
    QCOMPARE(list.at(1), QVariant(999.));
    QCOMPARE(list.at(2), QVariant(QLatin1String("string")));
    QCOMPARE(list.at(3), QVariant());
}

void tst_QtJson::toVariantList()
{
    QJsonArray array;
    QVariantList list = array.toVariantList();
    QVERIFY(list.isEmpty());

    array.append(QString("Value"));
    array.append(QJsonValue());
    QJsonArray inner;
    inner.append(true);
    inner.append(999.);
    inner.append(QLatin1String("string"));
    inner.append(QJsonValue());
    array.append(inner);

    list = array.toVariantList();

    QCOMPARE(list.size(), 3);
    QCOMPARE(list[0], QVariant(QString("Value")));
    QCOMPARE(list[1], QVariant());
    QCOMPARE(list[2].type(), QVariant::List);
    QVariantList vlist = list[2].toList();
    QCOMPARE(vlist.size(), 4);
    QCOMPARE(vlist.at(0), QVariant(true));
    QCOMPARE(vlist.at(1), QVariant(999.));
    QCOMPARE(vlist.at(2), QVariant(QLatin1String("string")));
    QCOMPARE(vlist.at(3), QVariant());
}

void tst_QtJson::toJson()
{
    QJsonObject object;
    object.insert("\\Key\n", QString("Value"));
    object.insert("null", QJsonValue());
    QJsonArray array;
    array.append(true);
    array.append(999.);
    array.append(QLatin1String("string"));
    array.append(QJsonValue());
    array.append(QLatin1String("\\\a\n\r\b\tabcABC\""));
    object.insert("Array", array);

    QByteArray json = QJsonDocument(object).toJson();

    QByteArray expected =
            "{\n"
            "    \"Array\": [\n"
            "        true,\n"
            "        999,\n"
            "        \"string\",\n"
            "        null,\n"
            "        \"\\\\\\u0007\\n\\r\\b\\tabcABC\\\"\"\n"
            "    ],\n"
            "    \"\\\\Key\\n\": \"Value\",\n"
            "    \"null\": null\n"
            "}\n";
    QCOMPARE(json, expected);

    QJsonDocument doc;
    doc.setObject(object);
    json = doc.toJson();
    QCOMPARE(json, expected);

    doc.setArray(array);
    json = doc.toJson();
    expected =
            "[\n"
            "    true,\n"
            "    999,\n"
            "    \"string\",\n"
            "    null,\n"
            "    \"\\\\\\u0007\\n\\r\\b\\tabcABC\\\"\"\n"
            "]\n";
    QCOMPARE(json, expected);
}

void tst_QtJson::fromJson()
{
    {
        QByteArray json = "[\n    true\n]\n";
        QJsonDocument doc = QJsonDocument::fromJson(json);
        QVERIFY(!doc.isEmpty());
        QCOMPARE(doc.isArray(), true);
        QCOMPARE(doc.isObject(), false);
        QJsonArray array = doc.array();
        QCOMPARE(array.size(), 1);
        QCOMPARE(array.at(0).type(), QJsonValue::Bool);
        QCOMPARE(array.at(0).toBool(), true);
        QCOMPARE(doc.toJson(), json);
    }
    {
        QByteArray json = "[]";
        QJsonDocument doc = QJsonDocument::fromJson(json);
        QVERIFY(!doc.isEmpty());
        QCOMPARE(doc.isArray(), true);
        QCOMPARE(doc.isObject(), false);
        QJsonArray array = doc.array();
        QCOMPARE(array.size(), 0);
    }
    {
        QByteArray json = "{}";
        QJsonDocument doc = QJsonDocument::fromJson(json);
        QVERIFY(!doc.isEmpty());
        QCOMPARE(doc.isArray(), false);
        QCOMPARE(doc.isObject(), true);
        QJsonObject object = doc.object();
        QCOMPARE(object.size(), 0);
    }
    {
        QByteArray json = "{\n    \"Key\": true\n}\n";
        QJsonDocument doc = QJsonDocument::fromJson(json);
        QVERIFY(!doc.isEmpty());
        QCOMPARE(doc.isArray(), false);
        QCOMPARE(doc.isObject(), true);
        QJsonObject object = doc.object();
        QCOMPARE(object.size(), 1);
        QCOMPARE(object.value("Key"), QJsonValue(true));
        QCOMPARE(doc.toJson(), json);
    }
    {
        QByteArray json = "[ null, true, false, \"Foo\", 1, [], {} ]";
        QJsonDocument doc = QJsonDocument::fromJson(json);
        QVERIFY(!doc.isEmpty());
        QCOMPARE(doc.isArray(), true);
        QCOMPARE(doc.isObject(), false);
        QJsonArray array = doc.array();
        QCOMPARE(array.size(), 7);
        QCOMPARE(array.at(0).type(), QJsonValue::Null);
        QCOMPARE(array.at(1).type(), QJsonValue::Bool);
        QCOMPARE(array.at(1).toBool(), true);
        QCOMPARE(array.at(2).type(), QJsonValue::Bool);
        QCOMPARE(array.at(2).toBool(), false);
        QCOMPARE(array.at(3).type(), QJsonValue::String);
        QCOMPARE(array.at(3).toString(), QLatin1String("Foo"));
        QCOMPARE(array.at(4).type(), QJsonValue::Double);
        QCOMPARE(array.at(4).toDouble(), 1.);
        QCOMPARE(array.at(5).type(), QJsonValue::Array);
        QCOMPARE(array.at(5).toArray().size(), 0);
        QCOMPARE(array.at(6).type(), QJsonValue::Object);
        QCOMPARE(array.at(6).toObject().size(), 0);
    }
    {
        QByteArray json = "{ \"0\": null, \"1\": true, \"2\": false, \"3\": \"Foo\", \"4\": 1, \"5\": [], \"6\": {} }";
        QJsonDocument doc = QJsonDocument::fromJson(json);
        QVERIFY(!doc.isEmpty());
        QCOMPARE(doc.isArray(), false);
        QCOMPARE(doc.isObject(), true);
        QJsonObject object = doc.object();
        QCOMPARE(object.size(), 7);
        QCOMPARE(object.value("0").type(), QJsonValue::Null);
        QCOMPARE(object.value("1").type(), QJsonValue::Bool);
        QCOMPARE(object.value("1").toBool(), true);
        QCOMPARE(object.value("2").type(), QJsonValue::Bool);
        QCOMPARE(object.value("2").toBool(), false);
        QCOMPARE(object.value("3").type(), QJsonValue::String);
        QCOMPARE(object.value("3").toString(), QLatin1String("Foo"));
        QCOMPARE(object.value("4").type(), QJsonValue::Double);
        QCOMPARE(object.value("4").toDouble(), 1.);
        QCOMPARE(object.value("5").type(), QJsonValue::Array);
        QCOMPARE(object.value("5").toArray().size(), 0);
        QCOMPARE(object.value("6").type(), QJsonValue::Object);
        QCOMPARE(object.value("6").toObject().size(), 0);
    }
}

void tst_QtJson::fromJsonErrors()
{
    {
        QJsonParseError error;
        QByteArray json = "{\n    \n\n";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::UnterminatedObject);
        QCOMPARE(error.offset, 8);
    }
    {
        QJsonParseError error;
        QByteArray json = "{\n    \"key\" 10\n";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::MissingNameSeparator);
        QCOMPARE(error.offset, 13);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    \n\n";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::UnterminatedArray);
        QCOMPARE(error.offset, 8);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n   1, true\n\n";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::UnterminatedArray);
        QCOMPARE(error.offset, 14);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n  1 true\n\n";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::MissingValueSeparator);
        QCOMPARE(error.offset, 7);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    nul";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::IllegalValue);
        QCOMPARE(error.offset, 7);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    nulzz";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::IllegalValue);
        QCOMPARE(error.offset, 10);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    tru";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::IllegalValue);
        QCOMPARE(error.offset, 7);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    trud]";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::IllegalValue);
        QCOMPARE(error.offset, 10);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    fal";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::IllegalValue);
        QCOMPARE(error.offset, 7);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    falsd]";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::IllegalValue);
        QCOMPARE(error.offset, 11);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    11111";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::TerminationByNumber);
        QCOMPARE(error.offset, 11);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    -1E10000]";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::IllegalNumber);
        QCOMPARE(error.offset, 14);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    -1e-10000]";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::IllegalNumber);
        QCOMPARE(error.offset, 15);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    \"\\u12\"]";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::IllegalEscapeSequence);
        QCOMPARE(error.offset, 11);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    \"foo" INVALID_UNICODE "bar\"]";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::IllegalUTF8String);
        QCOMPARE(error.offset, 13);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    \"";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::UnterminatedString);
        QCOMPARE(error.offset, 8);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    \"c" UNICODE_DJE "a\\u12\"]";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::IllegalEscapeSequence);
        QCOMPARE(error.offset, 15);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    \"c" UNICODE_DJE "a" INVALID_UNICODE "bar\"]";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::IllegalUTF8String);
        QCOMPARE(error.offset, 14);
    }
    {
        QJsonParseError error;
        QByteArray json = "[\n    \"c" UNICODE_DJE "a ]";
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        QVERIFY(doc.isEmpty());
        QCOMPARE(error.error, QJsonParseError::UnterminatedString);
        QCOMPARE(error.offset, 14);
    }
}

void tst_QtJson::fromBinary()
{
    QFile file(":/test.json");
    file.open(QFile::ReadOnly);
    QByteArray testJson = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(testJson);
    QJsonDocument outdoc = QJsonDocument::fromBinaryData(doc.toBinaryData());
    QVERIFY(!outdoc.isNull());
    QVERIFY(doc == outdoc);

    QFile bfile(":/test.bjson");
    bfile.open(QFile::ReadOnly);
    QByteArray binary = bfile.readAll();

    QJsonDocument bdoc = QJsonDocument::fromBinaryData(binary);
    QVERIFY(!bdoc.isNull());
    QVERIFY(doc.toVariant() == bdoc.toVariant());
    QVERIFY(doc == bdoc);
}

void tst_QtJson::toAndFromBinary_data()
{
    QTest::addColumn<QString>("filename");
    QTest::newRow("test.json") << (":/test.json");
    QTest::newRow("test2.json") << (":/test2.json");
}

void tst_QtJson::toAndFromBinary()
{
    QFETCH(QString, filename);
    QFile file(filename);
    QVERIFY(file.open(QFile::ReadOnly));
    QByteArray data = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QVERIFY(!doc.isNull());
    QJsonDocument outdoc = QJsonDocument::fromBinaryData(doc.toBinaryData());
    QVERIFY(!outdoc.isNull());
    QVERIFY(doc == outdoc);
}

void tst_QtJson::parseNumbers()
{
    {
        // test number parsing
        struct Numbers {
            const char *str;
            int n;
        };
        Numbers numbers [] = {
            { "0", 0 },
            { "1", 1 },
            { "10", 10 },
            { "-1", -1 },
            { "100000", 100000 },
            { "-999", -999 }
        };
        int size = sizeof(numbers)/sizeof(Numbers);
        for (int i = 0; i < size; ++i) {
            QByteArray json = "[ ";
            json += numbers[i].str;
            json += " ]";
            QJsonDocument doc = QJsonDocument::fromJson(json);
            QVERIFY(!doc.isEmpty());
            QCOMPARE(doc.isArray(), true);
            QCOMPARE(doc.isObject(), false);
            QJsonArray array = doc.array();
            QCOMPARE(array.size(), 1);
            QJsonValue val = array.at(0);
            QCOMPARE(val.type(), QJsonValue::Double);
            QCOMPARE(val.toDouble(), (double)numbers[i].n);
        }
    }
    {
        // test number parsing
        struct Numbers {
            const char *str;
            double n;
        };
        Numbers numbers [] = {
            { "0", 0 },
            { "1", 1 },
            { "10", 10 },
            { "-1", -1 },
            { "100000", 100000 },
            { "-999", -999 },
            { "1.1", 1.1 },
            { "1e10", 1e10 },
            { "-1.1", -1.1 },
            { "-1e10", -1e10 },
            { "-1E10", -1e10 },
            { "1.1e10", 1.1e10 },
            { "1.1e308", 1.1e308 },
            { "-1.1e308", -1.1e308 },
            { "1.1e-308", 1.1e-308 },
            { "-1.1e-308", -1.1e-308 },
            { "1.1e+308", 1.1e+308 },
            { "-1.1e+308", -1.1e+308 },
            { "1.e+308", 1.e+308 },
            { "-1.e+308", -1.e+308 }
        };
        int size = sizeof(numbers)/sizeof(Numbers);
        for (int i = 0; i < size; ++i) {
            QByteArray json = "[ ";
            json += numbers[i].str;
            json += " ]";
            QJsonDocument doc = QJsonDocument::fromJson(json);
            QVERIFY(!doc.isEmpty());
            QCOMPARE(doc.isArray(), true);
            QCOMPARE(doc.isObject(), false);
            QJsonArray array = doc.array();
            QCOMPARE(array.size(), 1);
            QJsonValue val = array.at(0);
            QCOMPARE(val.type(), QJsonValue::Double);
            QCOMPARE(val.toDouble(), numbers[i].n);
        }
    }
}

void tst_QtJson::parseStrings()
{
    const char *strings [] =
    {
        "Foo",
        "abc\\\"abc",
        "abc\\\\abc",
        "abc\\babc",
        "abc\\fabc",
        "abc\\nabc",
        "abc\\rabc",
        "abc\\tabc",
        "abc\\u0019abc",
        "abc" UNICODE_DJE "abc",
    };
    int size = sizeof(strings)/sizeof(const char *);

    for (int i = 0; i < size; ++i) {
        QByteArray json = "[\n    \"";
        json += strings[i];
        json += "\"\n]\n";
        QJsonDocument doc = QJsonDocument::fromJson(json);
        QVERIFY(!doc.isEmpty());
        QCOMPARE(doc.isArray(), true);
        QCOMPARE(doc.isObject(), false);
        QJsonArray array = doc.array();
        QCOMPARE(array.size(), 1);
        QJsonValue val = array.at(0);
        QCOMPARE(val.type(), QJsonValue::String);

        QCOMPARE(doc.toJson(), json);
    }

    struct Pairs {
        const char *in;
        const char *out;
    };
    Pairs pairs [] = {
        { "abc\\/abc", "abc/abc" },
        { "abc\\u0402abc", "abc" UNICODE_DJE "abc" },
        { "abc\\u0065abc", "abceabc" }
    };
    size = sizeof(pairs)/sizeof(Pairs);

    for (int i = 0; i < size; ++i) {
        QByteArray json = "[\n    \"";
        json += pairs[i].in;
        json += "\"\n]\n";
        QByteArray out = "[\n    \"";
        out += pairs[i].out;
        out += "\"\n]\n";
        QJsonDocument doc = QJsonDocument::fromJson(json);
        QVERIFY(!doc.isEmpty());
        QCOMPARE(doc.isArray(), true);
        QCOMPARE(doc.isObject(), false);
        QJsonArray array = doc.array();
        QCOMPARE(array.size(), 1);
        QJsonValue val = array.at(0);
        QCOMPARE(val.type(), QJsonValue::String);

        QCOMPARE(doc.toJson(), out);
    }

}

void tst_QtJson::parseDuplicateKeys()
{
    const char *json = "{ \"B\": true, \"A\": null, \"B\": false }";

    QJsonDocument doc = QJsonDocument::fromJson(json);
    QCOMPARE(doc.isObject(), true);

    QJsonObject o = doc.object();
    QCOMPARE(o.size(), 2);
    QJsonObject::const_iterator it = o.constBegin();
    QCOMPARE(it.key(), QLatin1String("A"));
    QCOMPARE(it.value(), QJsonValue());
    ++it;
    QCOMPARE(it.key(), QLatin1String("B"));
    QCOMPARE(it.value(), QJsonValue(false));
}

void tst_QtJson::testParser()
{
    QFile file(":/test.json");
    file.open(QFile::ReadOnly);
    QByteArray testJson = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(testJson);
    QVERIFY(!doc.isEmpty());
}

void tst_QtJson::compactArray()
{
    QJsonArray array;
    array.append(QLatin1String("First Entry"));
    array.append(QLatin1String("Second Entry"));
    array.append(QLatin1String("Third Entry"));
    QJsonDocument doc(array);
    int s =  doc.toBinaryData().size();
    array.removeAt(1);
    doc.setArray(array);
    QVERIFY(s > doc.toBinaryData().size());
    s = doc.toBinaryData().size();
    QCOMPARE(doc.toJson(),
             QByteArray("[\n"
                        "    \"First Entry\",\n"
                        "    \"Third Entry\"\n"
                        "]\n"));

    array.removeAt(0);
    doc.setArray(array);
    QVERIFY(s > doc.toBinaryData().size());
    s = doc.toBinaryData().size();
    QCOMPARE(doc.toJson(),
             QByteArray("[\n"
                        "    \"Third Entry\"\n"
                        "]\n"));

    array.removeAt(0);
    doc.setArray(array);
    QVERIFY(s > doc.toBinaryData().size());
    s = doc.toBinaryData().size();
    QCOMPARE(doc.toJson(),
             QByteArray("[\n"
                        "]\n"));

}

void tst_QtJson::compactObject()
{
    QJsonObject object;
    object.insert(QLatin1String("Key1"), QLatin1String("First Entry"));
    object.insert(QLatin1String("Key2"), QLatin1String("Second Entry"));
    object.insert(QLatin1String("Key3"), QLatin1String("Third Entry"));
    QJsonDocument doc(object);
    int s =  doc.toBinaryData().size();
    object.remove(QLatin1String("Key2"));
    doc.setObject(object);
    QVERIFY(s > doc.toBinaryData().size());
    s = doc.toBinaryData().size();
    QCOMPARE(doc.toJson(),
             QByteArray("{\n"
                        "    \"Key1\": \"First Entry\",\n"
                        "    \"Key3\": \"Third Entry\"\n"
                        "}\n"));

    object.remove(QLatin1String("Key1"));
    doc.setObject(object);
    QVERIFY(s > doc.toBinaryData().size());
    s = doc.toBinaryData().size();
    QCOMPARE(doc.toJson(),
             QByteArray("{\n"
                        "    \"Key3\": \"Third Entry\"\n"
                        "}\n"));

    object.remove(QLatin1String("Key3"));
    doc.setObject(object);
    QVERIFY(s > doc.toBinaryData().size());
    s = doc.toBinaryData().size();
    QCOMPARE(doc.toJson(),
             QByteArray("{\n"
                        "}\n"));

}

void tst_QtJson::validation()
{
    // this basically tests that we don't crash on corrupt data
    QFile file(":/test.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QByteArray testJson = file.readAll();
    QVERIFY(!testJson.isEmpty());

    QJsonDocument doc = QJsonDocument::fromJson(testJson);
    QVERIFY(!doc.isNull());

    QByteArray binary = doc.toBinaryData();

    // only test the first 1000 bytes. Testing the full file takes too long
    for (int i = 0; i < 1000; ++i) {
        QByteArray corrupted = binary;
        corrupted[i] = char(0xff);
        QJsonDocument doc = QJsonDocument::fromBinaryData(corrupted);
        if (doc.isNull())
            continue;
        QByteArray json = doc.toJson();
    }


    QFile file2(":/test3.json");
    file2.open(QFile::ReadOnly);
    testJson = file2.readAll();
    QVERIFY(!testJson.isEmpty());

    doc = QJsonDocument::fromJson(testJson);
    QVERIFY(!doc.isNull());

    binary = doc.toBinaryData();

    for (int i = 0; i < binary.size(); ++i) {
        QByteArray corrupted = binary;
        corrupted[i] = char(0xff);
        QJsonDocument doc = QJsonDocument::fromBinaryData(corrupted);
        if (doc.isNull())
            continue;
        QByteArray json = doc.toJson();

        corrupted = binary;
        corrupted[i] = 0x00;
        doc = QJsonDocument::fromBinaryData(corrupted);
        if (doc.isNull())
            continue;
        json = doc.toJson();
    }
}

void tst_QtJson::assignToDocument()
{
    {
        const char *json = "{ \"inner\": { \"key\": true } }";
        QJsonDocument doc = QJsonDocument::fromJson(json);

        QJsonObject o = doc.object();
        QJsonValue inner = o.value("inner");

        QJsonDocument innerDoc(inner.toObject());

        QVERIFY(innerDoc != doc);
        QVERIFY(innerDoc.object() == inner.toObject());
    }
    {
        const char *json = "[ [ true ] ]";
        QJsonDocument doc = QJsonDocument::fromJson(json);

        QJsonArray a = doc.array();
        QJsonValue inner = a.at(0);

        QJsonDocument innerDoc(inner.toArray());

        QVERIFY(innerDoc != doc);
        QVERIFY(innerDoc.array() == inner.toArray());
    }
}


void tst_QtJson::testDuplicateKeys()
{
    QJsonObject obj;
    obj.insert(QLatin1String("foo"), QLatin1String("bar"));
    obj.insert(QLatin1String("foo"), QLatin1String("zap"));
    QCOMPARE(obj.size(), 1);
    QCOMPARE(obj.value(QLatin1String("foo")).toString(), QLatin1String("zap"));
}

void tst_QtJson::testCompaction()
{
    // modify object enough times to trigger compactionCounter
    // and make sure the data is still valid
    QJsonObject obj;
    for (int i = 0; i < 33; ++i) {
        obj.remove(QLatin1String("foo"));
        obj.insert(QLatin1String("foo"), QLatin1String("bar"));
    }
    QCOMPARE(obj.size(), 1);
    QCOMPARE(obj.value(QLatin1String("foo")).toString(), QLatin1String("bar"));

    QJsonDocument doc = QJsonDocument::fromBinaryData(QJsonDocument(obj).toBinaryData());
    QVERIFY(!doc.isNull());
    QVERIFY(!doc.isEmpty());
    QCOMPARE(doc.isArray(), false);
    QCOMPARE(doc.isObject(), true);
    QVERIFY(doc.object() == obj);
}

void tst_QtJson::testDebugStream()
{
    {
        // QJsonObject

        QJsonObject object;
        QTest::ignoreMessage(QtDebugMsg, "QJsonObject() ");
        qDebug() << object;

        object.insert(QLatin1String("foo"), QLatin1String("bar"));
        QTest::ignoreMessage(QtDebugMsg, "QJsonObject({\"foo\": \"bar\"}) ");
        qDebug() << object;
    }

    {
        // QJsonArray

        QJsonArray array;
        QTest::ignoreMessage(QtDebugMsg, "QJsonArray() ");
        qDebug() << array;

        array.append(1);
        array.append(QLatin1String("foo"));
        QTest::ignoreMessage(QtDebugMsg, "QJsonArray([1,\"foo\"]) ");
        qDebug() << array;
    }

    {
        // QJsonDocument

        QJsonDocument doc;
        QTest::ignoreMessage(QtDebugMsg, "QJsonDocument() ");
        qDebug() << doc;

        QJsonObject object;
        object.insert(QLatin1String("foo"), QLatin1String("bar"));
        doc.setObject(object);
        QTest::ignoreMessage(QtDebugMsg, "QJsonDocument({\"foo\": \"bar\"}) ");
        qDebug() << doc;

        QJsonArray array;
        array.append(1);
        array.append(QLatin1String("foo"));
        QTest::ignoreMessage(QtDebugMsg, "QJsonDocument([1,\"foo\"]) ");
        doc.setArray(array);
        qDebug() << doc;
    }

    {
        // QJsonValue

        QJsonValue value;

        QTest::ignoreMessage(QtDebugMsg, "QJsonValue(null) ");
        qDebug() << value;

        value = QJsonValue(true); // bool
        QTest::ignoreMessage(QtDebugMsg, "QJsonValue(bool, true) ");
        qDebug() << value;

        value = QJsonValue((double)4.2); // double
        QTest::ignoreMessage(QtDebugMsg, "QJsonValue(double, 4.2) ");
        qDebug() << value;

        value = QJsonValue((int)42); // int
        QTest::ignoreMessage(QtDebugMsg, "QJsonValue(double, 42) ");
        qDebug() << value;

        value = QJsonValue(QLatin1String("foo")); // string
        QTest::ignoreMessage(QtDebugMsg, "QJsonValue(string, \"foo\") ");
        qDebug() << value;

        QJsonArray array;
        array.append(1);
        array.append(QLatin1String("foo"));
        value = QJsonValue(array); // array
        QTest::ignoreMessage(QtDebugMsg, "QJsonValue(array, QJsonArray([1,\"foo\"]) ) ");
        qDebug() << value;

        QJsonObject object;
        object.insert(QLatin1String("foo"), QLatin1String("bar"));
        value = QJsonValue(object); // object
        QTest::ignoreMessage(QtDebugMsg, "QJsonValue(object, QJsonObject({\"foo\": \"bar\"}) ) ");
        qDebug() << value;
    }
}

void tst_QtJson::testCompactionError()
{
    QJsonObject schemaObject;
    schemaObject.insert("_Type", QLatin1String("_SchemaType"));
    schemaObject.insert("name", QLatin1String("Address"));
    schemaObject.insert("schema", QJsonObject());
    {
        QJsonObject content(schemaObject);
        QJsonDocument doc(content);
        QVERIFY(!doc.isNull());
        QByteArray hash = QCryptographicHash::hash(doc.toBinaryData(), QCryptographicHash::Md5).toHex();
        schemaObject.insert("_Version", QString::fromLatin1(hash.constData(), hash.size()));
    }

    QJsonObject schema;
    schema.insert("streetNumber", schema.value("number").toObject());
    schemaObject.insert("schema", schema);
    {
        QJsonObject content(schemaObject);
        content.remove("_Uuid");
        content.remove("_Version");
        QJsonDocument doc(content);
        QVERIFY(!doc.isNull());
        QByteArray hash = QCryptographicHash::hash(doc.toBinaryData(), QCryptographicHash::Md5).toHex();
        schemaObject.insert("_Version", QString::fromLatin1(hash.constData(), hash.size()));
    }
}

void tst_QtJson::parseUnicodeEscapes()
{
    const QByteArray json = "[ \"A\\u00e4\\u00C4\" ]";

    QJsonDocument doc = QJsonDocument::fromJson(json);
    QJsonArray array = doc.array();

    QString result = QLatin1String("A");
    result += QChar(0xe4);
    result += QChar(0xc4);

    QCOMPARE(array.first().toString(), result);
}

void tst_QtJson::assignObjects()
{
    const char *json =
            "[ { \"Key\": 1 }, { \"Key\": 2 } ]";

    QJsonDocument doc = QJsonDocument::fromJson(json);
    QJsonArray array = doc.array();

    QJsonObject object = array.at(0).toObject();
    QCOMPARE(object.value("Key").toDouble(), 1.);

    object = array.at(1).toObject();
    QCOMPARE(object.value("Key").toDouble(), 2.);
}

void tst_QtJson::assignArrays()
{
    const char *json =
            "[ [ 1 ], [ 2 ] ]";

    QJsonDocument doc = QJsonDocument::fromJson(json);
    QJsonArray array = doc.array();

    QJsonArray inner = array.at(0).toArray()  ;
    QCOMPARE(inner.at(0).toDouble(), 1.);

    inner= array.at(1).toArray();
    QCOMPARE(inner.at(0).toDouble(), 2.);
}

void tst_QtJson::testTrailingComma()
{
    const char *jsons[] = { "{ \"Key\": 1, }", "[ { \"Key\": 1 }, ]" };

    for (unsigned i = 0; i < sizeof(jsons)/sizeof(jsons[0]); ++i) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(jsons[i], &error);
        QCOMPARE(error.error, QJsonParseError::MissingObject);
    }
}

void tst_QtJson::testDetachBug()
{
    QJsonObject dynamic;
    QJsonObject embedded;

    QJsonObject local;

    embedded.insert("Key1", QString("Value1"));
    embedded.insert("Key2", QString("Value2"));
    dynamic.insert(QLatin1String("Bogus"), QString("bogusValue"));
    dynamic.insert("embedded", embedded);
    local = dynamic.value("embedded").toObject();

    dynamic.remove("embedded");

    QCOMPARE(local.keys().size(),2);
    local.remove("Key1");
    local.remove("Key2");
    QCOMPARE(local.keys().size(), 0);

    local.insert("Key1", QString("anotherValue"));
    QCOMPARE(local.keys().size(), 1);
}

void tst_QtJson::valueEquals()
{
    QVERIFY(QJsonValue() == QJsonValue());
    QVERIFY(QJsonValue() != QJsonValue(QJsonValue::Undefined));
    QVERIFY(QJsonValue() != QJsonValue(true));
    QVERIFY(QJsonValue() != QJsonValue(1.));
    QVERIFY(QJsonValue() != QJsonValue(QJsonArray()));
    QVERIFY(QJsonValue() != QJsonValue(QJsonObject()));

    QVERIFY(QJsonValue(true) == QJsonValue(true));
    QVERIFY(QJsonValue(true) != QJsonValue(false));
    QVERIFY(QJsonValue(true) != QJsonValue(QJsonValue::Undefined));
    QVERIFY(QJsonValue(true) != QJsonValue());
    QVERIFY(QJsonValue(true) != QJsonValue(1.));
    QVERIFY(QJsonValue(true) != QJsonValue(QJsonArray()));
    QVERIFY(QJsonValue(true) != QJsonValue(QJsonObject()));

    QVERIFY(QJsonValue(1.) == QJsonValue(1.));
    QVERIFY(QJsonValue(1.) != QJsonValue(2.));
    QVERIFY(QJsonValue(1.) != QJsonValue(QJsonValue::Undefined));
    QVERIFY(QJsonValue(1.) != QJsonValue());
    QVERIFY(QJsonValue(1.) != QJsonValue(true));
    QVERIFY(QJsonValue(1.) != QJsonValue(QJsonArray()));
    QVERIFY(QJsonValue(1.) != QJsonValue(QJsonObject()));

    QVERIFY(QJsonValue(QJsonArray()) == QJsonValue(QJsonArray()));
    QJsonArray nonEmptyArray;
    nonEmptyArray.append(true);
    QVERIFY(QJsonValue(QJsonArray()) != nonEmptyArray);
    QVERIFY(QJsonValue(QJsonArray()) != QJsonValue(QJsonValue::Undefined));
    QVERIFY(QJsonValue(QJsonArray()) != QJsonValue());
    QVERIFY(QJsonValue(QJsonArray()) != QJsonValue(true));
    QVERIFY(QJsonValue(QJsonArray()) != QJsonValue(1.));
    QVERIFY(QJsonValue(QJsonArray()) != QJsonValue(QJsonObject()));

    QVERIFY(QJsonValue(QJsonObject()) == QJsonValue(QJsonObject()));
    QJsonObject nonEmptyObject;
    nonEmptyObject.insert("Key", true);
    QVERIFY(QJsonValue(QJsonObject()) != nonEmptyObject);
    QVERIFY(QJsonValue(QJsonObject()) != QJsonValue(QJsonValue::Undefined));
    QVERIFY(QJsonValue(QJsonObject()) != QJsonValue());
    QVERIFY(QJsonValue(QJsonObject()) != QJsonValue(true));
    QVERIFY(QJsonValue(QJsonObject()) != QJsonValue(1.));
    QVERIFY(QJsonValue(QJsonObject()) != QJsonValue(QJsonArray()));
}

void tst_QtJson::bom()
{
    QFile file(":/bom.json");
    file.open(QFile::ReadOnly);
    QByteArray json = file.readAll();

    // Import json document into a QJsonDocument
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);

    QVERIFY(!doc.isNull());
    QVERIFY(error.error == QJsonParseError::NoError);
}

void tst_QtJson::nesting()
{
    // check that we abort parsing too deeply nested json documents.
    // this is to make sure we don't crash because the parser exhausts the
    // stack.

    const char *array_data =
            "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
            "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
            "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
            "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
            "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
            "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
            "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
            "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["
            "]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"
            "]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"
            "]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"
            "]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"
            "]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"
            "]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"
            "]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"
            "]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]";

    QByteArray json(array_data);
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);

    QVERIFY(!doc.isNull());
    QVERIFY(error.error == QJsonParseError::NoError);

    json.prepend("[");
    json.append("]");
    doc = QJsonDocument::fromJson(json, &error);

    QVERIFY(doc.isNull());
    QVERIFY(error.error == QJsonParseError::DeepNesting);

    json = QByteArray("true ");

    for (int i = 0; i < 1024; ++i) {
        json.prepend("{ \"Key\": ");
        json.append(" }");
    }

    doc = QJsonDocument::fromJson(json, &error);

    QVERIFY(!doc.isNull());
    QVERIFY(error.error == QJsonParseError::NoError);

    json.prepend("[");
    json.append("]");
    doc = QJsonDocument::fromJson(json, &error);

    QVERIFY(doc.isNull());
    QVERIFY(error.error == QJsonParseError::DeepNesting);

}

QTEST_MAIN(tst_QtJson)
#include "tst_qtjson.moc"
