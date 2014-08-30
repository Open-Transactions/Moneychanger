#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/passwordcallback.hpp>

#include <gui/ui/dlgpassword.hpp>
#include <gui/ui/dlgpasswordconfirm.hpp>

#include <core/moneychanger.hpp>

#include <QDebug>


void MTPasswordCallback::runOne(const char * szDisplay, opentxs::OTPassword & theOutput)
{
    if (NULL == szDisplay)
    {
        qDebug() << QString("MTPasswordCallback::runOne: Failure: szDisplay (telling you why to enter password) is NULL!");
        return;
    }

//    MTDlgPassword * pDlg = new MTDlgPassword(NULL, theOutput);
    MTDlgPassword * pDlg = new MTDlgPassword(Moneychanger::It(), theOutput);

    pDlg->setAttribute(Qt::WA_DeleteOnClose);

    QString qstrDisplay(szDisplay);
    pDlg->setDisplay(qstrDisplay);

    pDlg->exec();
}

void MTPasswordCallback::runTwo(const char * szDisplay, opentxs::OTPassword & theOutput)
{
    if (NULL == szDisplay)
    {
        qDebug() << QString("MTPasswordCallback::runTwo: Failure: szDisplay (telling you why to enter password) is NULL!");
        return;
    }

//    MTDlgPasswordConfirm * pDlg = new MTDlgPasswordConfirm(NULL, theOutput);
    MTDlgPasswordConfirm * pDlg = new MTDlgPasswordConfirm(Moneychanger::It(), theOutput);

    pDlg->setAttribute(Qt::WA_DeleteOnClose);

    QString qstrDisplay(szDisplay);
    pDlg->setDisplay(qstrDisplay);

    pDlg->exec();
}


