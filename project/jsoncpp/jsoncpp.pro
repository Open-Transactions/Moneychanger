#-------------------------------------------------
#
# Json C++ Project File
#
#-------------------------------------------------

TEMPLATE    = lib
CONFIG     += staticlib

TARGET      = jsoncpp
#VERSION     = 0.6.0


#-------------------------------------------------
# Common Settings

include(../common.pri)

#-------------------------------------------------
# Source

include($${SOLUTION_DIR}../src/jsoncpp/jsoncpp.pri)

#-------------------------------------------------
# Include

INCLUDEPATH += $${SOLUTION_DIR}../src/jsoncpp
