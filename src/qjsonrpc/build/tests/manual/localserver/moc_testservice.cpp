/****************************************************************************
** Meta object code from reading C++ file 'testservice.h'
**
** Created:
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../tests/manual/localserver/testservice.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'testservice.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TestService[] = {

 // content:
       6,       // revision
       0,       // classname
       1,   14, // classinfo
       6,   16, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // classinfo: key, value
      18,   12,

 // slots: signature, parameters, type, tag, flags
      31,   30,   30,   30, 0x0a,
      63,   44,   30,   30, 0x0a,
     131,  105,   30,   30, 0x0a,
     202,  197,  189,   30, 0x0a,
     259,  246,   30,   30, 0x0a,
     313,  307,   30,   30, 0x2a,

       0        // eod
};

static const char qt_meta_stringdata_TestService[] = {
    "TestService\0agent\0serviceName\0\0"
    "testMethod()\0first,second,third\0"
    "testMethodWithParams(QString,bool,double)\0"
    "first,second,third,fourth\0"
    "testMethodWithVariantParams(QString,bool,double,QVariant)\0"
    "QString\0name\0testMethodWithParamsAndReturnValue(QString)\0"
    "first,second\0"
    "testMethodWithDefaultParameter(QString,QString)\0"
    "first\0testMethodWithDefaultParameter(QString)\0"
};

void TestService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        TestService *_t = static_cast<TestService *>(_o);
        switch (_id) {
        case 0: _t->testMethod(); break;
        case 1: _t->testMethodWithParams((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 2: _t->testMethodWithVariantParams((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3])),(*reinterpret_cast< const QVariant(*)>(_a[4]))); break;
        case 3: { QString _r = _t->testMethodWithParamsAndReturnValue((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 4: _t->testMethodWithDefaultParameter((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 5: _t->testMethodWithDefaultParameter((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData TestService::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject TestService::staticMetaObject = {
    { &QJsonRpcService::staticMetaObject, qt_meta_stringdata_TestService,
      qt_meta_data_TestService, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TestService::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TestService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TestService::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TestService))
        return static_cast<void*>(const_cast< TestService*>(this));
    return QJsonRpcService::qt_metacast(_clname);
}

int TestService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QJsonRpcService::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
