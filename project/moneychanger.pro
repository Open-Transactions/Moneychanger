
TEMPLATE = subdirs
CONFIG  += ordered

win32: SUBDIRS += curl

#SUBDIRS += qjsonrpc ## Put this back in when someone wants to use it.

SUBDIRS += jsoncpp
SUBDIRS += bitcoin-api
SUBDIRS += nmcrpc
SUBDIRS += moneychanger-qt
