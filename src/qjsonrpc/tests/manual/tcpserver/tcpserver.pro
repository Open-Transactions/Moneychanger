DEPTH = ../../..
include($${DEPTH}/qjsonrpc.pri)
include($${DEPTH}/tests/tests.pri)

TEMPLATE = app
TARGET = tcpserver
HEADERS = testservice.h
SOURCES = testservice.cpp \
          tcpserver.cpp
