
HEADERS += \
    $${PWD}/qjsonrpc_export.h \
    $${PWD}/qjsonrpcabstractserver.h \
    $${PWD}/qjsonrpcabstractserver_p.h \
    $${PWD}/qjsonrpclocalserver.h \
    $${PWD}/qjsonrpcmessage.h \
    $${PWD}/qjsonrpcservice.h \
    $${PWD}/qjsonrpcservice_p.h \
    $${PWD}/qjsonrpcservicereply.h \
    $${PWD}/qjsonrpcsocket.h \
    $${PWD}/qjsonrpcsocket_p.h \
    $${PWD}/qjsonrpctcpserver.h

SOURCES += \
    $${PWD}/qjsonrpcabstractserver.cpp \
    $${PWD}/qjsonrpclocalserver.cpp \
    $${PWD}/qjsonrpcmessage.cpp \
    $${PWD}/qjsonrpcservice.cpp \
    $${PWD}/qjsonrpcservicereply.cpp \
    $${PWD}/qjsonrpcsocket.cpp \
    $${PWD}/qjsonrpctcpserver.cpp

INCLUDEPATH += $${PWD}
