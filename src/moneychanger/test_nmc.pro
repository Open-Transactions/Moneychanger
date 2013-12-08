#-------------------------------------------------
# Namecoin credentials verifier.
#-------------------------------------------------

TEMPLATE    = app
TARGET      = test_nmc
INCLUDEPATH+= Handlers UI
QT         += core sql

HEADERS += Namecoin.hpp \
           Handlers/DBHandler.h Handlers/DBHandler.tpp \
           Handlers/FileHandler.h \
           UI/dlgpassword.h

QMAKE_CXXFLAGS += -std=c++11
QMAKE_LIBS += -lnmcrpc -ljsoncpp -lcurl

SOURCES += Namecoin.cpp \
           test_nmc.cpp \
           Handlers/DBHandler.cpp \
           Handlers/FileHandler.cpp \
           UI/dlgpassword.cpp

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opentxs
unix: PKGCONFIG += chaiscript
