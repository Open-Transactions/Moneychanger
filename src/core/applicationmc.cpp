#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/applicationmc.hpp>

#include <core/passwordcallback.hpp>
#include <core/moneychanger.hpp>

#include <core/handlers/contacthandler.hpp>

#include <opentxs/api/OTAPI.hpp>
#include <opentxs/api/OTAPI_Exec.hpp>
#include <opentxs/core/OTAsymmetricKey.hpp>
#include <opentxs/api/OTRecordList.hpp>
#include <opentxs/core/OTCaller.hpp>

#include <QVBoxLayout>
#include <QDebug>
#include <QSystemTrayIcon>
#include <QDialog>
#include <QLabel>



bool SetupPasswordCallback(opentxs::OTCaller & passwordCaller, opentxs::OTCallback & passwordCallback)
{
    passwordCaller.setCallback(&passwordCallback);

    bool bSuccess = OT_API_Set_PasswordCallback(passwordCaller);

    if (!bSuccess)
    {
        qDebug() << QString("Error setting password callback!");
        return false;
    }

    return true;
}


bool SetupAddressBookCallback(opentxs::OTLookupCaller & theCaller, opentxs::OTNameLookup & theCallback)
{
    theCaller.setCallback(&theCallback);

    bool bSuccess = OT_API_Set_AddrBookCallback(theCaller);

    if (!bSuccess)
    {
        qDebug() << QString("Error setting address book callback!");
        return false;
    }

    return true;
}




MTApplicationMC::MTApplicationMC(int &argc, char **argv)
    : QApplication(argc, argv)
{

}

MTApplicationMC::~MTApplicationMC()
{
}

void MTApplicationMC::appStarting()
{
    // ----------------------------------------
    //Moneychanger Details
    QString mc_app_name = "moneychanger-qt";
    QString mc_version = "v0.0.x";

    //Compiled details
    QString mc_window_title = mc_app_name+" | "+mc_version;
    // ----------------------------------------
    // Set Password Callback.
    //
    static opentxs::OTCaller           passwordCaller;
    static MTPasswordCallback passwordCallback;

    if (!SetupPasswordCallback(passwordCaller, passwordCallback))
    {
        qDebug() << "Failure setting password callback in MTApplicationMC";
        abort();
    }
    // ----------------------------------------
    // Set Address Book Callback.
    //
    static opentxs::OTLookupCaller theCaller;
    static MTNameLookupQT theCallback;

    if (!SetupAddressBookCallback(theCaller, theCallback))
    {
        qDebug() << "Failure setting address book callback in MTApplicationMC";
        abort();
    }
    // ----------------------------------------
    // Load OTAPI Wallet
    //
    opentxs::OTAPI_Wrap::It()->LoadWallet();
    // ----------------------------------------
    /** Init Moneychanger code (Start when necessary below) **/

    QPointer<Moneychanger> pMoneychanger = Moneychanger::It();

    pMoneychanger->installEventFilter(pMoneychanger.data());

    /** Check Systray capabilities **/
    // This app relies on system tray capabilites;
    // Make sure the system has this capability before running
    //
    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        /* ** ** **
         *Open dialog that will tell the user system tray is not available
         */
        //Create dialog
        QDialog * systray_notsupported = new QDialog(0);

        //Add details to the dialog window
        systray_notsupported->setWindowTitle(mc_window_title);

        //Create a vertical box to add to the dialog so multiple objects can be added to the window
        QVBoxLayout * systray_notsupported_vboxlayout = new QVBoxLayout(0);
        systray_notsupported->setLayout(systray_notsupported_vboxlayout);

        /* Add things to dialog */
        //Add label to dialog
        QLabel * systray_notsupported_main_msg_label = new QLabel("Your system doesn't seem to support <b>System Tray</b> capabilities.<br/>This program will not run with out it.");
        systray_notsupported_vboxlayout->addWidget(systray_notsupported_main_msg_label);

        //Show dialog
        systray_notsupported->show();
    }
    else
    {
        /* ** ** **
         *Start the Moneychanger systray app
         */
        pMoneychanger->bootTray();
    }
}

