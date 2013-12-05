#-------------------------------------------------
#
# Moneychanger Project File
#
#-------------------------------------------------

#-------------------------------------------------
# Global

TEMPLATE    = lib

TARGET      = qjsonrpc
#VERSION     =

QT         += network

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


include($${SOLUTION_DIR}../src/qjsonrpc/qjsonrpc.pri)


lessThan(QT_MAJOR_VERSION, 5): include($${SOLUTION_DIR}../src/qjsonrpc/json/json.pri)



#-------------------------------------------------
# Include

INCLUDEPATH += $${SOLUTION_DIR}../include/qjsonrpc

#-------------------------------------------------
# Options

