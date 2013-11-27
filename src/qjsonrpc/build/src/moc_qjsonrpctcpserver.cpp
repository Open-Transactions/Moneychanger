/****************************************************************************
** Meta object code from reading C++ file 'qjsonrpctcpserver.h'
**
** Created:
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/qjsonrpctcpserver.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qjsonrpctcpserver.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QJsonRpcTcpServer[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x08,
      47,   18,   18,   18, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QJsonRpcTcpServer[] = {
    "QJsonRpcTcpServer\0\0processIncomingConnection()\0"
    "clientDisconnected()\0"
};

void QJsonRpcTcpServer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QJsonRpcTcpServer *_t = static_cast<QJsonRpcTcpServer *>(_o);
        switch (_id) {
        case 0: _t->processIncomingConnection(); break;
        case 1: _t->clientDisconnected(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData QJsonRpcTcpServer::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QJsonRpcTcpServer::staticMetaObject = {
    { &QJsonRpcAbstractServer::staticMetaObject, qt_meta_stringdata_QJsonRpcTcpServer,
      qt_meta_data_QJsonRpcTcpServer, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QJsonRpcTcpServer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QJsonRpcTcpServer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QJsonRpcTcpServer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QJsonRpcTcpServer))
        return static_cast<void*>(const_cast< QJsonRpcTcpServer*>(this));
    return QJsonRpcAbstractServer::qt_metacast(_clname);
}

int QJsonRpcTcpServer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QJsonRpcAbstractServer::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
