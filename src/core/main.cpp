#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/moneychanger.hpp>
#include <core/applicationmc.hpp>
#include <core/modules.hpp>
#include <core/translation.hpp>

#include <bitcoin-api/btcmodules.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OTLog.hpp>
#include <opentxs/OTPaths.hpp>

#include <QTimer>


void shutdown_app()
{
    
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
    MTApplicationMC theApplication(argc, argv);  // <====== THIRD constructor (they are destroyed in reverse order.)
    theApplication.setQuitOnLastWindowClosed(false);

    { Modules modules; }    // run constructor once, initialize static pointers
    BtcModulesPtr btcModules = BtcModulesPtr(new BtcModules());

    //Set language
    Translation appTranslation;
    QTranslator translator;
    appTranslation.updateLanguage(theApplication, translator);

    QTimer::singleShot(0, &theApplication, SLOT(appStarting()));
    // ----------------------------------------------------------------

// ----------------------------------------------------------------
// NOTE: What's this about?
//
// For the Mac version only (and ONLY for actual release dmg's) we
// want the scripts folder to be found inside the application directory,
// near the binary itself.
//
// This is because we get problems with the MacOS .dmg where especially on
// Mavericks, the sandbox prevents us from opening the scripts (ot_commands.ot, etc)
// which on UNIX systems, are normally found in /usr/local/lib/opentxs
//
// There are several solutions at play. First, we are eliminating the scripts entirely,
// by converting them to C++. But in the meantime, until that's done, we have to live with
// the scripts, so we are putting them in a place where the application CAN open them:
// the application folder itself. That is, the "AppBinaryFolder".
//
// Here we use QCoreApplication::applicationDirPath() to get the application folder path,
// and we tell OT to use that as the "AppBinaryFolder". OT doesn't use that variable except
// in one place: when setting the Scripts directory -- and ONLY if the variable is set (and
// we ARE setting it here below.)
//
// If the variable is set (and in this case, it is) then OT will use the AppBinaryFolder instead
// of the PrefixPath, to prepend to the Scripts path. This will cause OT to look for the scripts
// in the ApplicationDirPath/lib/opentxs folder instead of /usr/local/lib/opentxs
//
// Notice also we are checking the path to see if it contains "clang" and "build", and we only
// set the AppBinaryFolder in the case where those substrings are both NOT found. That way this
// fix will not impact developer builds, but only actual release DMGs.
//
// This fix assumes that the DMG contains, in the MacOS folder, next to moneychanger-qt and mc_db,
// a folder "lib", which contains a folder "opentxs", which contains the scripts. If those files
// are not placed there in the DMG, then this code would obviously cause OT to fail trying to load
// its scripts.
//
#ifdef Q_OS_MAC
    QString qstrAppDirPath = QCoreApplication::applicationDirPath();

    if (!qstrAppDirPath.isEmpty())
    {
        int nIndexClang = qstrAppDirPath.indexOf("clang", 0);
        int nIndexBuild = qstrAppDirPath.indexOf("build", 0);

        if ((-1 == nIndexClang) && (-1 == nIndexBuild))
            OTPaths::SetAppBinaryFolder(qstrAppDirPath.toStdString().c_str());
    }
//  QMessageBox::information(NULL, "", QString::fromStdString(std::string(OTPaths::ScriptsFolder().Get())));
#endif
// ----------------------------------------------------------------
    int nExec = theApplication.exec(); // <=== Here's where we run the QApplication...
    // ----------------------------------------------------------------

    Moneychanger::It(NULL, true); // bShuttingDown=true.

    // ----------------------------------------------------------------
    OTLog::vOutput(0, "Finished executing the QApplication!\n(AppCleanup should occur "
                   "immediately after this point.)\nReturning: %d\n", nExec);
    // ----------------------------------------------------------------
    return nExec;
}

//#include "main.moc"



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











