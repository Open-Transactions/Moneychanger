#-------------------------------------------------
#
# Bitcoin API Project File
#
#-------------------------------------------------

TEMPLATE    = lib
CONFIG     += precompile_header
CONFIG     += staticlib

TARGET      = bitcoin-api
#VERSION     = 0.0.1

win32:DEFINES += "CURL_STATICLIB=1" "STATICLIB=1"

#-------------------------------------------------
# Common Settings

include(../common.pri)

#-------------------------------------------------
# Source

PRECOMPILED_HEADER = $${SOLUTION_DIR}../src/core/stable.hpp

include($${SOLUTION_DIR}../src/bitcoin-api/bitcoin-api.pri)


#-------------------------------------------------
# Options


win32:{
    DEFINES     += "_UNICODE=1"
    CharacterSet = 1
}

#-------------------------------------------------
# Package Config
unix:{
    PKGCONFIG += opentxs
}
