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
# Common Settings

include($${SOLUTION_DIR}common.pri)

#-------------------------------------------------
# Source


include($${SOLUTION_DIR}../src/qjsonrpc/qjsonrpc.pri)


lessThan(QT_MAJOR_VERSION, 5): include($${SOLUTION_DIR}../src/qjsonrpc/json/json.pri)



#-------------------------------------------------
# Include

INCLUDEPATH += $${SOLUTION_DIR}../include/qjsonrpc

#-------------------------------------------------
# Options

win32:{
    DEFINES     += "_UNICODE=1"
    CharacterSet = 1
}
