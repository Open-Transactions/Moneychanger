
CONFIG     += debug_and_release silent

SOLUTION_DIR=$${PWD}/

INCLUDEPATH += $${SOLUTION_DIR}../src
INCLUDEPATH += $${SOLUTION_DIR}../src/quazip

#-------------------------------------------------
# Output

win32:{
    INCLUDEPATH += $${SOLUTION_DIR}../src/curl/include
    INCLUDEPATH += $${SOLUTION_DIR}../src/libidn/libidn
    INCLUDEPATH += $${SOLUTION_DIR}../src/libidn/windows/include

    equals(TEMPLATE,vcapp)|equals(TEMPLATE,vclib):{
        DESTDIR     = $${SOLUTION_DIR}../lib/$(PlatformName)/$(Configuration)
        MOC_DIR     = $${SOLUTION_DIR}../obj/$${TARGET}
        OBJECTS_DIR = $${SOLUTION_DIR}../obj/$${TARGET}
        RCC_DIR     = $${SOLUTION_DIR}../out/$${TARGET}/resources
        UI_DIR      = $${SOLUTION_DIR}../out/$${TARGET}/ui/
    }
    else:{
        !contains(QMAKE_HOST.arch, x86_64):{
            CONFIG(debug, debug|release):{
                DESTDIR = $${SOLUTION_DIR}../lib/Win32/Debug
            }
            else:{
                DESTDIR = $${SOLUTION_DIR}../lib/Win32/Debug
            }
        }
        else:{
            CONFIG(debug, debug|release):{
                DESTDIR = $${SOLUTION_DIR}../lib/x64/Debug
            }
            else:{
                DESTDIR = $${SOLUTION_DIR}../lib/x64/Debug
            }
        }
    }
}


#-------------------------------------------------
# Options

QMAKE_CFLAGS_WARN_ON -= -Wall -Wextra -Wunused-parameter -Wunused-function -Wunneeded-internal-declaration
QMAKE_CXXFLAGS_WARN_ON -= -Wall -Wextra -Wunused-parameter -Wunused-function -Wunneeded-internal-declaration

DEFINES     += OT_CRYPTO_USING_OPENSSL

win32:{
    QMAKE_CXXFLAGS += -std=c++17
    DEFINES     += "_UNICODE" "NOMINMAX"
    CharacterSet = 1
    QMAKE_CXXFLAGS += /bigobj /Zm480 /wd4512 /wd4100
}


unix:{
    QMAKE_CXXFLAGS += -fPIC

    mac:{
        QMAKE_CXXFLAGS += -std=c++1z -Wno-dynamic-exception-spec
        CONFIG += c++1z

        MAC_OS_VERSION = $$system(sw_vers -productVersion)
        MAC_OS_VERSION ~= s/\([0-9]*.[0-9]*\).*/\1/

        QMAKE_MACOSX_DEPLOYMENT_TARGET = $${MAC_OS_VERSION}
        CONFIG += link_pkgconfig

        QT_CONFIG -= no-pkg-config

        #we do it this way, since we don't want any more tokens.
        PKG_CONFIG_LIBDIR = "/usr/local/lib/pkgconfig:" #end with a colon.
        PKG_CONFIG_LIBDIR = "/usr/local/opt/openssl/lib/pkgconfig:$${PKG_CONFIG_LIBDIR}"
        PKG_CONFIG_LIBDIR = "/usr/local/opt/qt/lib/pkgconfig:$${PKG_CONFIG_LIBDIR}"
    }

    # LINUX:
    else:{
        QMAKE_CXXFLAGS += -std=c++17
        CONFIG += link_pkgconfig
    }
}



