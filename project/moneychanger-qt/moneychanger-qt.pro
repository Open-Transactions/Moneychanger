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
win32:CONFIG += console

TARGET      = moneychanger-qt
VERSION     = 0.0.1

QT         += core gui sql network widgets

DEFINES    += "OT_ZMQ_MODE=1"
DEFINES    += "EXPORT="

#-------------------------------------------------
# Common Settings

include(../common.pri)

#-------------------------------------------------
# Source

#PRECOMPILED_HEADER = $${SOLUTION_DIR}../src/core/stable.hpp

include($${SOLUTION_DIR}../project/QtQREncoder/QtQREncoder.pri)
include($${SOLUTION_DIR}../project/qjsonrpc/qjsonrpc.pri)
include($${SOLUTION_DIR}../src/core/core.pri)
include($${SOLUTION_DIR}../src/gui/gui.pri)
include($${SOLUTION_DIR}../src/rpc/rpc.pri)
include($${SOLUTION_DIR}../src/bitcoin/bitcoin.pri)
include($${SOLUTION_DIR}../src/namecoin/namecoin.pri)
include($${SOLUTION_DIR}../src/quazip/quazip.pri)

#-------------------------------------------------
# Package Config

# MAC AND LINUX:
unix:{
    PKGCONFIG += libzmq
}


#-------------------------------------------------
# Include

win32:{
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
    !contains(MAC_OS_VERSION, 10.9):!contains(MAC_OS_VERSION, 10.10): {
        INCLUDEPATH += $$QMAKE_MAC_SDK/System/Library/Frameworks/CoreFoundation.framework/Versions/A/Headers
    }
}

#-------------------------------------------------
# Linked Libs

LIBS += -lopentxs
LIBS += -lopentxs-proto
LIBS += -lprotobuf-lite

# MAC AND LINUX:
unix: {

##  LIBS += -L$${OUT_PWD}/../curl
##  LIBS += -lcurl

    LIBS += -L$${OUT_PWD}/../bitcoin-api
    LIBS += -lbitcoin-api

    LIBS += -L$${OUT_PWD}/../jsoncpp
    LIBS += -ljsoncpp

    INCLUDEPATH += $${PWD}/../qjsonrpc/src
    LIBS += -L$${OUT_PWD}/../qjsonrpc/src
    LIBS += -lqjsonrpc

    INCLUDEPATH += $${PWD}/../QtQREncoder
    INCLUDEPATH += $${PWD}/../QtQREncoder/qrencode
    LIBS += -L$${OUT_PWD}/../QtQREncoder
    LIBS += -lqrencode

    LIBS += -L$${OUT_PWD}/../nmcrpc
    LIBS += -lnmcrpc

    LIBS += -L$${OUT_PWD}/../quazip
    LIBS += -lquazip -lz


    mac:{

        QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@executable_path/../Frameworks/

        !contains(MAC_OS_VERSION, 10.9):!contains(MAC_OS_VERSION, 10.10):{
            # if not on Mavericks
            LIBS += -lboost_system-mt
            LIBS += -lboost_thread-mt
            LIBS += -lboost_chrono-mt
            LIBS += -lboost_atomic-mt
        }
        LIBS += -L/usr/local/lib/
        LIBS += -framework Cocoa -framework CoreFoundation
    }

    # LINUX:
    else: {
        lessThan(GCC_VERSION, 4.7):{
            # only need if no C++11 available
            LIBS += -lboost_system-mt
            LIBS += -lboost_thread-mt
            LIBS += -lboost_chrono-mt
            LIBS += -lboost_atomic-mt
        }
    }

    LIBS += -lzmq   # needed for sampleescrowserverzmq

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
    LIBS += qjsonrpc.lib
    LIBS += qrencode.lib
    LIBS += curl.lib
    LIBS += nmcrpc.lib
    LIBS += quazip.lib

    LIBS += otlib.lib
    LIBS += otapi.lib

    LIBS += Advapi32.lib
}




# MAC AND LINUX:
# need to put -ldl last.
unix:{
    LIBS += -ldl
    LIBS += -lcurl
    LIBS += -lxmlrpc
    LIBS += -lxmlrpc++
    LIBS += -lxmlrpc_client++
}



