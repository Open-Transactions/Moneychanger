
PRECOMPILED_HEADER = $${PWD}/stable.h

HEADERS += \
    $${PWD}/applicationmc.h \
    $${PWD}/filedownloader.h \
    $${PWD}/httpinterface.h \
    $${PWD}/moneychanger.h \
    $${PWD}/MTRecord.hpp \
    $${PWD}/MTRecordList.hpp \
    $${PWD}/ot_worker.h \
    $${PWD}/passwordcallback.h \
    $${PWD}/translation.h \
    $${PWD}/handlers/contacthandler.h \
    $${PWD}/handlers/DBHandler.h \
    $${PWD}/handlers/FileHandler.h

SOURCES += \
    $${PWD}/applicationmc.cpp \
    $${PWD}/filedownloader.cpp \
    $${PWD}/httpinterface.cpp \
    $${PWD}/main.cpp \
    $${PWD}/moneychanger.cpp \
    $${PWD}/MTRecord.cpp \
    $${PWD}/MTRecordList.cpp \
    $${PWD}/ot_worker.cpp \
    $${PWD}/passwordcallback.cpp \
    $${PWD}/translation.cpp \
    $${PWD}/handlers/contacthandler.cpp \
    $${PWD}/handlers/DBHandler.cpp \
    $${PWD}/handlers/FileHandler.cpp

INCLUDEPATH += $${PWD}
