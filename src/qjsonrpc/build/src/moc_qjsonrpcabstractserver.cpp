/****************************************************************************
** Meta object code from reading C++ file 'qjsonrpcabstractserver.h'
**
** Created:
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qjsonrpcabstractserver.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qjsonrpcabstractserver.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QJsonRpcAbstractServer[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      32,   24,   23,   23, 0x0a,
      86,   72,   23,   23, 0x0a,
     138,  131,   23,   23, 0x2a,
     170,   23,   23,   23, 0x09,
     198,   23,   23,   23, 0x09,
     219,   24,   23,   23, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_QJsonRpcAbstractServer[] = {
    "QJsonRpcAbstractServer\0\0message\0"
    "notifyConnectedClients(QJsonRpcMessage)\0"
    "method,params\0"
    "notifyConnectedClients(QString,QVariantList)\0"
    "method\0notifyConnectedClients(QString)\0"
    "processIncomingConnection()\0"
    "clientDisconnected()\0"
    "processMessage(QJsonRpcMessage)\0"
};

void QJsonRpcAbstractServer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QJsonRpcAbstractServer *_t = static_cast<QJsonRpcAbstractServer *>(_o);
        switch (_id) {
        case 0: _t->notifyConnectedClients((*reinterpret_cast< const QJsonRpcMessage(*)>(_a[1]))); break;
        case 1: _t->notifyConnectedClients((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QVariantList(*)>(_a[2]))); break;
        case 2: _t->notifyConnectedClients((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->processIncomingConnection(); break;
        case 4: _t->clientDisconnected(); break;
        case 5: _t->processMessage((*reinterpret_cast< const QJsonRpcMessage(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QJsonRpcAbstractServer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QJsonRpcAbstractServer::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QJsonRpcAbstractServer,
      qt_meta_data_QJsonRpcAbstractServer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QJsonRpcAbstractServer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QJsonRpcAbstractServer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QJsonRpcAbstractServer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QJsonRpcAbstractServer))
        return static_cast<void*>(const_cast< QJsonRpcAbstractServer*>(this));
    if (!strcmp(_clname, "QJsonRpcServiceProvider"))
        return static_cast< QJsonRpcServiceProvider*>(const_cast< QJsonRpcAbstractServer*>(this));
    return QObject::qt_metacast(_clname);
}

int QJsonRpcAbstractServer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
