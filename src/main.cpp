#include <QApplication>
#include <QtGui>
#include <QLibrary>
#include <QSystemTrayIcon>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>


#include "moneychanger.h"

#include "passwordcallback.h"

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>

#include <opentxs/OTPassword.h>

#include <opentxs/OTAsymmetricKey.h>

#include <opentxs/OTLog.h>

#include "applicationmc.h"


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
    
    /*
    QDir dir(argv[0]);  // e.g. appdir/Contents/MacOS/appname
    assert(dir.cdUp());
    assert(dir.cdUp());
    assert(dir.cd("PlugIns"));  // e.g. appdir/Contents/PlugIns
    QCoreApplication::setLibraryPaths(QStringList(dir.absolutePath()));
    printf("after change, libraryPaths=(%s)\n", QCoreApplication::libraryPaths().join(",").toUtf8().data());
    */
    QApplication::libraryPaths();
    MTApplicationMC theApplication(argc, argv);  // <====== THIRD constructor (they are destroyed in reverse order.)
    theApplication.setQuitOnLastWindowClosed(false);
//  QApplication:: setQuitOnLastWindowClosed(false);

    QTimer::singleShot(0, &theApplication, SLOT(appStarting()));
    // ----------------------------------------------------------------
    int nExec = theApplication.exec(); // <=== Here's where we run the QApplication...
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











