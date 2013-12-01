DEPTH = ../../..
include($${DEPTH}/qjsonrpc.pri)
include($${DEPTH}/tests/tests.pri)

QT += gui
TEMPLATE = app
TARGET = server
HEADERS = testservice.h
SOURCES = testservice.cpp \
          localserver.cpp
