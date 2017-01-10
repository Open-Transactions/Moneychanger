#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/dlgencrypt.hpp>
#include <ui_dlgencrypt.h>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/handlers/focuser.h>

#include <gui/ui/dlgexportedtopass.hpp>

#include <opentxs/core/app/App.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/client/OTAPI_Wrap.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/client/OT_ME.hpp>
#include <opentxs/client/OT_API.hpp>

#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTEnvelope.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>
#include <opentxs/core/crypto/OTSignedFile.hpp>

#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QListWidgetItem>
#include <QMessageBox>

#include <set>

#include <string>


DlgEncrypt::DlgEncrypt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgEncrypt),
    already_init(false),
    m_bEncrypt(false),
    m_bSign(false),
    m_nymId("")
{
    ui->setupUi(this);

    this->installEventFilter(this);

    ui->toolButton  ->setStyleSheet("QToolButton { border: 0px solid #575757; }");
    ui->toolButton_2->setStyleSheet("QToolButton { border: 0px solid #575757; }");
}

DlgEncrypt::~DlgEncrypt()
{
    delete ui;
}

void DlgEncrypt::SetEncrypt(bool bEncrypt/*=true*/)
{
    m_bEncrypt = bEncrypt;
}

void DlgEncrypt::SetSign(bool bSign/*=true*/)
{
    m_bSign = bSign;
}

void DlgEncrypt::SetCurrentNymIDBasedOnIndex(int index)
{
    if ((m_mapNyms.size() > 0) && (index >= 0))
    {
        int nCurrentIndex = -1;

        for (mapIDName::iterator it_map = m_mapNyms.begin(); it_map != m_mapNyms.end(); ++it_map)
        {
            ++nCurrentIndex; // zero on first iteration.

            if (nCurrentIndex == index)
            {
                m_nymId = it_map.key();
                break;
            }
        }
    }
    else
        m_nymId = QString("");
}


void DlgEncrypt::PopulateCombo()
{
    if (ui)
    {
        ui->comboBoxNym->blockSignals(true);
        // ----------------------------
        int nDefaultNymIndex    = 0;
        bool bFoundNymDefault   = false;
        // -----------------------------------------------
        const int32_t nym_count = opentxs::OTAPI_Wrap::Exec()->GetNymCount();
        // -----------------------------------------------
        for (int32_t ii = 0; ii < nym_count; ++ii)
        {
            QString OT_nym_id = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetNym_ID(ii));
            QString OT_nym_name("");
            // -----------------------------------------------
            if (!OT_nym_id.isEmpty())
            {
                if (!m_nymId.isEmpty() && (OT_nym_id == m_nymId))
                {
                    bFoundNymDefault = true;
                    nDefaultNymIndex = ii;
                }
                // -----------------------------------------------
                MTNameLookupQT theLookup;

                OT_nym_name = QString::fromStdString(theLookup.GetNymName(OT_nym_id.toStdString(), ""));
                // -----------------------------------------------
                m_mapNyms.insert(OT_nym_id, OT_nym_name);
                ui->comboBoxNym->insertItem(ii, OT_nym_name);
            }
         }
        // -----------------------------------------------
        if (m_mapNyms.size() > 0)
        {
            SetCurrentNymIDBasedOnIndex(nDefaultNymIndex);
            ui->comboBoxNym->setCurrentIndex(nDefaultNymIndex);
        }
        else
            SetCurrentNymIDBasedOnIndex(-1);
        // -----------------------------------------------
        ui->comboBoxNym->blockSignals(false);
    }
}


void DlgEncrypt::PopulateWidgetNotAdded()
{
    mapIDName theContactMap;

    if (MTContactHandler::getInstance()->GetContacts(theContactMap))
    {
        // loop through contact map and get Nyms.
        for (mapIDName::iterator it_contact = theContactMap.begin();
             it_contact != theContactMap.end();
             ++it_contact)
        {
            QString qstrContactID   = it_contact.key  ();
            QString qstrContactName = it_contact.value();

            if (!qstrContactID.isEmpty())
            {
                int       nContactID = qstrContactID.toInt();
                mapIDName theNymMap;

                if ((nContactID > 0) && (MTContactHandler::getInstance()->GetNyms(theNymMap, nContactID) > 0))
                {
                    // Loop through the nyms for this contact, and add each to the Widget
                    // for "Not Added" contacts.
                    //
                    for (mapIDName::iterator it_nym = theNymMap.begin();
                         it_nym != theNymMap.end();
                         ++it_nym)
                    {
                        QString qstrNymID   = it_nym.key  ();
//                      QString qstrNymName = it_nym.value();

                        QString qstrRecipientName = QString("%1 (%2)").arg(qstrContactName).arg(qstrNymID);
                        // --------------------------------------
                        QListWidgetItem * pRecipItem = new QListWidgetItem(qstrRecipientName);
                        ui->listWidgetNotAdded->insertItem(ui->listWidgetNotAdded->count(), pRecipItem);
                        pRecipItem->setData(Qt::UserRole, QVariant(qstrNymID));

//                      connect(this, SIGNAL(), )
                    }
                }
            }
        }
    }

    if (ui->listWidgetNotAdded->count() > 0)
        ui->listWidgetNotAdded->setCurrentRow(0);
}





