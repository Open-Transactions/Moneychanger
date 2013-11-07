#-------------------------------------------------
#
# Project created by QtCreator 2013-06-08T22:08:39
#
#-------------------------------------------------

TEMPLATE    = app
TARGET      = moneychanger-qt
#VERSION     =
INCLUDEPATH+= Handlers UI Widgets
QT         += core gui sql network
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
    UI/dlgnewcontact.h \
    translation.h \
    Widgets/settings.h\
    Widgets/pageaddcontract.h \
    Widgets/wizardaddcontract.h \
    Widgets/pageimportcontract.h \
    Widgets/pagecreatecontract.h \
    filedownloader.h \
    Widgets/wizardaddaccount.h \
    Widgets/pageacct_asset.h \
    Widgets/pageacct_server.h \
    Widgets/pageacct_nym.h \
    Widgets/pageacct_name.h \
    Widgets/wizardaddnym.h \
    Widgets/pagenym_authority.h \
    Widgets/pagenym_keysize.h \
    Widgets/pagenym_source.h \
    Widgets/pagenym_altlocation.h \
    passwordcallback.h \
    UI/dlgpassword.h \
    UI/dlgpasswordconfirm.h \
    applicationmc.h \
    Widgets/cashpurse.h \
    Widgets/dlggetamount.h

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
    UI/getstringdialog.cpp \
    translation.cpp \
    Widgets/settings.cpp \
    Widgets/pageaddcontract.cpp \
    Widgets/wizardaddcontract.cpp \
    Widgets/pageimportcontract.cpp \
    Widgets/pagecreatecontract.cpp \
    filedownloader.cpp \
    Widgets/wizardaddaccount.cpp \
    Widgets/pageacct_asset.cpp \
    Widgets/pageacct_server.cpp \
    Widgets/pageacct_nym.cpp \
    Widgets/pageacct_name.cpp \
    Widgets/wizardaddnym.cpp \
    Widgets/pagenym_authority.cpp \
    Widgets/pagenym_keysize.cpp \
    Widgets/pagenym_source.cpp \
    Widgets/pagenym_altlocation.cpp \
    passwordcallback.cpp \
    UI/dlgpassword.cpp \
    UI/dlgpasswordconfirm.cpp \
    applicationmc.cpp \
    Widgets/cashpurse.cpp \
    Widgets/dlggetamount.cpp

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
    UI/getstringdialog.ui \
    Widgets/settings.ui \
    Widgets/pageaddcontract.ui \
    Widgets/wizardaddcontract.ui \
    Widgets/pageimportcontract.ui \
    Widgets/pagecreatecontract.ui \
    Widgets/wizardaddaccount.ui \
    Widgets/pageacct_asset.ui \
    Widgets/pageacct_server.ui \
    Widgets/pageacct_nym.ui \
    Widgets/pageacct_name.ui \
    Widgets/wizardaddnym.ui \
    Widgets/pagenym_authority.ui \
    Widgets/pagenym_keysize.ui \
    Widgets/pagenym_source.ui \
    Widgets/pagenym_altlocation.ui \
    UI/dlgpassword.ui \
    UI/dlgpasswordconfirm.ui \
    Widgets/cashpurse.ui \
    Widgets/dlggetamount.ui

mac:{

	OS_VERSION = $$system(uname -r)
        
	QT_CONFIG -= no-pkg-config
        !contains(OS_VERSION, 13.0.0):LIBS += -lboost_system-mt -lboost_thread-mt -ldl

	contains(OS_VERSION, 13.0.0):LIBS += -lboost_system -lboost_thread -ldl

    # -------------------------------------------
    # NOTE: This is necessary on Mac OSX Mavericks (10.9)
    #	Because libc++ is now chosen by default over libstdc++
    #   And your dependencies will have to be rebuilt with similar options.
    #
	contains(OS_VERSION, 13.0.0):QT_CONFIG += -spec macx-clang-libc++
	contains(OS_VERSION, 13.0.0):LIBS += -stdlib=libc++
	contains(OS_VERSION, 13.0.0):CONFIG += c++11
	contains(OS_VERSION, 13.0.0):QMAKE_CXXFLAGS += -mmacosx-version-min=10.7 -stdlib=libc++ -std=c++11
    #
    # -------------------------------------------
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

TRANSLATIONS += Translations/en_US.ts \
                Translations/de_DE.ts
