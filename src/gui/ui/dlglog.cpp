#include "dlglog.hpp"
#include "ui_dlglog.h"

#include <core/moneychanger.hpp>

#include <QDate>
#include <QKeyEvent>


DlgLog::DlgLog(QWidget *parent) :
    QDialog(parent),
    m_bFirstRun(true),
    ui(new Ui::DlgLog)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

DlgLog::~DlgLog()
{
    delete ui;
}

void DlgLog::appendToLog(QString qstrAppend)
{
    QString qstrTimestamp = QDate::currentDate().toString(Qt::SystemLocaleShortDate);

    ui->plainTextEdit->appendPlainText(QString("%1: %2\n").arg(qstrTimestamp).arg(qstrAppend));
}

void DlgLog::on_pushButtonClear_clicked()
{
    ui->plainTextEdit->clear();
}


bool DlgLog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }
    // -------------------------------------------
    // standard event processing
    return QDialog::eventFilter(obj, event);
}

void DlgLog::dialog()
{
    FirstRun();
    show();
    setFocus();
}

void DlgLog::FirstRun()
{
    if (m_bFirstRun)
    {
        m_bFirstRun = false;
        // -----------------------
        // Initialization here...
        // ----------------------------------------------------------------
//        QPixmap pixmapContacts(":/icons/icons/user.png");
//        QPixmap pixmapRefresh (":/icons/icons/refresh.png");
//        // ----------------------------------------------------------------
//        QIcon contactsButtonIcon(pixmapContacts);
//        QIcon refreshButtonIcon (pixmapRefresh);
//        // ----------------------------------------------------------------
//        ui->toolButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
//        ui->toolButton->setAutoRaise(true);
//        ui->toolButton->setIcon(refreshButtonIcon);
////      ui->toolButton->setIconSize(pixmapRefresh.rect().size());
//        ui->toolButton->setIconSize(pixmapContacts.rect().size());
//        ui->toolButton->setText(tr("Refresh"));

//        // ******************************************************
//        {
//            m_pMarketDetails = new MTDetailEdit(this);

//            m_pMarketDetails->SetMarketMap(m_mapMarkets);
//            // -------------------------------------
//            m_pMarketDetails->setWindowTitle(tr("Markets"));
//            // -------------------------------------
//            QVBoxLayout * pLayout = new QVBoxLayout;
//            pLayout->addWidget(m_pMarketDetails);

//            m_pMarketDetails->setContentsMargins(1,1,1,1);
//            pLayout->setContentsMargins(1,1,1,1);

//            ui->tabMarkets->setContentsMargins(1,1,1,1);

//            ui->tabMarkets->setLayout(pLayout);
//            // -------------------------------------
//            connect(this,             SIGNAL(needToLoadOrRetrieveMarkets()),
//                    m_pMarketDetails, SLOT(onSetNeedToRetrieveOfferTradeFlags()));
//            // -------------------------------------
//            connect(this, SIGNAL(needToLoadOrRetrieveMarkets()),
//                    this, SLOT(LoadOrRetrieveMarkets()));
//            // -------------------------------------
//            connect(m_pMarketDetails, SIGNAL(CurrentMarketChanged(QString)),
//                    this,             SLOT(onCurrentMarketChanged_Markets(QString)));
//            // -------------------------------------
//            // Connect market panel "current market changed" to *this "onCurrentMarketChanged
//            connect(m_pMarketDetails, SIGNAL(CurrentMarketChanged(QString)),
//                    this,             SLOT(onNeedToLoadOrRetrieveOffers(QString)));
//            // -------------------------------------

//        }
//        // ******************************************************
////        void CurrentMarketChanged(QString qstrMarketID);


//        // ******************************************************
//        {
//            m_pOfferDetails = new MTDetailEdit(this);

//            m_pOfferDetails->SetMarketMap(m_mapMarkets);
//            m_pOfferDetails->SetOfferMap (m_mapOffers);
//            // -------------------------------------
//            m_pOfferDetails->setWindowTitle(tr("Orders"));

//            // NOTE: This gets set already whenever MTDetailEdit::FirstRun
//            // is called. But we want it to be set before that, in this case,
//            // since m_pMarketDetails and m_pOfferDetails are intertwined.
//            //
//            m_pOfferDetails->SetType(MTDetailEdit::DetailEditTypeOffer);
//            // -------------------------------------
//            QVBoxLayout * pLayout = new QVBoxLayout;
//            pLayout->addWidget(m_pOfferDetails);

//            m_pOfferDetails->setContentsMargins(1,1,1,1);
//            pLayout->setContentsMargins(1,1,1,1);

//            ui->tabOffers->setContentsMargins(1,1,1,1);

//            ui->tabOffers->setLayout(pLayout);
//            // -------------------------------------
//            connect(m_pOfferDetails, SIGNAL(CurrentMarketChanged(QString)),
//                    this,            SLOT(onCurrentMarketChanged_Offers(QString)));
//            // -------------------------------------
//            connect(m_pOfferDetails, SIGNAL(CurrentMarketChanged(QString)),
//                    this,            SLOT(onNeedToLoadOrRetrieveOffers(QString)));
//            // -------------------------------------
//            connect(m_pOfferDetails, SIGNAL(NeedToLoadOrRetrieveOffers(QString)),
//                    this,            SLOT(onNeedToLoadOrRetrieveOffers(QString)));
//            // -------------------------------------
//        }
//        // ******************************************************

//        // Whenever the GUI refreshes (say, the list of servers is cleared
//        // and re-populated) then we will set the CURRENT selection as whatever
//        // is in m_NotaryID / m_nymId.
//        //
//        // The initial state for this comes from the defaults, but once the user
//        // starts changing the selection of the combo box, the new current selection
//        // will go into these variables.
//        //
//        m_nymId    = Moneychanger::It()->get_default_nym_id();
//        m_NotaryID = Moneychanger::It()->get_default_notary_id();

//        m_pMarketDetails->SetMarketNymID   (m_nymId);
//        m_pOfferDetails ->SetMarketNymID   (m_nymId);
//        m_pMarketDetails->SetMarketNotaryID(m_NotaryID);
//        m_pOfferDetails ->SetMarketNotaryID(m_NotaryID);
    }
}

