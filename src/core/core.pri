
HEADERS += \
    $${PWD}/stable.hpp \
    $${PWD}/applicationmc.hpp \
    $${PWD}/filedownloader.hpp \
    $${PWD}/httpinterface.hpp \
    $${PWD}/modules.hpp \
    $${PWD}/moneychanger.hpp \
    $${PWD}/ot_worker.hpp \
    $${PWD}/passwordcallback.hpp \
    $${PWD}/translation.hpp \
    $${PWD}/utils.hpp \
    $${PWD}/handlers/contacthandler.hpp \
    $${PWD}/handlers/DBHandler.hpp \
    $${PWD}/handlers/FileHandler.hpp \
    ../../src/core/network/XmlRPC.h \
    ../../src/core/network/Network.h \
    ../../src/core/network/BitMessage.h \
    ../../src/core/mtcomms.h \
    ../../src/core/network/base64.h \
    ../../src/core/network/BitMessageQueue.h \
    $$PWD/handlers/focuser.h \
    $$PWD/handlers/modeltradearchive.hpp \
    $$PWD/handlers/modelmessages.hpp \
    $$PWD/handlers/modelpayments.hpp

SOURCES += \
    $${PWD}/applicationmc.cpp \
    $${PWD}/filedownloader.cpp \
    $${PWD}/httpinterface.cpp \
    $${PWD}/main.cpp \
    $${PWD}/modules.cpp \
    $${PWD}/moneychanger.cpp \
    $${PWD}/ot_worker.cpp \
    $${PWD}/passwordcallback.cpp \
    $${PWD}/translation.cpp \
    $${PWD}/utils.cpp \
    $${PWD}/handlers/contacthandler.cpp \
    $${PWD}/handlers/DBHandler.cpp \
    $${PWD}/handlers/FileHandler.cpp \
    ../../src/core/network/XmlRPC.cpp \
    ../../src/core/network/Network.cpp \
    ../../src/core/network/BitMessage.cpp \
    ../../src/core/mtcomms.cpp \
    ../../src/core/network/base64.cpp \
    ../../src/core/network/BitMessageQueue.cpp \
    $$PWD/handlers/modeltradearchive.cpp \
    $$PWD/handlers/modelmessages.cpp \
    $$PWD/handlers/modelpayments.cpp

mac: {
  OBJECTIVE_SOURCES += ../../src/core/handlers/focuser.mm
}

linux|win32: {
  SOURCES += ../../src/core/handlers/focuser.cpp
}
