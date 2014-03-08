#-------------------------------------------------
#
# Moneychanger Project File
#
#-------------------------------------------------

TEMPLATE    = lib
CONFIG     += staticlib

TARGET      = nmcrpc
#VERSION     =

#-------------------------------------------------
# Common Settings

include(../common.pri)


#-------------------------------------------------
# Source

include($${SOLUTION_DIR}../src/nmcrpc/nmcrpc.pri)


#-------------------------------------------------
# Include

INCLUDEPATH += $${SOLUTION_DIR}../src/jsoncpp


#-------------------------------------------------
# Package Config
unix:{
    PKGCONFIG += libidn
}

