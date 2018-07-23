
HEADERS += \
    $${PWD}/stable.hpp \
    $${PWD}/applicationmc.hpp \
    $${PWD}/filedownloader.hpp \
    $${PWD}/moneychanger.hpp \
    $${PWD}/passwordcallback.hpp \
    $${PWD}/translation.hpp \
    $${PWD}/utils.hpp \
    $${PWD}/handlers/contacthandler.hpp \
    $${PWD}/handlers/DBHandler.hpp \
    $${PWD}/handlers/FileHandler.hpp \
    $$PWD/handlers/focuser.h \
    $$PWD/handlers/modeltradearchive.hpp \
    $$PWD/handlers/modelmessages.hpp \
    $$PWD/handlers/modelpayments.hpp \
    $$PWD/handlers/modelclaims.hpp \
    $$PWD/handlers/modelverifications.hpp \
    $$PWD/mapidname.hpp \
    $$PWD/handlers/modelagreements.hpp \
    $$PWD/handlers/serialportreader.hpp

SOURCES += \
    $${PWD}/applicationmc.cpp \
    $${PWD}/filedownloader.cpp \
    $${PWD}/main.cpp \
    $${PWD}/moneychanger.cpp \
    $${PWD}/passwordcallback.cpp \
    $${PWD}/translation.cpp \
    $${PWD}/utils.cpp \
    $${PWD}/handlers/contacthandler.cpp \
    $${PWD}/handlers/DBHandler.cpp \
    $${PWD}/handlers/FileHandler.cpp \
    $$PWD/handlers/modeltradearchive.cpp \
    $$PWD/handlers/modelmessages.cpp \
    $$PWD/handlers/modelpayments.cpp \
    $$PWD/handlers/modelclaims.cpp \
    $$PWD/handlers/modelverifications.cpp \
    $$PWD/handlers/modelagreements.cpp \
    $$PWD/handlers/serialportreader.cpp

mac: {
  OBJECTIVE_SOURCES += ../../src/core/handlers/focuser.mm
}

linux|win32: {
  SOURCES += ../../src/core/handlers/focuser.cpp
}
