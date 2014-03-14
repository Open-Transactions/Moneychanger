#-------------------------------------------------
#
# LibIDN Project File
#
#-------------------------------------------------

TEMPLATE    = lib
CONFIG     += staticlib

TARGET      = libidn
#VERSION     = 1.28.0

DEFINES += "IDNA_EXPORTS" "HAVE_CONFIG_H" "LIBIDN_BUILDING" "LIBIDN_STATIC"

#-------------------------------------------------
# Common Settings

include(../common.pri)


#-------------------------------------------------
# Source

include($${SOLUTION_DIR}../src/libidn/libidn/libidn.pri)
include($${SOLUTION_DIR}../src/libidn/libidn/gl/gl.pri)

#-------------------------------------------------
# Include

include($${SOLUTION_DIR}../src/libidn/windows/idn_windows.pri)

