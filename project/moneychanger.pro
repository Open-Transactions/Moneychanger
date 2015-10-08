
TEMPLATE = subdirs
CONFIG  += ordered

#-------------------------------------------------
# Subdirectories

win32: SUBDIRS += curl
win32: SUBDIRS += libidn

SUBDIRS += jsoncpp
SUBDIRS += bitcoin-api
SUBDIRS += nmcrpc
SUBDIRS += quazip

unix: SUBDIRS += qjsonrpc

SUBDIRS += moneychanger-qt
