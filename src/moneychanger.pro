#-------------------------------------------------
#
# Project created by QtCreator 2013-06-08T22:08:39
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = moneychanger-qt
TEMPLATE = app

INCLUDEPATH+="/usr/local/include/"
DEPENDPATH += .
SOURCES += main.cpp\
           moneychanger.cpp \
           ot_worker.cpp \
           MTRecordList.cpp \
           MTRecord.cpp \
    marketwindow.cpp \
    overviewwindow.cpp \
    FileHandler.cpp \
    DBHandler.cpp

HEADERS += moneychanger.h \
           ot_worker.h \
           MTRecordList.h \
           MTRecord.h \
    MTRecord.hpp \
    MTRecordList.hpp \
    marketwindow.h \
    overviewwindow.h \
    FileHandler.h \
    DBHandler.h

DEFINES += "OT_ZMQ_MODE=1"

mac:{
QT_CONFIG -= no-pkg-config
LIBS += -lboost_system-mt -ldl
}

unix:LIBS += /usr/local/lib/libboost_thread-mt.a -ldl

##QMAKE_CXXFLAGS += -fPIC -DPIC --param ssp-buffer-size=4

# Set libraries and includes at end, to use platform-defined defaults if not overridden

# Dependency library locations can be customized with:
#    OPENTXS_INCLUDE_PATH, OPENTXS_LIB_PATH, CHAI_INCLUDE_PATH,
#    OPENSSL_INCLUDE_PATH and OPENSSL_LIB_PATH respectively

##OPENTXS_INCLUDE_PATH	+=
##OPENTXS_LIB_PATH	+=
##CHAI_INCLUDE_PATH	+=
##OPENSSL_INCLUDE_PATH	+=
##OPENSSL_LIB_PATH	+=

##INCLUDEPATH	+= $$OPENTXS_INCLUDE_PATH $$CHAI_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH
##DEPENDPATH	+= $$OPENTXS_INCLUDE_PATH $$CHAI_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH
##LIBS		+= $$join(OPENTXS_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,)
##LIBS 		+= -lotapi -lssl -lcrypto

## Automatic path from pkg-config

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opentxs

unix: PKGCONFIG += chaiscript

#OTHER_FILES +=

RESOURCES += resource.qrc

OTHER_FILES +=

FORMS += \
    marketwindow.ui
