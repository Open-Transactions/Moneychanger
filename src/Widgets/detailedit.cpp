#include <QDebug>

#include "detailedit.h"
#include "ui_detailedit.h"

#include "editdetails.h"

#include "home.h"

#include "contactdetails.h"
#include "nymdetails.h"
#include "serverdetails.h"
#include "assetdetails.h"
#include "accountdetails.h"

#include "moneychanger.h"

#include <OTLog.h>

MTDetailEdit::MTDetailEdit(QWidget *parent, Moneychanger & theMC) :
    QWidget(parent, Qt::Window),
    m_bFirstRun(true),
    m_nCurrentRow(-1),
    m_pDetailPane(NULL),
    m_pDetailLayout(NULL),
    m_pTabWidget(NULL),
    m_pMoneychanger(&theMC),
    m_Type(MTDetailEdit::DetailEditTypeError),
    ui(new Ui::MTDetailEdit)
{
    ui->setupUi(this);
}

MTDetailEdit::~MTDetailEdit()
{
//    delete m_pDetailPane;
//    delete m_pDetailLayout;

    delete ui;
}

void MTDetailEdit::onBalancesChangedFromAbove()
{
    this->RefreshRecords();
}


void MTDetailEdit::onBalancesChangedFromBelow(QString qstrAcctID)
{
    m_PreSelected   = qstrAcctID;
    m_qstrCurrentID = qstrAcctID;

    emit balancesChanged();
}


void MTDetailEdit::dialog(MTDetailEdit::DetailEditType theType, bool bIsModal/*=false*/)
{
    if (m_bFirstRun)
    {
        m_bFirstRun = false;
        // -------------------------------------------
        ui->tableWidget->setColumnCount(2);
        ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        // -------------------------------------------
        ui->tableWidget->horizontalHeader()->resizeSection(0, 5);
        // -------------------------------------------
        ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
        ui->tableWidget->verticalHeader()->setDefaultSectionSize(60);
        ui->tableWidget->verticalHeader()->hide();
        ui->tableWidget->horizontalHeader()->hide();
        // -------------------------------------------
        ui->tableWidget->setContentsMargins(10,0,0,0);
        // -------------------------------------------
        ui->tableWidget->setSizePolicy(
                    QSizePolicy::Expanding,
                    QSizePolicy::Expanding);
        // ----------------------------------
        m_pTabWidget  = new QTabWidget;
        // ----------------------------------
        m_pTabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_pTabWidget->setContentsMargins(5, 5, 5, 5);
        // ----------------------------------
        QWidget * pTab1 = new QWidget;
        // ----------------------------------
        pTab1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        pTab1->setContentsMargins(5, 5, 5, 5);
        // ----------------------------------
        m_pTabWidget->addTab(pTab1, tr("Details"));
        // -------------------------------------------
        // Instantiate m_pDetailPane to one of various types.
        //
        m_Type = theType;

        switch (m_Type)
        {
        case MTDetailEdit::DetailEditTypeNym:     m_pDetailPane = new MTNymDetails(this, *this);     break;
        case MTDetailEdit::DetailEditTypeContact: m_pDetailPane = new MTContactDetails(this, *this); break;
        case MTDetailEdit::DetailEditTypeServer:  m_pDetailPane = new MTServerDetails(this, *this);  break;
        case MTDetailEdit::DetailEditTypeAsset:   m_pDetailPane = new MTAssetDetails(this, *this);   break;

        case MTDetailEdit::DetailEditTypeAccount:
            m_pDetailPane = new MTAccountDetails(this, *this);
            // -------------------------------------------
            connect(m_pDetailPane,   SIGNAL(DefaultAccountChanged(QString, QString)),
                    m_pMoneychanger, SLOT  (setDefaultAccount(QString, QString)));
            // -------------------------------------------
//            connect(m_pDetailPane,   SIGNAL(cashBalanceChanged()),
//                    m_pMoneychanger, SLOT  (onCashBalanceChanged()));
//            // -------------------------------------------
//            connect(m_pDetailPane,   SIGNAL(acctBalanceChanged()),
//                    m_pMoneychanger, SLOT  (onAcctBalanceChanged()));
//            // -------------------------------------------
            break;
        default:
            qDebug() << "MTDetailEdit::dialog: MTDetailEdit::DetailEditTypeError";
            return;
        }
        // -------------------------------------------
        m_pDetailPane->SetOwnerPointer(*this);
        // -------------------------------------------
        m_pDetailLayout = new QVBoxLayout;
        m_pDetailLayout->addWidget(m_pDetailPane);

        m_pDetailPane  ->setContentsMargins(1,1,1,1);
        m_pDetailLayout->setContentsMargins(1,1,1,1);
        // ----------------------------------

        pTab1->setLayout(m_pDetailLayout);

        // ----------------------------------
        int nCustomTabCount = m_pDetailPane->GetCustomTabCount();

        if (nCustomTabCount > 0)
        {
            for (int ii = 0; ii < nCustomTabCount; ii++)
            {
                QWidget * pTab = m_pDetailPane->CreateCustomTab(ii);
                // ----------------------------------
                if (NULL != pTab)
                {
                    pTab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                    pTab->setContentsMargins(5, 5, 5, 5);

                    QString qstrTabName = m_pDetailPane->GetCustomTabName(ii);

                    m_pTabWidget->addTab(pTab, qstrTabName);
                }
                // ----------------------------------
            }
        }
        // -----------------------------------------------
        QGridLayout * pGridLayout = new QGridLayout;
        pGridLayout->addWidget(m_pTabWidget);

        pGridLayout->setContentsMargins(0,0,0,0);
        m_pTabWidget->setTabPosition(QTabWidget::South);
        // ----------------------------------
        ui->widget->setContentsMargins(1,1,1,1);
        // ----------------------------------
        ui->widget->setLayout(pGridLayout);
        // ----------------------------------
    } // first run.
    // -------------------------------------------
    RefreshRecords();
    // -------------------------------------------
//    if (m_map.size() < 1)
//        on_addButton_clicked();
    // -------------------------------------------
    if (bIsModal)
    {
        QDialog theDlg;
        theDlg.setWindowTitle(this->windowTitle());
//        theDlg.installEventFilter(this);

        QVBoxLayout * pLayout = new QVBoxLayout;

        pLayout->addWidget(this);

        theDlg.setLayout(pLayout);
        theDlg.setWindowFlags(Qt::Tool); // A hack so it will show the close button.
        theDlg.exec();

        pLayout->removeWidget(this);
    }
    else
    {
        this->installEventFilter(this);

        show();
        setFocus();
    }
    // -------------------------------------------
}