void DlgEncrypt::dialog()
{
    if (!already_init)
    {
        // ----------------------------
        Focuser f(this);
        f.show();
        f.focus();
        // ----------------------------
        if (m_bEncrypt)
        {
            ui->checkBoxEncrypt->toggle();
        }
        else
        {
            ui->checkBoxEncrypt->toggle();
            ui->checkBoxEncrypt->toggle();
        }
        // ----------------------------
        if (m_bSign)
        {
            ui->checkBoxSign->toggle();
        }
        else
        {
            ui->checkBoxSign->toggle();
            ui->checkBoxSign->toggle();
        }

//        ui->fromButton->setFocus();

        QString qstrTempID = Moneychanger::It()->get_default_nym_id();

        if (!qstrTempID.isEmpty())
            m_nymId = qstrTempID;

        PopulateCombo();
        PopulateWidgetNotAdded();

        already_init = true;
    }
}




void DlgEncrypt::on_pushButtonEncrypt_clicked()
{
    QString qstrText = ui->plainTextEdit->toPlainText().trimmed();
    // --------------------------------
    if (qstrText.isEmpty())
    {
        // pop up a message box warning that the input text is empty.
        //
        QMessageBox::warning(this, tr("Input Text is Empty"),
                             tr("Please paste some text to be signed/encrypted."));
        return;
    }
    else
    {
        if (m_bSign)
        {
            if (m_nymId.isEmpty())
            {
                QMessageBox::warning(this, tr("Missing Signer"),
                                     tr("No signer is selected. Perhaps you need to create an identity first, to sign with."));
                return;
            }
            else
            {
                // Sign the contents.
                //
                std::string  str_nym    (m_nymId.toStdString());
                opentxs::String     strNym     (str_nym.c_str());
                opentxs::Identifier nym_id     (strNym);

                std::string  str_text   (qstrText.toStdString());
                opentxs::String     strText    (str_text.c_str());
//              opentxs::OTASCIIArmor ascText    (strText);
//              std::string  str_encoded(ascText.Get());
//              opentxs::String     strEncoded (str_encoded.c_str());
//              std::string  str_type   ("MESSAGE");

                if (!nym_id.IsEmpty())
                {
                    opentxs::OTPasswordData thePWData("Signer passphrase");

                    opentxs::Nym * pNym = opentxs::OTAPI_Wrap::OTAPI()->GetOrLoadPrivateNym(nym_id,
                                                                           false, //bChecking=false
                                                                           __FUNCTION__,
                                                                           &thePWData);
                    if (NULL == pNym)
                    {
                        QString qstrErrorMsg = QString("%1: %2").arg(tr("Failed loading the signer; unable to continue. NymID")).arg(m_nymId);
                        QMessageBox::warning(this, tr("Failed Loading Signer"), qstrErrorMsg);
                        return;
                    }
                    else
                    {
                        // FOR VERIFY STEP:
    //                  inline opentxs::String & opentxs::OTSignedFile::GetFilePayload()                       { return m_strSignedFilePayload;   }

                        opentxs::String     strSignedOutput;
                        opentxs::OTSignedFile theSignedFile;

                        theSignedFile.SetSignerNymID(strNym);

                        theSignedFile.SetFilePayload(strText);
                        theSignedFile.SignContract(*pNym, &thePWData);
                        theSignedFile.SaveContract();
                        theSignedFile.SaveContractRaw(strSignedOutput);

                        // Set the result onto qstrText
                        //
                        if (!strSignedOutput.Exists())
                        {
                            QMessageBox::warning(this, tr("Signing Failed"),
                                                 tr("Failed trying to sign, using the selected identity."));
                            return;
                        }
                        else if (!theSignedFile.VerifySignature(*pNym))
                        {
                            QMessageBox::warning(this, tr("Test Verification Failed"),
                                                 tr("Failed trying to test verify, immediately after signing. Trying authentication key..."));

                            if (!theSignedFile.VerifySigAuthent(*pNym))
                            {
                                QMessageBox::warning(this, tr("Authent Test Also Failed"),
                                                     tr("Failed trying to verify signature with authentication key as well."));
                                return;
                            }
                            else
                                QMessageBox::information(this, tr("SUCCESS USING AUTHENTICATION KEY"), tr("Tried authent key instead of signing key, and it worked!"));
                        }
                        else
                        {
                            std::string str_signed_output(strSignedOutput.Get());
                            qstrText = QString::fromStdString(str_signed_output);
                        }
                    } // else (we have pNym.)
                }
//              std::string  str_output (opentxs::OTAPI_Wrap::Exec()->FlatSign(str_nym, str_encoded, str_type));
            }
        }
        // --------------------------------
        // Encrypt qstrText and pop up a dialog with the encrypted result.
        //
        if (m_bEncrypt)
        {
            if (ui->listWidgetAdded->count() > 0)
            {
                std::set<const opentxs::Nym*> setRecipients;
                bool      bRecipientsShouldBeAvailable = false;

                // Loop through each NymID in listWidgetAdded, and put them on a opentxs::setOfNyms
                // so we can pass them along to opentxs::OTEnvelope (and so we can clean them up afterwards.)
                // UPDATE: Can't clean them up! Because the wallet only owns private nyms, not public
                // ones, we never know for sure which ones are safe to erase. TODO: Fix in OT by using
                // shared_ptr.
                //
                for (int nIndex = 0; nIndex < ui->listWidgetAdded->count(); ++nIndex)
                {
                    bRecipientsShouldBeAvailable = true;

                    QListWidgetItem   * pItem    = ui->listWidgetAdded->item(nIndex);
                    QVariant            qvarItem = pItem->data(Qt::UserRole);
                    QString             qstrNymID(qvarItem.toString());
                    std::string         str_nym(qstrNymID.toStdString());
                    opentxs::String     strNym(str_nym.c_str());
                    opentxs::Identifier nym_id(strNym);

                    if (!nym_id.IsEmpty())
                    {
                        opentxs::OTPasswordData thePWData("Sometimes need to load private part of nym in order to use its public key. (Fix that!)");

                        const opentxs::Nym * pNym = opentxs::OTAPI_Wrap::OTAPI()->GetOrLoadNym(nym_id,
                                                                               false, //bChecking=false
                                                                               __FUNCTION__,
                                                                               &thePWData);
                        if (NULL == pNym)
                        {
                            QString qstrErrorMsg = QString("%1: %2").arg(tr("Failed loading a recipient; attempting to continue without. NymID")).arg(qstrNymID);

                            QMessageBox::warning(this, tr("Failed Loading Recipient"), qstrErrorMsg);
                        }
                        else
                        {
                            setRecipients.insert(setRecipients.begin(), pNym);
                        }
                    }
                    // qstrNymID will be passed to opentxs::OTEnvelope on its recipient list.
                } // for (selected Nyms.)
                // ---------------------------------------------------
                // We might also want to encrypt to the Signer's Nym, if there is one.
                // We'll default this to ON, but give users the choice to deactivate it.
                //
                if (ui->checkBoxAlso->isVisible() &&
                    ui->checkBoxAlso->isEnabled() &&
                    ui->checkBoxAlso->isChecked() &&
                    !m_nymId.isEmpty())
                {
                    std::string str_signer_nym(m_nymId.toStdString());
                    opentxs::String strSignerNymID(str_signer_nym.c_str());
                    bool bSignerIsAlreadyThere = false;

                    //FOR_EACH(opentxs::setOfNyms(), setRecipients) // See if it's already there, in which case we don't need to do anything else.
                    for(auto it = setRecipients.begin(); it != setRecipients.end(); ++ it)
                    {
                        const opentxs::Nym       * pNym = *it;
                        opentxs::String            strNymID;
                        pNym->GetIdentifier(strNymID);

                        if (strSignerNymID.Compare(strNymID))
                            bSignerIsAlreadyThere = true;
                    }
                    // -------------------------
                    if (!bSignerIsAlreadyThere) // Not already there? Add signer to list of recipients.
                    {
                        bRecipientsShouldBeAvailable = true;

                        opentxs::Identifier signer_nym_id(strSignerNymID);

                        if (!signer_nym_id.IsEmpty())
                        {
                            opentxs::OTPasswordData thePWData("Sometimes need to load private part of nym in order to use its public key. (Fix that!)");

                            auto pNym =
                                opentxs::App::Me().Contract().Nym(signer_nym_id);
                            if (!pNym)
                            {
                                QString qstrErrorMsg = QString("%1: %2").
                                        arg(tr("Failed trying to load the signer; attempting to continue without. NymID")).arg(m_nymId);
                                QMessageBox::warning(this, tr("Failed Loading Signer"), qstrErrorMsg);
                            }
                            else
                            {
                                setRecipients.insert(setRecipients.begin(), pNym.get());
                            }
                        }
                    }
                }
                // ---------------------------------------------------
                if (setRecipients.size() > 0)
                {
                    opentxs::OTEnvelope theEnvelope;
                    opentxs::String   strInput(qstrText.toStdString().c_str());

                    if (!theEnvelope.Seal(setRecipients, strInput))
                    {
                        QMessageBox::warning(this, tr("Encryption Failed"),
                                             tr("Failed trying to encrypt message."));
                        return;
                    }
                    else
                    {
                        // Success encrypting!
                        //
                        opentxs::String     strOutput;
                        opentxs::OTASCIIArmor ascCiphertext(theEnvelope);

                        if (ascCiphertext.WriteArmoredString(strOutput, "ENVELOPE")) // -----BEGIN OT ARMORED ENVELOPE-----
                        {
                            std::string str_output(strOutput.Get());
                            qstrText = QString::fromStdString(str_output);
                        }
                    }
                }
                else if (bRecipientsShouldBeAvailable) // They should be, but they weren't.
                {
                    QMessageBox::warning(this, tr("Failed Loading Recipients"),
                                         tr("Due to failure loading any of the recipients, unable to commence."));
                    return;
                }
            } // if (listItems.size() > 0)
        } // if (m_bEncrypt)
        // -------------------
        // If it's NOT encrypted, but it IS signed, then we want to OT ARMOR it as well.
        // (We don't have to if it's encrypted, since that process already armors it for us.
        //  But this is for the case where it's signed and NOT encrypted.)
        //
        else if (m_bSign && !qstrText.isEmpty())
        {
            std::string  str_text(qstrText.toStdString());
            opentxs::String     strText (str_text.c_str());
            opentxs::String     strOutput;
            opentxs::OTASCIIArmor ascText (strText);

            if (ascText.WriteArmoredString(strOutput, "SIGNED FILE")) // -----BEGIN OT ARMORED SIGNED FILE-----
            {
                std::string str_output(strOutput.Get());
                qstrText = QString::fromStdString(str_output);
            }
        }
        // -----------------------------------------------
        // if qstrText still contains something, pop up a dialog to display the result to the user.
        //
        if (!qstrText.isEmpty())
        {
            QString qstrType("Output:");

            if (m_bSign)
            {
                qstrType = QString(tr("Signed Output:"));
            }
            // -----------
            if (m_bEncrypt)
            {
                if (m_bSign)
                    qstrType = QString(tr("Signed and Encrypted Output:"));
                else
                    qstrType = QString(tr("Encrypted Output:"));
            }
            // -----------
            QString qstrSubTitle(tr("Be sure to copy it somewhere before closing this dialog."));
            // -----------
            // Pop up the result dialog.
            //
            DlgExportedToPass dlgExported(this, qstrText,
                                          qstrType,
                                          qstrSubTitle, false);
            dlgExported.exec();
        }
    } // if (!qstrText.isEmpty())
}



