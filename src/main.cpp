#include <QApplication>
#include <QtGui>
#include <QLibrary>
#include <QSystemTrayIcon>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>

#include "moneychanger.h"

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

void shutdown_app(){

}


class __OTclient_RAII
{
public:
    __OTclient_RAII()
    {
        // SSL gets initialized in here, before any keys are loaded.
        OTAPI_Wrap::AppInit();
    }
    ~__OTclient_RAII()
    {

        OTAPI_Wrap::AppCleanup();
    }
};


// ----------------------------------------



int main(int argc, char *argv[])
{
    // NB: you don't really have to do this if your library links ok
    // but I tested it and it works and gives a nice message, rw.
    //
    //QLibrary otapi("libotapi.so");     // <===== FIRST constructor called.
    //if (!otapi.load())
    //    qDebug() << otapi.errorString();
    //if (otapi.load())
    //    qDebug() << "otapi loaded";
    // ----------------------------------------
    // AppInit() is called here by this object's constructor. (And
    // AppCleanup() will be called automatically when we exit main(),
    // by this same object's destructor.)
    //
    __OTclient_RAII the_client_cleanup;  // <===== SECOND constructor is called here.
    // ----------------------------------------
    if (NULL == OTAPI_Wrap::It())
    {
        OTLog::vError(0, "Error, exiting: OTAPI_Wrap::AppInit() call must have failed.\n");
        return -1;
    }
    // ----------------------------------------
    //Init qApp
    QApplication a(argc, argv);  // <====== THIRD constructor (they are destroyed in reverse order.)

    //Config qApp
    QApplication::setQuitOnLastWindowClosed(false);
    // ----------------------------------------
    //Moneychanger Details
    QString mc_app_name = "moneychanger-qt";
    QString mc_version = "v0.0.x";

    //Compiled details
    QString mc_window_title = mc_app_name+" | "+mc_version;
    // ----------------------------------------
    // Load OTAPI Wallet
    //
    OTAPI_Wrap::It()->LoadWallet();

    // ----------------------------------------
    /** Init Moneychanger code (Start when nessecary below) **/
    Moneychanger systray;

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
        systray.bootTray();
    }
    // ----------------------------------------------------------------
    // Leave this here for now, it will help debugging.
    int nServerCount = static_cast<int>(OTAPI_Wrap::GetServerCount());
    OTLog::vOutput(0, "Number of servers in the wallet: %d\n", nServerCount);
    // ----------------------------------------------------------------
    for (int i = 0; i < nServerCount; ++i)
    {
        std::string str_ServerID = OTAPI_Wrap::GetServer_ID(i);
        OTLog::vOutput(0, "Server at index %d has ID: %s\n", i, str_ServerID.c_str());
        // ---------------------------------------
        std::string str_ServerName = OTAPI_Wrap::GetServer_Name(str_ServerID);
        OTLog::vOutput(0, "The name for this server is: %s\n", str_ServerName.c_str());
    }
    // ----------------------------------------------------------------
    //
    int nExec = a.exec(); // <=== Here's where we run the QApplication...
    // ----------------------------------------------------------------
    OTLog::vOutput(0, "Finished executing the QApplication!\n(AppCleanup should occur "
                   "immediately after this point.)\nReturning: %d\n", nExec);
    // ----------------------------------------------------------------
    return nExec;
}
// Note: all the DESTRUCTORS for the local objects to main are called HERE -- in REVERSE ORDER.
//
// Here's the order of the constructors from the top of main():
//
// First: QLibrary otapi,
// Second: __OTclient_RAII the_client_cleanup
// Third: QApplication a
// Third: Moneychanger w
//
// Here's the order of the destructrors at the bottom of main():
//
// First:  Moneychanger w
// Second: QApplication a
// Third:  __OTclient_RAII the_client_cleanup
// Third:  QLibrary otapi,











