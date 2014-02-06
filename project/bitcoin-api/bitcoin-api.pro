#-------------------------------------------------
#
# Moneychanger Project File
#
#-------------------------------------------------

#-------------------------------------------------
# Global

TEMPLATE    = lib

TARGET      = bitcoin-api
#VERSION     =

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


 
#-------------------------------------------------
# Linked Libraries

unix:LIBS += -lcurl -ljsoncpp                   ## cross-platform communication with bitcoind
mac: LIBS += -lcurl /usr/local/lib/libjsoncpp.a ## custom jsoncpp build

unix:INCLUDEPATH += /usr/include/jsoncpp       ## assume system jsoncpp build
mac: INCLUDEPATH += /usr/local/include/jsoncpp ## custom jsoncpp build

#-------------------------------------------------
# Options

# uncomment one of the following two lines:
##QMAKE_CXXFLAGS += -std=c++11
DEFINES += OT_USE_TR1

win32:{
    DEFINES     += "_UNICODE=1"
    CharacterSet = 1
}
