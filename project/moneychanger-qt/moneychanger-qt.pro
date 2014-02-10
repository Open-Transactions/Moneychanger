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
    LIBS += -ldl

    LIBS += -L$${OUT_PWD}/../bitcoin-api
    LIBS += -lbitcoin-api

    LIBS += -L$${OUT_PWD}/../jsoncpp
    LIBS += -ljsoncpp

    LIBS += -L$${OUT_PWD}/../curl
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