void DlgEncrypt::on_pushButtonClipboard_clicked()
{
    // Get text from the clipboard, and add it to the plainTextEdit widget
    //
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        QString qstrClipText = clipboard->text();

        if (!qstrClipText.isEmpty())
            ui->plainTextEdit->insertPlainText(qstrClipText);
    }
}




void DlgEncrypt::on_pushButtonAdd_clicked()
{
    bool bNeedToSort = false;

    // See if something is selected in listWidgetNotAdded
    QList<QListWidgetItem *>  listItems =  ui->listWidgetNotAdded->selectedItems();

    if (listItems.size() > 0)
    {
        // If something is selected, move it/them to listWidgetAdded
        for (QList<QListWidgetItem *>::iterator it = listItems.begin(); it != listItems.end(); ++it)
        {
            // For each item on the list, remove it from listWidgetNotAdded
            // and add it to listWidgetAdded.
            //
            QListWidgetItem * pItem = *it;

            if (NULL != pItem)
            {
                bNeedToSort = true;
                int row = ui->listWidgetNotAdded->row(pItem);

                ui->listWidgetNotAdded->takeItem(row);
                ui->listWidgetAdded   ->insertItem(ui->listWidgetAdded->count(), pItem);
            }
        }
    }

    if (bNeedToSort)
    {
        ui->listWidgetAdded   ->sortItems();
        ui->listWidgetNotAdded->sortItems();
    }

    setEncryptBtnText();
}


