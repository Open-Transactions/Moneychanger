#include <QDebug>

#include "detailedit.h"
#include "ui_detailedit.h"

#include "editdetails.h"
#include "contactdetails.h"

#include "moneychanger.h"

MTDetailEdit::MTDetailEdit(QWidget *parent) :
    QWidget(parent, Qt::Window),
    m_bFirstRun(true),
    m_nCurrentRow(-1),
    m_pDetailPane(NULL),
    m_pDetailLayout(NULL),
    m_Type(MTDetailEdit::DetailEditTypeError),
    ui(new Ui::MTDetailEdit)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

MTDetailEdit::~MTDetailEdit()
{
    delete m_pDetailPane;
    delete m_pDetailLayout;

    delete ui;
}


void MTDetailEdit::dialog(MTDetailEdit::DetailEditType theType)
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
        // -------------------------------------------
        m_Type = theType;

        switch (m_Type)
        {
        case MTDetailEdit::DetailEditTypeContact:
        case MTDetailEdit::DetailEditTypeNym:
        case MTDetailEdit::DetailEditTypeServer:
        case MTDetailEdit::DetailEditTypeAsset:
        case MTDetailEdit::DetailEditTypeAccount:

            m_pDetailPane = new MTContactDetails;
            break;

        default:
            qDebug() << "MTDetailEdit::dialog: MTDetailEdit::DetailEditTypeError";
        }
        // -------------------------------------------
        m_pDetailPane->SetOwnerPointer(*this);
        // -------------------------------------------
        m_pDetailLayout = new QVBoxLayout;
        m_pDetailLayout->addWidget(m_pDetailPane);

        m_pDetailPane  ->setContentsMargins(1,1,1,1);
        m_pDetailLayout->setContentsMargins(1,1,1,1);

        ui->widget->setContentsMargins(1,1,1,1);

//        if (NULL != ui->widget->layout())
//            delete ui->widget->layout();

        ui->widget->setLayout(m_pDetailLayout);
    } // first run.
    // -------------------------------------------
    RefreshRecords();
    // -------------------------------------------
    show();
    setFocus();
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
        QWidget * pWidget  = MTEditDetails::CreateDetailHeaderWidget(qstrID, qstrValue);
        // -------------------------------------------
        if (NULL != pWidget)
            ui->tableWidget->setCellWidget( nIndex, 1, pWidget );
        else
            qDebug() << "Failed creating detail header widget in MTDetailEdit::RefreshRecords()";
    } // For loop
    // ------------------------
    if (ui->tableWidget->rowCount() > 0)
    {
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
        int nIndex = -1;

        for (mapIDName::iterator ii = m_map.begin(); ii != m_map.end(); ii++)
        {
            ++nIndex; // 0 on first iteration.
            // -------------------------------------
            if (nIndex == m_nCurrentRow)
            {
                m_qstrCurrentID   = ii.key();
                m_qstrCurrentName = ii.value();

                ui->editButton->setEnabled(true);
                ui->deleteButton->setEnabled(true);

                qDebug() << "SETTING current row to " << nIndex << " on the tableWidget.";
                // ----------------------------------------
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

    ui->editButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
    // -------------------------------------
}


void MTDetailEdit::SetPreSelected(QString strSelected)
{
    m_PreSelected = strSelected;
}

bool MTDetailEdit::eventFilter(QObject *obj, QEvent *event)\
{
    if (event->type() == QEvent::Close)
    {
        // -------------------------------------------
        switch (m_Type)
        {
        case MTDetailEdit::DetailEditTypeContact:
            ((Moneychanger *)parentWidget())->close_addressbook();
            break;

        case MTDetailEdit::DetailEditTypeNym:
            ((Moneychanger *)parentWidget())->close_nymmanager_dialog();
            break;

        case MTDetailEdit::DetailEditTypeServer:
            ((Moneychanger *)parentWidget())->close_servermanager_dialog();
            break;

        case MTDetailEdit::DetailEditTypeAsset:
            ((Moneychanger *)parentWidget())->close_assetmanager_dialog();
            break;

        case MTDetailEdit::DetailEditTypeAccount:
            ((Moneychanger *)parentWidget())->close_accountmanager_dialog();
            break;

        default:
            qDebug() << "MTDetailEdit::eventFilter: MTDetailEdit::DetailEditTypeError";
        }
        // -------------------------------------------
        return true;
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Escape){
            close(); // This is caught by this same filter.
            return true;
        }
        return true;
    }else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
