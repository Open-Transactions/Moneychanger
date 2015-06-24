#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/transportdetails.hpp>
#include <ui_transportdetails.h>

#include <gui/ui/getstringdialog.hpp>

#include <gui/widgets/overridecursor.hpp>
#include <gui/widgets/dlgchooser.hpp>

#include <core/moneychanger.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>
#include <core/mtcomms.h>

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/client/OT_ME.hpp>

#include <QMessageBox>
#include <QDebug>
#include <QStringList>


TransportDetails::TransportDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    m_pHeaderWidget(NULL),
    ui(new Ui::TransportDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);

    ui->lineEditID  ->setStyleSheet("QLineEdit { background-color: lightgray }");
    ui->lineEditType->setStyleSheet("QLineEdit { background-color: lightgray }");

    ui->toolButton->setStyleSheet("QToolButton { border: 0px solid #575757; }");
    // ----------------------------------
    // Note: This is a placekeeper, so later on I can just erase
    // the widget at 0 and replace it with the real header widget.
    //
    m_pHeaderWidget  = new QWidget;
    ui->verticalLayout->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------
}


void TransportDetails::refresh(QString strID, QString strName)
{
    qDebug() << "TransportDetails::refresh";

    if ((NULL == ui) || strID.isEmpty())
        return;

    QWidget * pHeaderWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, strID, strName, "", "", ":/icons/icons/p2p.png", false);

    pHeaderWidget->setObjectName(QString("DetailHeader")); // So the stylesheet doesn't get applied to all its sub-widgets.

    if (m_pHeaderWidget)
    {
        ui->verticalLayout->removeWidget(m_pHeaderWidget);

        m_pHeaderWidget->setParent(NULL);
        m_pHeaderWidget->disconnect();
        m_pHeaderWidget->deleteLater();

        m_pHeaderWidget = NULL;
    }
    ui->verticalLayout->insertWidget(0, pHeaderWidget);
    m_pHeaderWidget = pHeaderWidget;
    // ----------------------------------
    ui->lineEditID  ->setText(strID);
    ui->lineEditName->setText(strName);

    int nMethodID = strID.toInt();

    ui->lineEditConnect->setText(MTContactHandler::getInstance()->GetMethodConnectStr(nMethodID));
    ui->lineEditType->setText(MTContactHandler::getInstance()->GetMethodType(nMethodID));
}

void TransportDetails::AddButtonClicked()
{
    QString strNewName    = QString("Bitmessage (Main)");
    QString strNewConnect = QString("");
    // ------------------------------------------------
    MTGetStringDialog nameDlg(this, tr("Enter a Display Name:"));
    nameDlg.setWindowTitle(tr("New Transport Method"));

    if (QDialog::Accepted == nameDlg.exec())
    {
        strNewName = nameDlg.GetOutputString();
        // ------------------------------------------------
        // Next, choose the message type. (Such as "bitmessage".)
        //
        DlgChooser theChooser(this);
        mapIDName & the_map = theChooser.m_map;
        // -----------------------------------------------
        MTContactHandler::getInstance()->GetMsgMethodTypes(the_map); //bAddServers=false by default
        // -----------------------------------------------
        theChooser.setWindowTitle(tr("Select Messaging Type"));
        // -----------------------------------------------
        if (theChooser.exec() == QDialog::Accepted)
        {
            if (!theChooser.m_qstrCurrentID.isEmpty())
            {
                QString qstrType        = theChooser.m_qstrCurrentID;   // "bitmessage"
                QString qstrTypeDisplay = theChooser.m_qstrCurrentName; // "Bitmessage"
                // ----------------------------
                MTGetStringDialog connectDlg(this, tr("Enter connect info:"), QString::fromStdString(MTComms::connectPlaceholder(qstrType.toStdString())));
                connectDlg.setWindowTitle(tr("Connection String"));

                if (QDialog::Accepted == connectDlg.exec())
                {
                    strNewConnect = connectDlg.GetOutputString();
                    // ------------------------------------------------
                    int nMethod  = MTContactHandler::getInstance()->AddMsgMethod(strNewName, qstrType, qstrTypeDisplay, strNewConnect);

                    if (nMethod <= 0)
                    {
                        QMessageBox::warning(this, tr("Failed creating transport method"),
                                             tr("Failed trying to create transport type."));
                        return;
                    }
                    // -------------------------------------------------------
                    // else (Successfully created the new Transport method...)
                    // Now let's add this it to the Map, and refresh the dialog,
                    // and then set the new transport method as the current one.
                    //
                    QString qstrMethodID = QString("%1").arg(nMethod);

                    m_pOwner->m_map.insert(qstrMethodID, strNewName);
                    m_pOwner->SetPreSelected(qstrMethodID);
                    // ------------------------------------------------
                    emit RefreshRecordsAndUpdateMenu();
                    // ------------------------------------------------
                }
            }
        }
    }
}

/*
 bool MTContactHandler::getInstance()->GetMsgMethods(mapIDName & theMap);

  int  MTContactHandler::getInstance()->AddMsgMethod(QString display_name, QString type, QString type_display, QString connect);
  bool MTContactHandler::getInstance()->DeleteMsgMethod(int nMethodID);
*/

void TransportDetails::DeleteButtonClicked()
{
    if (!m_pOwner->m_qstrCurrentID.isEmpty())
    {
        bool bSuccess = MTContactHandler::getInstance()->DeleteMsgMethod(m_pOwner->m_qstrCurrentID.toInt());

        if (bSuccess)
        {
            m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
            // ------------------------------------------------
            emit RefreshRecordsAndUpdateMenu();
            // ------------------------------------------------
        }
    }
}

void TransportDetails::on_lineEditConnect_editingFinished()
{
    int nMethodID = m_pOwner->m_qstrCurrentID.toInt();

    if (nMethodID > 0)
        MTContactHandler::getInstance()->SetMethodConnectStr(nMethodID, ui->lineEditConnect->text());
}


void TransportDetails::on_lineEditName_editingFinished()
{
    int nMethodID = m_pOwner->m_qstrCurrentID.toInt();

    if (nMethodID > 0)
    {
        bool bSuccess = MTContactHandler::getInstance()->SetMethodDisplayName(nMethodID, ui->lineEditName->text());

        if (bSuccess)
        {
            m_pOwner->m_map.remove(m_pOwner->m_qstrCurrentID);
            m_pOwner->m_map.insert(m_pOwner->m_qstrCurrentID, ui->lineEditName->text());

            m_pOwner->SetPreSelected(m_pOwner->m_qstrCurrentID);
            // ------------------------------------------------
            emit RefreshRecordsAndUpdateMenu();
            // ------------------------------------------------
        }
    }
}

void TransportDetails::ClearContents()
{
    ui->lineEditID     ->setText("");
    ui->lineEditType   ->setText("");
    ui->lineEditName   ->setText("");
    ui->lineEditConnect->setText("");
}


TransportDetails::~TransportDetails()
{
    delete ui;
}
