
PRECOMPILED_HEADER = $${PWD}/stable.hpp

HEADERS += \
    $${PWD}/stable.hpppp \
    $${PWD}/applicationmc.h \
    $${PWD}/filedownloader.h \
    $${PWD}/FastDelegate.h \
    $${PWD}/FastDelegateBind.h \
    $${PWD}/httpinterface.h \
    $${PWD}/modules.h \
    $${PWD}/moneychanger.h \
    $${PWD}/MTRecord.hpp \
    $${PWD}/MTRecordList.hpp \
    $${PWD}/ot_worker.h \
    $${PWD}/passwordcallback.h \
    $${PWD}/translation.h \
    $${PWD}/utils.h \
    $${PWD}/handlers/contacthandler.h \
    $${PWD}/handlers/DBHandler.h \
    $${PWD}/handlers/FileHandler.h

SOURCES += \
    $${PWD}/applicationmc.cpp \
    $${PWD}/filedownloader.cpp \
    $${PWD}/httpinterface.cpp \
    $${PWD}/main.cpp \
    $${PWD}/modules.cpp \
    $${PWD}/moneychanger.cpp \
    $${PWD}/MTRecord.cpp \
    $${PWD}/MTRecordList.cpp \
    $${PWD}/ot_worker.cpp \
    $${PWD}/passwordcallback.cpp \
    $${PWD}/translation.cpp \
    $${PWD}/utils.cpp \
    $${PWD}/handlers/contacthandler.cpp \
    $${PWD}/handlers/DBHandler.cpp \
    $${PWD}/handlers/FileHandler.cpp

INCLUDEPATH += $${PWD}
