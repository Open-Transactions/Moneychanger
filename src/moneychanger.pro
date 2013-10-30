#-------------------------------------------------
#
# Project created by QtCreator 2013-06-08T22:08:39
#
#-------------------------------------------------

TEMPLATE    = app
TARGET      = moneychanger-qt
#VERSION     =
INCLUDEPATH+= Handlers UI Widgets
QT         += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES    += "OT_ZMQ_MODE=1"

DEPENDPATH += Handlers UI Widgets
HEADERS += moneychanger.h \
           ot_worker.h \
    MTRecord.hpp \
    MTRecordList.hpp \
    Handlers/contacthandler.h \
    Handlers/FileHandler.h \
    Handlers/DBHandler.h \
    Widgets/marketwindow.h \
    Widgets/overviewwindow.h \
    Widgets/withdrawascashwindow.h \
    Widgets/withdrawasvoucherwindow.h \
    Widgets/depositwindow.h \
    Widgets/sendfundswindow.h \
    Widgets/requestfundswindow.h \
    Widgets/home.h \
    Widgets/homedetail.h \
    Widgets/createinsurancecompany.h \
    Widgets/dlgchooser.h \
    Widgets/compose.h \
    Widgets/overridecursor.h \
    Widgets/detailedit.h \
    Widgets/editdetails.h \
    Widgets/contactdetails.h \
    Widgets/identifierwidget.h \
    Widgets/senddlg.h \
    Widgets/requestdlg.h \
    Widgets/nymdetails.h \
    Widgets/serverdetails.h \
    Widgets/assetdetails.h \
    Widgets/accountdetails.h \
    UI/getstringdialog.h \
    UI/dlgnewcontact.h

SOURCES += main.cpp\
           moneychanger.cpp \
           ot_worker.cpp \
           MTRecordList.cpp \
           MTRecord.cpp \
    Handlers/FileHandler.cpp \
    Handlers/DBHandler.cpp \
    Handlers/contacthandler.cpp \
    Widgets/marketwindow.cpp \
    Widgets/overviewwindow.cpp \
    Widgets/withdrawascashwindow.cpp \
    Widgets/withdrawasvoucherwindow.cpp \
    Widgets/depositwindow.cpp \
    Widgets/sendfundswindow.cpp \
    Widgets/requestfundswindow.cpp \
    Widgets/home.cpp \
    Widgets/homedetail.cpp \
    Widgets/createinsurancecompany.cpp \
    Widgets/dlgchooser.cpp \
    Widgets/compose.cpp \
    Widgets/overridecursor.cpp \
    Widgets/detailedit.cpp \
    Widgets/editdetails.cpp \
    Widgets/contactdetails.cpp \
    Widgets/identifierwidget.cpp \
    Widgets/senddlg.cpp \
    Widgets/requestdlg.cpp \
    Widgets/nymdetails.cpp \
    Widgets/serverdetails.cpp \
    Widgets/assetdetails.cpp \
    Widgets/accountdetails.cpp \
    UI/dlgnewcontact.cpp \
    UI/getstringdialog.cpp

RESOURCES += resource.qrc

FORMS += \
    Widgets/home.ui \
    Widgets/homedetail.ui \
    Widgets/dlgchooser.ui \
    Widgets/compose.ui \
    Widgets/detailedit.ui \
    Widgets/editdetails.ui \
    Widgets/contactdetails.ui \
    Widgets/identifierwidget.ui \
    Widgets/senddlg.ui \
    Widgets/requestdlg.ui \
    Widgets/nymdetails.ui \
    Widgets/serverdetails.ui \
    Widgets/assetdetails.ui \
    Widgets/accountdetails.ui \
    UI/dlgnewcontact.ui \
    UI/marketwindow.ui \
    UI/createinsurancecompany.ui \
    UI/getstringdialog.ui

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
	LIBS += -ldl
}

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

