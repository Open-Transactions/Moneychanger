#-------------------------------------------------
#
# Curl Project File
#
#-------------------------------------------------

TEMPLATE    = lib
CONFIG     += staticlib

TARGET      = curl
#VERSION     = 0.6.0

DEFINES += "CURL_STATICLIB=1" "BUILDING_LIBCURL=1" "HTTP_ONLY=1"

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

