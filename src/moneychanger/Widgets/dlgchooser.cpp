#include <QDebug>
#include <QLabel>
#include <QDateTime>
#include <QKeyEvent>
#include <QPushButton>

#include "dlgchooser.h"
#include "ui_dlgchooser.h"

#include "home.h"

DlgChooser::DlgChooser(QWidget *parent, QString qstrInstructions/*=QString("")*/) :
    QDialog(parent),
    m_bFirstRun(true),
    m_bIsAccounts(false),
    m_qstrInstructions(qstrInstructions),
    m_nCurrentRow(-1),
    ui(new Ui::DlgChooser)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    if (!m_qstrInstructions.isEmpty())
    {
        QLabel * pLabel = new QLabel(m_qstrInstructions);
        pLabel->setWordWrap(true);

        ui->verticalLayout->insertWidget(0, pLabel);
    }
}

DlgChooser::~DlgChooser()
{
    delete ui;
}



void DlgChooser::SetPreSelected(QString strSelected)
{
    m_PreSelected = strSelected;
}


//virtual
void DlgChooser::showEvent(QShowEvent * event)
{
    if (m_bFirstRun)
    {
        m_bFirstRun = false;
        // ----------------------
        ui->tableWidget->setRowCount(m_map.size());
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

//        ui->verticalLayout->setSizePolicy(
//                    QSizePolicy::Expanding,
//                    QSizePolicy::Expanding);

        int nPreselectedIndex = -1;
        // --------------------------------
        int nIndex = -1;
        for (mapIDName::iterator ii = m_map.begin(); ii != m_map.end(); ii++)
        {
            ++nIndex; // 0 on first iteration.

            qDebug() << "Chooser Iteration: " << nIndex;
            // -------------------------------------
            QString qstrID    = ii.key();
            QString qstrValue = ii.value();
            // -------------------------------------
            if (!m_PreSelected.isEmpty() && (m_PreSelected == qstrID))
                nPreselectedIndex = nIndex;
            // -------------------------------------
            QWidget     * row_widget        = new QWidget;
            QGridLayout * row_widget_layout = new QGridLayout;

            row_widget_layout->setSpacing(4);
            row_widget_layout->setContentsMargins(10, 4, 10, 4);

            row_widget->setLayout(row_widget_layout);
            row_widget->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");
            // -------------------------------------------
            //Render row.
            //Header of row
            QString qstr_name = qstrValue;
//          QString qstr_name = tr("NAME GOES HERE");

            QLabel * header_of_row = new QLabel;
            QString header_of_row_string = QString("");
            header_of_row_string.append(qstr_name);

            header_of_row->setText(header_of_row_string);

            //Append header to layout
            row_widget_layout->addWidget(header_of_row, 0, 0, 1,1, Qt::AlignLeft);
            // -------------------------------------------
            // Amount (with currency tla)
            QLabel * currency_amount_label = new QLabel;
//          QString currency_amount = tr("AMOUNT");
            QString currency_amount = QString("");

            if (m_bIsAccounts)
            {
                currency_amount = MTHome::shortAcctBalance(qstrID);
            }

            currency_amount_label->setStyleSheet(QString("QLabel { color : %1; }").arg(QString("black")));
            // ----------------------------------------------------------------
            currency_amount_label->setText(currency_amount);
            // ----------------------------------------------------------------
            row_widget_layout->addWidget(currency_amount_label, 0, 1, 1,1, Qt::AlignRight);
            // -------------------------------------------
            //Sub-info
            QWidget * row_content_container = new QWidget;
            QGridLayout * row_content_grid = new QGridLayout;

            // left top right bottom

            row_content_grid->setSpacing(4);
            row_content_grid->setContentsMargins(3, 4, 3, 4);

            row_content_container->setLayout(row_content_grid);

            row_widget_layout->addWidget(row_content_container, 1,0, 1,2);
            // -------------------------------------------
            // Column one
            //Date (sub-info)
            //Calc/convert date/times
            QDateTime timestamp;

            long lDate = 0;

            timestamp.setTime_t(lDate);

            QLabel * row_content_date_label = new QLabel;
            QString row_content_date_label_string;
            row_content_date_label_string.append(qstrID); // NymID, acct ID, etc.
//          row_content_date_label_string.append(QString(timestamp.toString(Qt::SystemLocaleShortDate)));

            row_content_date_label->setStyleSheet("QLabel { color : grey; font-size:11pt;}");
            row_content_date_label->setText(row_content_date_label_string);

            row_content_grid->addWidget(row_content_date_label, 0,0, 1,1, Qt::AlignLeft);
            // -------------------------------------------
            // Column two
            //Status
            QLabel * row_content_status_label = new QLabel;
            QString row_content_status_string;

//          row_content_status_string.append(tr("DESCRIPTION"));
            row_content_status_string.append(QString(""));
            // -------------------------------------------
            //add string to label
            row_content_status_label->setStyleSheet("QLabel { color : grey; font-size:11pt;}");
            row_content_status_label->setWordWrap(false);
            row_content_status_label->setText(row_content_status_string);

            //add to row_content grid
            row_content_grid->addWidget(row_content_status_label, 0,1, 1,1, Qt::AlignRight);
            // -------------------------------------------
            ui->tableWidget->setCellWidget( nIndex, 1, row_widget );

            if (NULL == ui->tableWidget->cellWidget(nIndex, 1))
                qDebug() << "FAILED adding row_widget as cellWidget on tableWidget!";
        } // For loop
        // ------------------------
        if (ui->tableWidget->rowCount() > 0)
        {
            if (nPreselectedIndex > (-1))
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
    } // first run.


    // ----------------------------------
    // call inherited method
    //
    QDialog::showEvent(event);
}

bool DlgChooser::eventFilter(QObject *obj, QEvent *event)
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
    else
    {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void DlgChooser::on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn);
    Q_UNUSED(previousRow);
    Q_UNUSED(previousColumn);

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

                ui->pushButton->setEnabled(true);

                qDebug() << "SETTING current row to " << nIndex << " on the tableWidget.";

                return;
            }
        }
    }
    // -------------------------------------
    m_nCurrentRow     = -1;
    m_qstrCurrentID   = QString("");
    m_qstrCurrentName = QString("");

    ui->pushButton->setEnabled(false);
}

void DlgChooser::on_tableWidget_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    this->on_pushButton_clicked();
}

void DlgChooser::on_cancelButton_clicked()
{
    QDialog::reject();
}

void DlgChooser::on_pushButton_clicked()
{
    QDialog::accept();
}


