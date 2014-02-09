#-------------------------------------------------
#
# Moneychanger QT Project File
#
#-------------------------------------------------

# note:  make sure you read: http://www.qtcentre.org/wiki/index.php?title=Undocumented_qmake
# so many functions that are not documented.

#-------------------------------------------------
# Global

TEMPLATE    = app
CONFIG     += precompile_header

TARGET      = moneychanger-qt
#VERSION     = 0.0.1

QT         += core gui sql network widgets

DEFINES    += "OT_ZMQ_MODE=1"

#-------------------------------------------------
# Common Settings

include(../common.pri)

#-------------------------------------------------
# Source

include($${SOLUTION_DIR}../src/core/core.pri)
include($${SOLUTION_DIR}../src/gui/gui.pri)
include($${SOLUTION_DIR}../src/bitcoin/bitcoin.pri)

#-------------------------------------------------
# Include

INCLUDEPATH += $${SOLUTION_DIR}../src
INCLUDEPATH += $${SOLUTION_DIR}../src/jsoncpp
INCLUDEPATH += $${SOLUTION_DIR}../src/bitcoin-api


#-------------------------------------------------
# Linked Libs

# Bitcoin-Api
LIBS += -L$${SOLUTION_DIR}bitcoin-api -lbitcoin-api

# Jsoncpp
LIBS += -L$${SOLUTION_DIR}jsoncpp -ljsoncpp

LIBS += -lcurl ## bitcoin-api dependency

# QJsonRpc
##LIBS += -L$${SOLUTION_DIR}qjsonrpc -lqjsonrpc


#-------------------------------------------------
# Options

QMAKE_CXXFLAGS += -fPIC ## put only here, sub-libs pick it up from elsewhere?

## Windows
win32:{

    INCLUDEPATH += $(SystemDrive)/OpenSSL-Win$(PlatformArchitecture)/include
    LIBPATH += $(SystemDrive)/OpenSSL-Win$(PlatformArchitecture)/lib/VC
    LIBS +=

    INCLUDEPATH += $${SOLUTION_DIR}../../Open-Transactions/include
    INCLUDEPATH += $${SOLUTION_DIR}../../Open-Transactions/include/otlib
    LIBPATH += $${SOLUTION_DIR}../../Open-Transactions/lib/$(PlatformName)/$(Configuration)/
    LIBS += otlib.lib otapi.lib

    DEFINES     += "_UNICODE" "NOMINMAX"
    CharacterSet = 1

    QMAKE_CXXFLAGS += /bigobj /Zm480 /wd4512 /wd4100

}

## Mac OS X
mac:{
    OS_VERSION = $$system(uname -r)

    # this is still a mess! but getting better. Need to remove boost hacks eventually.


    #Common
    LIBS += -ldl -mmacosx-version-min=10.7 -framework CoreFoundation
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.7 -static

    #Mavericks is version 13
    contains(OS_VERSION, 13.0.0):{
        QT_CONFIG += -spec macx-clang-libc++
        CONFIG += c++11
        QMAKE_CXXFLAGS += -stdlib=libc++ -std=c++11
        LIBS += -stdlib=libc++ -lboost_system -lboost_thread
    }

    #Not Mavericks
    !contains(OS_VERSION, 13.0.0): {
        MAC_SDK  = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk
        QMAKE_MAC_SDK=macosx10.8

        INCLUDEPATH += $$QMAKE_MAC_SDK/System/Library/Frameworks/CoreFoundation.framework/Versions/A/Headers
        DEPENDPATH  += $$QMAKE_MAC_SDK/System/Library/Frameworks/CoreFoundation.framework/Versions/A/Headers
        LIBS += -lboost_system-mt -lboost_thread-mt

        if( !exists( $$MAC_SDK) ) {error("The selected Mac OSX SDK does not exist at $$MAC_SDK!")}
    }
}


# Linux

linux:{
        LIBS += -ldl -lboost_system -lboost_thread  ##need to remove these boost hacks eventually
}

QMAKE_CFLAGS_WARN_ON -= -Wall -Wextra -Wunused-parameter -Wunused-function -Wunneeded-internal-declaration
QMAKE_CXXFLAGS_WARN_ON -= -Wall -Wextra -Wunused-parameter -Wunused-function -Wunneeded-internal-declaration

#we do it this way, since we don't want any more tokens.
mac: PKG_CONFIG_LIBDIR = "/usr/local/lib/pkgconfig:$${PKG_CONFIG_LIBDIR}"
mac: PKG_CONFIG_LIBDIR = "/usr/local/opt/openssl/lib/pkgconfig:$${PKG_CONFIG_LIBDIR}"
mac: PKG_CONFIG_LIBDIR = "$${PKG_CONFIG_LIBDIR}:" #end with a colon.

mac:  QT_CONFIG -= no-pkg-config
unix: CONFIG += link_pkgconfig

unix: PKGCONFIG += opentxs
unix: PKGCONFIG += chaiscript

