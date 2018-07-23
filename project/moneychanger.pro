
TEMPLATE = subdirs
CONFIG  += ordered

#-------------------------------------------------
# Subdirectories

win32: SUBDIRS += curl
win32: SUBDIRS += libidn

SUBDIRS += quazip

unix: SUBDIRS += QtQREncoder

SUBDIRS += moneychanger-qt
