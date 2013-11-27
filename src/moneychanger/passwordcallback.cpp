
#include <QDebug>

#include "passwordcallback.h"

#include "dlgpassword.h"
#include "dlgpasswordconfirm.h"



void MTPasswordCallback::runOne(const char * szDisplay, OTPassword & theOutput)
{
    if (NULL == szDisplay)
    {
        qDebug() << QString("MTPasswordCallback::runOne: Failure: szDisplay (telling you why to enter password) is NULL!");
        return;
    }

    MTDlgPassword theDlg(NULL, theOutput);

    QString qstrDisplay(szDisplay);
    theDlg.setDisplay(qstrDisplay);

    theDlg.exec();
}

void MTPasswordCallback::runTwo(const char * szDisplay, OTPassword & theOutput)
{
    if (NULL == szDisplay)
    {
        qDebug() << QString("MTPasswordCallback::runTwo: Failure: szDisplay (telling you why to enter password) is NULL!");
        return;
    }

    MTDlgPasswordConfirm theDlg(NULL, theOutput);

    QString qstrDisplay(szDisplay);
    theDlg.setDisplay(qstrDisplay);

    theDlg.exec();
}


