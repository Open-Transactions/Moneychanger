#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgexportcash.hpp>
#include <ui_dlgexportcash.h>

#include <gui/widgets/dlgchooser.hpp>

#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>



DlgExportCash::DlgExportCash(QWidget *parent) :
    QDialog(parent),
    m_bExportToPassphrase(false),
    ui(new Ui::DlgExportCash)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

bool DlgExportCash::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
            close(); // This is caught by this same filter.
            return true;
        }
    }
    // standard event processing
    return QDialog::eventFilter(obj, event);
}


void DlgExportCash::on_buttonBox_accepted()
{
    if (m_bExportToPassphrase)
    {
        accept();
        return;
    }
    // --------------------------------
    // If we're exporting to a recipient, then we need to select
    // the recipient...
    //
    // Select recipient from the address book and convert to Nym ID.
    // -----------------------------------------------
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    MTContactHandler::getInstance()->GetContacts(the_map);

    if (!m_hisNymId.isEmpty())
    {
        int nContactID = MTContactHandler::getInstance()->FindContactIDByNymID(m_hisNymId);

        if (nContactID > 0)
        {
            QString strTempID = QString("%1").arg(nContactID);
            theChooser.SetPreSelected(strTempID);
        }
    }
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Choose the Recipient"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
//        qDebug() << QString("SELECT was clicked for ID: %1").arg(theChooser.m_qstrCurrentID);

        // If not the same as before, then we have to choose a NymID based on the selected Contact.
        //
        QString qstrContactName;
        int     nSelectedContactID = theChooser.m_qstrCurrentID.toInt();

        if (nSelectedContactID <= 0) // Should never happen.
        {
            m_hisNymId = QString("");
            m_hisName  = QString("");
            return;
        }
        // else...
        //
        qstrContactName = MTContactHandler::getInstance()->GetContactName(nSelectedContactID);

        if (qstrContactName.isEmpty())
            m_hisName = tr("(Contact has a blank name)");
        else
            m_hisName = qstrContactName;
        // ---------------------------------------------
        // Next we need to find a Nym based on this Contact...
        //
        mapIDName theNymMap;

        if (MTContactHandler::getInstance()->GetNyms(theNymMap, nSelectedContactID))
        {
            // This contact has exactly one Nym, so we'll go with it.
            //
            if (theNymMap.size() == 1)
            {
                mapIDName::iterator theNymIt = theNymMap.begin();

                m_hisNymId = theNymIt.key();
//              QString qstrNymName = theNymIt.value();
                // ---------------------------
                accept();
                return;
            }
            else // There are multiple Nyms to choose from.
            {
                DlgChooser theNymChooser(this);
                theNymChooser.m_map = theNymMap;
                theNymChooser.setWindowTitle(tr("Recipient has multiple Nyms. (Please choose one.)"));
                // -----------------------------------------------
                if (theNymChooser.exec() == QDialog::Accepted)
                {
                    m_hisNymId = theNymChooser.m_qstrCurrentID;
                    accept();
                    return;
                }
                else // User must have cancelled.
                {
                    m_hisNymId = QString("");
                    m_hisName  = QString("");
                }
            }
        }
        else // No nyms found for this ContactID.
        {
            m_hisNymId = QString("");
            m_hisName  = QString("");
            // -------------------------------------
            QMessageBox::warning(this, tr("Contact has no known identities"),
                                 tr("Sorry, Contact '%1' has no known NymIDs (to export cash to.)").arg(qstrContactName));
        }
        // --------------------------------
    } // Chooser (on click accepted)
}

void DlgExportCash::on_buttonBox_rejected()
{
    reject();
}

DlgExportCash::~DlgExportCash()
{
    delete ui;
}

// Export to a recipient.
void DlgExportCash::on_radioButton_clicked()
{
    m_bExportToPassphrase = false;
}

// Export to a passphrase.
void DlgExportCash::on_radioButton_2_clicked()
{
    m_bExportToPassphrase = true;
}
