#-------------------------------------------------
#
# Moneychanger Project File
#
#-------------------------------------------------

TEMPLATE    = lib

TARGET      = bitcoin-api
#VERSION     = 0.0.1

QT         += core network

#-------------------------------------------------
# Common Settings

include(../common.pri)

#-------------------------------------------------
# Source

include($${SOLUTION_DIR}../src/bitcoin-api/bitcoin-api.pri)


#-------------------------------------------------
# Include

INCLUDEPATH += $${SOLUTION_DIR}../src/bitcoin-api
INCLUDEPATH += $${SOLUTION_DIR}../src/jsoncpp

 
#-------------------------------------------------
# Linked Libraries

LIBS += -L$${SOLUTION_DIR}jsoncpp -ljsoncpp
LIBS += -lcurl

#-------------------------------------------------
# Options

# uncomment one of the following two lines:
##QMAKE_CXXFLAGS += -std=c++11
DEFINES += OT_USE_TR1

win32:{
    DEFINES     += "_UNICODE=1"
    CharacterSet = 1
}
