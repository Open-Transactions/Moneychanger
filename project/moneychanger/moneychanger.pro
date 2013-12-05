#-------------------------------------------------
#
# Moneychanger Project File
#
#-------------------------------------------------

# note:  make sure you read: http://www.qtcentre.org/wiki/index.php?title=Undocumented_qmake
# so many functions that are not documented.

#-------------------------------------------------
# Global

TEMPLATE    = app

TARGET      = moneychanger-qt
#VERSION     =

QT         += core gui sql network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES    += "OT_ZMQ_MODE=1"

#-------------------------------------------------
# Objects

MOC_DIR        = $${SOLUTION_DIR}../obj/$${TARGET}
OBJECTS_DIR    = $${SOLUTION_DIR}../obj/$${TARGET}


#-------------------------------------------------
# Output

RCC_DIR        = $${SOLUTION_DIR}../out/$${TARGET}/resources
UI_HEADERS_DIR = $${SOLUTION_DIR}../out/$${TARGET}/ui/include
UI_SOURCES_DIR = $${SOLUTION_DIR}../out/$${TARGET}/ui/src


#-------------------------------------------------
# Target

DESTDIR = $${SOLUTION_DIR}../lib/$${TARGET}


#-------------------------------------------------
# Source


include($${SOLUTION_DIR}../src/core/core.pri)
include($${SOLUTION_DIR}../src/gui/gui.pri)


#-------------------------------------------------
# Include

INCLUDEPATH += $${SOLUTION_DIR}../src
INCLUDEPATH += $${SOLUTION_DIR}../include
INCLUDEPATH += $${SOLUTION_DIR}../include/opentxs


#-------------------------------------------------
# Options




    
mac:{

	OS_VERSION = $$system(uname -r)        
	QT_CONFIG -= no-pkg-config
        !contains(OS_VERSION, 13.0.0):LIBS += -lboost_system-mt -lboost_thread-mt -ldl
	contains(OS_VERSION, 13.0.0):LIBS += -lboost_system -lboost_thread -ldl

    # -------------------------------------------
    # NOTE: This is necessary on Mac OSX Mavericks (10.9)
    #	Because libc++ is now chosen by default over libstdc++
    #   And your dependencies will have to be rebuilt with similar options.
    #
	contains(OS_VERSION, 13.0.0):QT_CONFIG += -spec macx-clang-libc++
	contains(OS_VERSION, 13.0.0):LIBS += -stdlib=libc++ -mmacosx-version-min=10.7 
	contains(OS_VERSION, 13.0.0):CONFIG += c++11
	contains(OS_VERSION, 13.0.0):QMAKE_CXXFLAGS += -mmacosx-version-min=10.7 -stdlib=libc++ -std=c++11 -static
	QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7

	MAC_SDK  = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk
	if( !exists( $$MAC_SDK) ) {
	  error("The selected Mac OSX SDK does not exist at $$MAC_SDK!")
	}
	QMAKE_MAC_SDK=macosx10.8
	INCLUDEPATH += $$QMAKE_MAC_SDK/System/Library/Frameworks/CoreFoundation.framework/Versions/A/Headers
	DEPENDPATH  += $$QMAKE_MAC_SDK/System/Library/Frameworks/CoreFoundation.framework/Versions/A/Headers
	LIBS += -framework CoreFoundation
    #
    # -------------------------------------------
}

linux:{
        LIBS += -ldl
}


#QMAKE_CXXFLAGS_WARN_ON  = ""
#QMAKE_CXXFLAGS_WARN_OFF = ""

QMAKE_CFLAGS_WARN_ON -= -Wall -Wunused-parameter -Wunused-function -Wunneeded-internal-declaration
QMAKE_CXXFLAGS_WARN_ON -= -Wall -Wunused-parameter -Wunused-function -Wunneeded-internal-declaration

#QMAKE_CXXFLAGS  -= -Wunused-parameter -Wunused-function -Wunneeded-internal-declaration
#QMAKE_CXXFLAGS  += -Wno-unused-parameter -Wno-unused-function -Wno-unneeded-internal-declaration


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

CONFIG += debug_and_release

