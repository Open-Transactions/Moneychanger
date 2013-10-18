#-------------------------------------------------
#
# Project created by QtCreator 2013-06-08T22:08:39
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = moneychanger-qt
TEMPLATE = app

INCLUDEPATH+="/usr/local/include/"
DEPENDPATH += .
SOURCES += main.cpp\
           moneychanger.cpp \
           ot_worker.cpp \
           MTRecordList.cpp \
           MTRecord.cpp \
    Widgets/MarketWindow.cpp \
    Widgets/overviewwindow.cpp \
    Handlers/FileHandler.cpp \
    Handlers/DBHandler.cpp \
    Widgets/addressbookwindow.cpp \
    Widgets/nymmanagerwindow.cpp \
    Widgets/assetmanagerwindow.cpp \
    Widgets/accountmanagerwindow.cpp \
    Widgets/servermanagerwindow.cpp \
    Widgets/withdrawascashwindow.cpp \
    Widgets/withdrawasvoucherwindow.cpp \
    Widgets/depositwindow.cpp \
    Widgets/sendfundswindow.cpp \
    Widgets/requestfundswindow.cpp \
    Widgets/home.cpp \
    Widgets/homedetail.cpp \
    Handlers/contacthandler.cpp \
    Widgets/createinsurancecompany.cpp \
    Widgets/dlgchooser.cpp \
    Widgets/compose.cpp \
    Widgets/overridecursor.cpp \
    Widgets/detailedit.cpp \
    Widgets/editdetails.cpp \
    Widgets/contactdetails.cpp \
    UI/getstringdialog.cpp \
    Widgets/identifierwidget.cpp \
    UI/dlgnewcontact.cpp \
    Widgets/senddlg.cpp \
    Widgets/requestdlg.cpp


HEADERS += moneychanger.h \
           ot_worker.h \
    MTRecord.hpp \
    MTRecordList.hpp \
    Widgets/marketwindow.h \
    Widgets/overviewwindow.h \
    Handlers/FileHandler.h \
    Handlers/DBHandler.h \
    Widgets/addressbookwindow.h \
    Widgets/nymmanagerwindow.h \
    Widgets/assetmanagerwindow.h \
    Widgets/accountmanagerwindow.h \
    Widgets/servermanagerwindow.h \
    Widgets/withdrawascashwindow.h \
    Widgets/withdrawasvoucherwindow.h \
    Widgets/depositwindow.h \
    Widgets/sendfundswindow.h \
    Widgets/requestfundswindow.h \
    Widgets/home.h \
    Widgets/homedetail.h \
    Handlers/contacthandler.h \
    Widgets/createinsurancecompany.h \
    Widgets/dlgchooser.h \
    Widgets/compose.h \
    Widgets/overridecursor.h \
    Widgets/detailedit.h \
    Widgets/editdetails.h \
    Widgets/contactdetails.h \
    UI/getstringdialog.h \
    Widgets/identifierwidget.h \
    UI/dlgnewcontact.h \
    Widgets/senddlg.h \
    Widgets/requestdlg.h


DEFINES += "OT_ZMQ_MODE=1"

mac:{
	QT_CONFIG -= no-pkg-config
	LIBS += -lboost_system-mt -lboost_thread-mt -ldl

    # -------------------------------------------
    # Un-comment this block to use C++11.
    #
    # Comment-out this block to deactivate C++11
    #
	# QT_CONFIG += -spec macx-clang-libc++
    # LIBS += -stdlib=libc++
    # CONFIG += c++11
    # QMAKE_CXXFLAGS += -mmacosx-version-min=10.7 -stdlib=libc++ -std=c++11
    #
    # -------------------------------------------


# Stuff that didn't work.
#    QMAKE_CXXFLAGS += -spec macx-clang-libc++  # apparently this doesn't really exist.
#    CXXFLAGS += -std=c++0x // apparently the qmake version of this above is the one I'm supposed to use, not this...
}


linux:{
	LIBS += -lboost_system -lboost_thread -ldl
}

##QMAKE_CXXFLAGS += -fPIC -DPIC --param ssp-buffer-size=4

# Set libraries and includes at end, to use platform-defined defaults if not overridden

# Dependency library locations can be customized with:
#    OPENTXS_INCLUDE_PATH, OPENTXS_LIB_PATH, CHAI_INCLUDE_PATH,
#    OPENSSL_INCLUDE_PATH and OPENSSL_LIB_PATH respectively

##OPENTXS_INCLUDE_PATH	+=
##OPENTXS_LIB_PATH	+=
##CHAI_INCLUDE_PATH	+=
##OPENSSL_INCLUDE_PATH	+=
##OPENSSL_LIB_PATH	+=

##INCLUDEPATH	+= $$OPENTXS_INCLUDE_PATH $$CHAI_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH
##DEPENDPATH	+= $$OPENTXS_INCLUDE_PATH $$CHAI_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH
##LIBS		+= $$join(OPENTXS_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,)
##LIBS 		+= -lotapi -lssl -lcrypto

## Automatic path from pkg-config

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opentxs

unix: PKGCONFIG += chaiscript

#OTHER_FILES +=

RESOURCES += resource.qrc

OTHER_FILES +=

FORMS += \
    UI/marketwindow.ui \
    Widgets/home.ui \
    Widgets/homedetail.ui \ 
    UI/createinsurancecompany.ui \
    Widgets/dlgchooser.ui \
    Widgets/compose.ui \
    Widgets/detailedit.ui \
    Widgets/editdetails.ui \
    Widgets/contactdetails.ui \
    UI/getstringdialog.ui \
    Widgets/identifierwidget.ui \
    UI/dlgnewcontact.ui \
    Widgets/senddlg.ui \
    Widgets/requestdlg.ui

