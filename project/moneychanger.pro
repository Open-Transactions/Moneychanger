
TEMPLATE = subdirs
CONFIG  += ordered

SUBDIRS += \
##         qjsonrpc		\ ## Put this back in when someone wants to use it.
           jsoncpp		\
           bitcoin-api		\
           moneychanger-qt
