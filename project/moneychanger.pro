
TEMPLATE = subdirs
CONFIG  += ordered

win32: SUBDIRS += curl
win32: SUBDIRS += libidn

SUBDIRS += jsoncpp
SUBDIRS += qjsonrpc
SUBDIRS += bitcoin-api
SUBDIRS += nmcrpc
SUBDIRS += quazip
SUBDIRS += moneychanger-qt