void MTDetailEdit::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);

//    if (m_map.size() < 1)
//        on_addButton_clicked();
}

//virtual
//void MTDetailEdit::showEvent(QShowEvent * event)
//{
//    dialog();

//    // ----------------------------------
//    // call inherited method
//    //
//    QDialog::showEvent(event);
//}



void MTDetailEdit::RefreshRecords()
{
    disconnect(ui->tableWidget, SIGNAL(currentCellChanged(int,int,int,int)),
               this, SLOT(on_tableWidget_currentCellChanged(int,int,int,int)));
    // ----------------------------------------------------------------------
    int mapSize = m_map.size();
    // -------------------------------------------------------
    int nTotalRecords = mapSize;
    // -------------------------------------------------------
    int nGridItemCount = ui->tableWidget->rowCount();
    // -------------------------------------------------------
    for (int ii = 0; ii < nGridItemCount; ii++)
    {
        QTableWidgetItem * item = ui->tableWidget->takeItem(0,1); // Row 0, Column 1
        ui->tableWidget->removeRow(0); // Row 0.

        if (NULL != item)
            delete item;
    }
    // -------------------------------------------------------
    ui->tableWidget->setRowCount(nTotalRecords);
    // -------------------------------------------------------
    int nPreselectedIndex = -1;
    // --------------------------------
    int nIndex = -1;
    for (mapIDName::iterator ii = m_map.begin(); ii != m_map.end(); ii++)
    {
        ++nIndex; // 0 on first iteration.

        qDebug() << "MTDetailEdit Iteration: " << nIndex;
        // -------------------------------------
        QString qstrID    = ii.key();
        QString qstrValue = ii.value();

        qDebug() << QString("MTDetailEdit::RefreshRecords: Name: %1, ID: %2").arg(qstrValue, qstrID);
        // -------------------------------------
        if (!m_PreSelected.isEmpty() && (m_PreSelected == qstrID))
            nPreselectedIndex = nIndex;
        // -------------------------------------
        QWidget * pWidget = NULL;

        // -------------------------------------------
        switch (m_Type)
        {
        case MTDetailEdit::DetailEditTypeContact:
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "", ":/icons/icons/user.png");
            break;

        case MTDetailEdit::DetailEditTypeNym:
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "", ":/icons/icons/identity_BW.png");
            break;

        case MTDetailEdit::DetailEditTypeServer:
            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "", ":/icons/server");
            break;

        case MTDetailEdit::DetailEditTypeAsset:
        {
            // Not exposed yet through API. Todo.
//            QString qstrCurrencySymbol =

            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, "", "", ":/icons/icons/assets.png");
            break;
        }

        case MTDetailEdit::DetailEditTypeAccount:
        {
            QString qstrAmount = MTHome::shortAcctBalance(qstrID);

            pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, qstrAmount, "");