void DlgEncrypt::on_listWidgetNotAdded_itemDoubleClicked(QListWidgetItem *item)
{
    int row = ui->listWidgetNotAdded->row(item);

    if (row >= 0)
    {
        ui->listWidgetNotAdded->takeItem(row);
        ui->listWidgetAdded   ->insertItem(ui->listWidgetAdded->count(), item);
    }

    setEncryptBtnText();
}

void DlgEncrypt::on_listWidgetAdded_itemDoubleClicked(QListWidgetItem *item)
{
    int row = ui->listWidgetAdded->row(item);

    if (row >= 0)
    {
        ui->listWidgetAdded   ->takeItem(row);
        ui->listWidgetNotAdded->insertItem(ui->listWidgetNotAdded->count(), item);
    }

    setEncryptBtnText();
}

void DlgEncrypt::on_pushButtonRemove_clicked()
{
    bool bNeedToSort = false;

    // See if something is selected in listWidgetAdded (so we can remove it.)
    QList<QListWidgetItem *>  listItems =  ui->listWidgetAdded->selectedItems();

    if (listItems.size() > 0)
    {
        // If something is selected, move it/them to listWidgetAdded
        for (QList<QListWidgetItem *>::iterator it = listItems.begin(); it != listItems.end(); ++it)
        {
            // For each item on the list, remove it from listWidgetAdded
            // and add it to listWidgetNotAdded.
            //
            QListWidgetItem * pItem = *it;

            if (NULL != pItem)
            {
                bNeedToSort = true;
                int row = ui->listWidgetAdded->row(pItem);

                ui->listWidgetAdded   ->takeItem(row);
                ui->listWidgetNotAdded->insertItem(ui->listWidgetNotAdded->count(), pItem);
            }
        }
    }

    if (bNeedToSort)
    {
        ui->listWidgetAdded   ->sortItems();
        ui->listWidgetNotAdded->sortItems();
    }

    setEncryptBtnText();
}



