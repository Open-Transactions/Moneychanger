/****************************************************************************
** Meta object code from reading C++ file 'qjsonrpcservice.h'
**
** Created:
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qjsonrpcservice.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qjsonrpcservice.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QJsonRpcService[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: signature, parameters, type, tag, flags
      24,   17,   16,   16, 0x05,
      56,   48,   16,   16, 0x05,
     110,   96,   16,   16, 0x05,
     162,  155,   16,   16, 0x25,

 // slots: signature, parameters, type, tag, flags
     207,  199,  194,   16, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_QJsonRpcService[] = {
    "QJsonRpcService\0\0result\0result(QJsonRpcMessage)\0"
    "message\0notifyConnectedClients(QJsonRpcMessage)\0"
    "method,params\0"
    "notifyConnectedClients(QString,QVariantList)\0"
    "method\0notifyConnectedClients(QString)\0"
    "bool\0request\0dispatch(QJsonRpcMessage)\0"
};

void QJsonRpcService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QJsonRpcService *_t = static_cast<QJsonRpcService *>(_o);
        switch (_id) {
        case 0: _t->result((*reinterpret_cast< const QJsonRpcMessage(*)>(_a[1]))); break;
        case 1: _t->notifyConnectedClients((*reinterpret_cast< const QJsonRpcMessage(*)>(_a[1]))); break;
        case 2: _t->notifyConnectedClients((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QVariantList(*)>(_a[2]))); break;
        case 3: _t->notifyConnectedClients((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: { bool _r = _t->dispatch((*reinterpret_cast< const QJsonRpcMessage(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QJsonRpcService::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QJsonRpcService::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QJsonRpcService,
      qt_meta_data_QJsonRpcService, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QJsonRpcService::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QJsonRpcService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QJsonRpcService::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QJsonRpcService))
        return static_cast<void*>(const_cast< QJsonRpcService*>(this));
    return QObject::qt_metacast(_clname);
}

int QJsonRpcService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void QJsonRpcService::result(const QJsonRpcMessage & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QJsonRpcService::notifyConnectedClients(const QJsonRpcMessage & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QJsonRpcService::notifyConnectedClients(const QString & _t1, const QVariantList & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
