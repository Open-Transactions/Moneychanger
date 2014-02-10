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
# Options

QMAKE_CFLAGS_WARN_ON -= -Wall -Wextra -Wunused-parameter -Wunused-function -Wunneeded-internal-declaration
QMAKE_CXXFLAGS_WARN_ON -= -Wall -Wextra -Wunused-parameter -Wunused-function -Wunneeded-internal-declaration

win32:{
    DEFINES     += "_UNICODE" "NOMINMAX"
    CharacterSet = 1
    QMAKE_CXXFLAGS += /bigobj /Zm480 /wd4512 /wd4100
}

unix:{
    CONFIG += link_pkgconfig
    QMAKE_CXXFLAGS += -fPIC ## put only here, sub-libs pick it up from elsewhere?
}

mac:{
    QT_CONFIG -= no-pkg-config
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.7 -static
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7

    #we do it this way, since we don't want any more tokens.
    PKG_CONFIG_LIBDIR = "/usr/local/lib/pkgconfig:$${PKG_CONFIG_LIBDIR}"
    PKG_CONFIG_LIBDIR = "/usr/local/opt/openssl/lib/pkgconfig:$${PKG_CONFIG_LIBDIR}"
    PKG_CONFIG_LIBDIR = "$${PKG_CONFIG_LIBDIR}:" #end with a colon.

    contains(MAC_OS_VERSION, 13.0.0):{
        QT_CONFIG += -spec macx-clang-libc++
        CONFIG += c++11
        QMAKE_CXXFLAGS += -stdlib=libc++ -std=c++11
    }
    else:{
        MAC_SDK  = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk
        QMAKE_MAC_SDK=macosx10.8
        !exists($$MAC_SDK): error("The selected Mac OSX SDK does not exist at $${MAC_SDK}!")
    }
}

#-------------------------------------------------
# Package Config

unix:{
    PKGCONFIG += opentxs
    PKGCONFIG += chaiscript
}


#-------------------------------------------------
# Include

INCLUDEPATH += $${SOLUTION_DIR}../src
INCLUDEPATH += $${SOLUTION_DIR}../src/jsoncpp
INCLUDEPATH += $${SOLUTION_DIR}../src/bitcoin-api

win32:{
    INCLUDEPATH += $${SOLUTION_DIR}../../Open-Transactions/include
    INCLUDEPATH += $${SOLUTION_DIR}../../Open-Transactions/include/otlib

    equals(TEMPLATE,vcapp):{
        INCLUDEPATH += $(SystemDrive)/OpenSSL-Win$(PlatformArchitecture)/include
        }
    else:{
        !contains(QMAKE_HOST.arch, x86_64):{
            INCLUDEPATH += C:/OpenSSL-Win32/include
        }
        else:{
            INCLUDEPATH += C:/OpenSSL-Win64/include
        }
    }
}

mac:{
    !contains(MAC_OS_VERSION, 13.0.0): {
        INCLUDEPATH += $$QMAKE_MAC_SDK/System/Library/Frameworks/CoreFoundation.framework/Versions/A/Headers
    }
}


#-------------------------------------------------
# Linked Libs

# QJsonRpc
##LIBS += -L$${SOLUTION_DIR}qjsonrpc -lqjsonrpc

unix: {
    LIBS += -L$${SOLUTION_DIR}bitcoin-api
    LIBS += -L$${SOLUTION_DIR}jsoncpp
    LIBS += -L$${SOLUTION_DIR}curl

    LIBS += -ldl
    LIBS += -lbitcoin-api
    LIBS += -ljsoncpp
    LIBS += -lcurl

    LIBS += -lboost_system-mt
    LIBS += -lboost_thread-mt
}

win32: {
    QMAKE_LIBDIR += $${DESTDIR}

    equals(TEMPLATE,vcapp):{
        QMAKE_LIBDIR += $(SystemDrive)/OpenSSL-Win$(PlatformArchitecture)/lib/VC
        QMAKE_LIBDIR += $${SOLUTION_DIR}../../Open-Transactions/lib/$(PlatformName)/$(Configuration)/
    }
    else:{
        !contains(QMAKE_HOST.arch, x86_64):{
            QMAKE_LIBDIR += C:/OpenSSL-Win32/lib/VC
            CONFIG(debug, debug|release):{
                QMAKE_LIBDIR += $${SOLUTION_DIR}../../Open-Transactions/lib/Win32/Debug/
            }
            else:{
                QMAKE_LIBDIR += $${SOLUTION_DIR}../../Open-Transactions/lib/Win32/Release/
            }
        }
        else:{
            QMAKE_LIBDIR += C:/OpenSSL-Win64/lib/VC
            CONFIG(debug, debug|release):{
                QMAKE_LIBDIR += $${SOLUTION_DIR}../../Open-Transactions/lib/x64/Debug/
            }
            else:{
                QMAKE_LIBDIR += $${SOLUTION_DIR}../../Open-Transactions/lib/x64/Release/
            }
        }
    }

    LIBS += bitcoin-api.lib
    LIBS += jsoncpp.lib
    LIBS += curl.lib

    LIBS += otlib.lib
    LIBS += otapi.lib

    LIBS += Advapi32.lib
}

mac:{
    LIBS += -mmacosx-version-min=10.7
    LIBS += -framework CoreFoundation

    contains(MAC_OS_VERSION, 13.0.0):{
        LIBS += -stdlib=libc++
    }
}