//          pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue, qstrAmount, "", ":/icons/icons/vault.png");
            break;
        }

        default:
            qDebug() << "MTDetailEdit::RefreshRecords: MTDetailEdit::DetailEditTypeError";
            return;
        }
        // -------------------------------------------
        if (NULL != pWidget)
            ui->tableWidget->setCellWidget( nIndex, 1, pWidget );
        else
            qDebug() << "Failed creating detail header widget in MTDetailEdit::RefreshRecords()";
    } // For loop
    // ------------------------
    if (ui->tableWidget->rowCount() > 0)
    {
        connect(ui->tableWidget, SIGNAL(currentCellChanged(int,int,int,int)),
                this, SLOT(on_tableWidget_currentCellChanged(int,int,int,int)));

        m_pTabWidget->setVisible(true);

        if ((nPreselectedIndex > (-1)) && (nPreselectedIndex < ui->tableWidget->rowCount()))
        {
            qDebug() << QString("SETTING current row to %1 on the tableWidget.").arg(nPreselectedIndex);
            ui->tableWidget->setCurrentCell(nPreselectedIndex, 1);
        }
        else
        {
            qDebug() << "SETTING current row to 0 on the tableWidget.";
            ui->tableWidget->setCurrentCell(0, 1);
        }
    }
    // ------------------------
    else
    {
        ui->deleteButton->setEnabled(false);
        m_pDetailPane->ClearContents();
        m_pTabWidget->setVisible(false);
    }
}


void MTDetailEdit::on_addButton_clicked()
{
    if (NULL != m_pDetailPane)
        m_pDetailPane->AddButtonClicked();
}

void MTDetailEdit::on_deleteButton_clicked()
{
    if (!m_qstrCurrentID.isEmpty() && (NULL != m_pDetailPane))
        m_pDetailPane->DeleteButtonClicked();
}

void MTDetailEdit::on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    m_nCurrentRow = currentRow;
    // -------------------------------------
    if (m_nCurrentRow >= 0)
    {
        m_pTabWidget->setVisible(true);

        int nIndex = -1;

        for (mapIDName::iterator ii = m_map.begin(); ii != m_map.end(); ii++)
        {
            ++nIndex; // 0 on first iteration.
            // -------------------------------------
            if (nIndex == m_nCurrentRow) // ONLY HAPPENS ONCE
            {
                m_qstrCurrentID   = ii.key();
                m_qstrCurrentName = ii.value();

                ui->deleteButton->setEnabled(true);

                qDebug() << "SETTING current row to " << nIndex << " on the tableWidget.";
                // ----------------------------------------
                m_PreSelected = m_qstrCurrentID;

                m_pDetailPane->refresh(m_qstrCurrentID, m_qstrCurrentName);
                // ----------------------------------------
                return;
            }
        }
    }
    // -------------------------------------
    m_nCurrentRow     = -1;
    m_qstrCurrentID   = QString("");
    m_qstrCurrentName = QString("");

    ui->deleteButton->setEnabled(false);

    m_pTabWidget->setVisible(false);
    // -------------------------------------
}

Moneychanger * MTDetailEdit::GetMoneychanger() { return m_pMoneychanger; }

void MTDetailEdit::SetPreSelected(QString strSelected)
{
    m_PreSelected = strSelected;
}

bool MTDetailEdit::eventFilter(QObject *obj, QEvent *event)\
{
    if (event->type() == QEvent::Close)
    {
        // -------------------------------------------
        Moneychanger * pMC = (Moneychanger *)this->parentWidget();

        if ((m_pMoneychanger == pMC) && (m_pMoneychanger != NULL))
        {
            switch (m_Type)
            {
            case MTDetailEdit::DetailEditTypeContact:
                m_pMoneychanger->close_addressbook();
                break;

            case MTDetailEdit::DetailEditTypeNym:
                m_pMoneychanger->close_nymmanager_dialog();
                break;

            case MTDetailEdit::DetailEditTypeServer:
                m_pMoneychanger->close_servermanager_dialog();
                break;

            case MTDetailEdit::DetailEditTypeAsset:
                m_pMoneychanger->close_assetmanager_dialog();
                break;

            case MTDetailEdit::DetailEditTypeAccount:
                m_pMoneychanger->close_accountmanager_dialog();
                break;

            default:
                qDebug() << "MTDetailEdit::eventFilter: MTDetailEdit::DetailEditTypeError";
            }

            return true;
        }
    }
    // -------------------------------------------
    else if (event->type() == QEvent::KeyPress)
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
//  else
    {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
