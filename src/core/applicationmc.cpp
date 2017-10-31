#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/applicationmc.hpp>

#include <core/moneychanger.hpp>

#include <core/handlers/contacthandler.hpp>
#include <core/handlers/focuser.h>

#include <opentxs/client/OTAPI_Wrap.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/client/OTRecordList.hpp>
#include <opentxs/core/crypto/OTCaller.hpp>

#include <QVBoxLayout>
#include <QDebug>
#include <QSystemTrayIcon>
#include <QDialog>
#include <QLabel>




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
    opentxs::OTAPI_Wrap::Exec()->LoadWallet();
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
//        QString qstrMenuFileExists = QString(opentxs::OTPaths::AppDataFolder().Get()) + QString("/knotworkpigeons");

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
        Focuser f(systray_notsupported);
        f.show();
        f.focus();
    }
    else
    {
        /* ** ** **
         *Start the Moneychanger systray app
         */
        pMoneychanger->bootTray();
    }
}