void DlgEncrypt::on_checkBoxEncrypt_toggled(bool checked)
{
    if (checked)
    {
        // Turn the stuff black
        ui->labelRecipients   ->setEnabled(true);
        ui->listWidgetAdded   ->setEnabled(true);
        ui->listWidgetNotAdded->setEnabled(true);
        ui->pushButtonAdd     ->setEnabled(true);
        ui->pushButtonRemove  ->setEnabled(true);

        m_bEncrypt = true;
    }
    else
    {
        // Turn the stuff grey
        ui->labelRecipients   ->setEnabled(false);
        ui->listWidgetAdded   ->setEnabled(false);
        ui->listWidgetNotAdded->setEnabled(false);
        ui->pushButtonAdd     ->setEnabled(false);
        ui->pushButtonRemove  ->setEnabled(false);

        m_bEncrypt = false;
    }
    // ---------------
    setEncryptBtnText();
}

void DlgEncrypt::setEncryptBtnText()
{
    bool    bEnabled = false;
    QString qstrButtonTitle("Sign / Encrypt");

    if (m_bSign)
    {
        bEnabled = true;

        qstrButtonTitle = "Sign";
    }

    if (m_bEncrypt)
    {
        bEnabled = (ui->listWidgetAdded->count() > 0); //=true;

        ui->checkBoxAlso->setVisible(true);

        if (m_bSign)
        {
            qstrButtonTitle = "Sign and Encrypt";
            ui->checkBoxAlso->setEnabled(true);
        }
        else
        {
            qstrButtonTitle = "Encrypt";
            ui->checkBoxAlso->setEnabled(false);
        }
    }
    else
        ui->checkBoxAlso->setVisible(false);

    ui->pushButtonEncrypt->setText   (qstrButtonTitle);
    ui->pushButtonEncrypt->setEnabled(bEnabled);
}

void DlgEncrypt::on_checkBoxSign_toggled(bool checked)
{
    if (checked)
    {
        // Turn the stuff black
        ui->comboBoxNym->setEnabled(true);
        ui->labelAs    ->setEnabled(true);

        m_bSign = true;
    }
    else
    {
        // Turn the stuff grey
        ui->comboBoxNym->setEnabled(false);
        ui->labelAs    ->setEnabled(false);

        m_bSign = false;
    }

    setEncryptBtnText();
}




bool DlgEncrypt::eventFilter(QObject *obj, QEvent *event)
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


void DlgEncrypt::on_comboBoxNym_currentIndexChanged(int index)
{
    SetCurrentNymIDBasedOnIndex(index);
}

