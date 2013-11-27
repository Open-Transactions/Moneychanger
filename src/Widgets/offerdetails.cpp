#include <QMessageBox>
#include <QDebug>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>

#include "offerdetails.h"
#include "ui_offerdetails.h"

#include "detailedit.h"
#include "moneychanger.h"
#include "overridecursor.h"

MTOfferDetails::MTOfferDetails(QWidget *parent, MTDetailEdit & theOwner) :
    MTEditDetails(parent, theOwner),
    ui(new Ui::MTOfferDetails)
{
    ui->setupUi(this);
    this->setContentsMargins(0, 0, 0, 0);
//  this->installEventFilter(this); // NOTE: Successfully tested theory that the base class has already installed this.
    // ----------------------------------
    // Note: This is a placekeeper, so later on I can just erase
    // the widget at 0 and replace it with the real header widget.
    //
    m_pHeaderWidget  = new QWidget;
    ui->verticalLayout->insertWidget(0, m_pHeaderWidget);
    // ----------------------------------
}


void MTOfferDetails::refresh(QString strID, QString strName)
{
    if (!strID.isEmpty() && (NULL != ui))
    {
        // FYI, contents of qstrID:
//      QString qstrCompositeID = QString("%1,%2,%3,%4").arg(qstrServerID).arg(qstrNymID).arg(qstrMarketID).arg(lOfferID);
/*
        QString     qstrMarketServerID, qstrNymID, qstrMarketID, qstrMarketServerName, qstrOfferID;
        QStringList theIDs = strID.split(","); // theIDs.at(0) is ServerID, at(1) NymID, at(2) is MarketID, at(3) is OfferID

        if (4 == theIDs.size()) // Should always be 4...
        {
            qstrMarketServerID  = theIDs.at(0);
            qstrNymID           = theIDs.at(1);
            qstrMarketID        = theIDs.at(2);
            qstrOfferID         = theIDs.at(3);
            // ---------------------------------
            if (!qstrMarketServerID.isEmpty()) // Should never be empty.
                qstrMarketServerName = QString::fromStdString( OTAPI_Wrap::GetServer_Name(qstrMarketServerID.toStdString()) );
        }
        // -------------------------------------
        pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrOfferID, qstrValue, qstrMarketServerName, "");
        */
//      pWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, qstrOfferID, qstrValue, qstrMarketServerName, "", /*OFFER ICON HERE*/);

        QWidget * pHeaderWidget  = MTEditDetails::CreateDetailHeaderWidget(m_Type, strID, strName, "", "", ":/icons/icons/assets.png", false);

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
//        ui->lineEditID  ->setText(strID);
//        ui->lineEditName->setText(strName);

        FavorLeftSideForIDs();
        // --------------------------
    }
}

void MTOfferDetails::AddButtonClicked()
{

}

void MTOfferDetails::DeleteButtonClicked()
{

}

void MTOfferDetails::FavorLeftSideForIDs()
{
    if (NULL != ui)
    {
//        ui->lineEditID  ->home(false);
//        ui->lineEditName->home(false);
    }
}

void MTOfferDetails::ClearContents()
{
//    ui->lineEditID  ->setText("");
//    ui->lineEditName->setText("");

//    if (m_pPlainTextEdit)
//        m_pPlainTextEdit->setPlainText("");
}

// ------------------------------------------------------

bool MTOfferDetails::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize)
    {
        // This insures that the left-most part of the IDs and Names
        // remains visible during all resize events.
        //
        FavorLeftSideForIDs();
    }
//    else
//    {
        // standard event processing
//        return QObject::eventFilter(obj, event);
        return MTEditDetails::eventFilter(obj, event);

        // NOTE: Since the base class has definitely already installed this
        // function as the event filter, I must assume that this version
        // is overriding the version in the base class.
        //
        // Therefore I call the base class version here, since as it's overridden,
        // I don't expect it will otherwise ever get called.
//    }
}

// ------------------------------------------------------

MTOfferDetails::~MTOfferDetails()
{
    delete ui;
}
