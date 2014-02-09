#-------------------------------------------------
#
# Curl Project File
#
#-------------------------------------------------

TEMPLATE    = lib
CONFIG     += staticlib

TARGET      = curl
#VERSION     = 0.6.0


#-------------------------------------------------
# Common Settings

include(../common.pri)

#-------------------------------------------------
# Source

include($${SOLUTION_DIR}../src/curl/libcurl/libcurl.pri)
include($${SOLUTION_DIR}../src/curl/libcurl/vtls/vtls.pri)

#-------------------------------------------------
# Include

include($${SOLUTION_DIR}../src/curl/include/curl.pri)
