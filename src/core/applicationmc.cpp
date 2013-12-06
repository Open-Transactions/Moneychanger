#ifndef STABLE_H
#include <core/stable.h>
#endif

#include "applicationmc.h"

#include "moneychanger.h"

#include "passwordcallback.h"

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>

#include <opentxs/OTPassword.h>

#include <opentxs/OTAsymmetricKey.h>

#include <opentxs/OTLog.h>

bool SetupPasswordCallback(OTCaller & passwordCaller, OTCallback & passwordCallback);


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
    // Load OTAPI Wallet
    //
    static OTCaller           passwordCaller;
    static MTPasswordCallback passwordCallback;

    if (!SetupPasswordCallback(passwordCaller, passwordCallback))
    {
        qDebug() << "Failure setting password callback in MTApplicationMC";
        abort();
    }
    // ----------------------------------------
    OTAPI_Wrap::It()->LoadWallet();
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

bool SetupPasswordCallback(OTCaller & passwordCaller, OTCallback & passwordCallback)
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


