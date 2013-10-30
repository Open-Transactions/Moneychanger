#include <QLabel>
#include <QEvent>
#include <QKeyEvent>

#include "editdetails.h"
#include "ui_editdetails.h"

#include "detailedit.h"

MTEditDetails::MTEditDetails(QWidget *parent) :
    QWidget(parent),
    m_pOwner(NULL)
//  ui(new Ui::MTEditDetails)
{
//  ui->setupUi(this);

    this->installEventFilter(this);

//  this->setContentsMargins(0, 0, 0, 0);
}

MTEditDetails::~MTEditDetails()
{
//    delete ui;
}

void MTEditDetails::SetOwnerPointer(MTDetailEdit & theOwner)
{
    m_pOwner = &theOwner;
}


bool MTEditDetails::eventFilter(QObject *obj, QEvent *event)
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




//static
QWidget * MTEditDetails::CreateDetailHeaderWidget(QString strID, QString strName, bool bExternal/*=true*/)
{
    QString strColor("black");
    // ---------------------------------------
    //Append to transactions list in overview dialog.
    QWidget * row_widget = new QWidget;
    QGridLayout * row_widget_layout = new QGridLayout;

    row_widget_layout->setSpacing(4);
    row_widget_layout->setContentsMargins(10, 4, 10, 4);

    row_widget->setLayout(row_widget_layout);
    row_widget->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");
    // -------------------------------------------
    //Render row.
    //Header of row
//  QString tx_name = QString("Name goes here");
    QString tx_name = strName;

    if(tx_name.trimmed() == "")
    {
        //Tx has no name
        tx_name.clear();
        tx_name = "Record";
    }

    QLabel * header_of_row = new QLabel;
    QString header_of_row_string = QString("");
    header_of_row_string.append(tx_name);

    header_of_row->setText(header_of_row_string);

    //Append header to layout
    row_widget_layout->addWidget(header_of_row, 0, 0, 1,1, Qt::AlignLeft);
    // -------------------------------------------
    // Amount (with currency tla)
    QLabel * currency_amount_label = new QLabel;
    QString currency_amount;

    currency_amount_label->setStyleSheet(QString("QLabel { color : %1; }").arg(strColor));
    // ----------------------------------------------------------------
//  currency_amount = QString("amount goes here");
    currency_amount = QString("");
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


    QLabel * row_content_date_label = new QLabel;
//    QString row_content_date_label_string("Date goes here");
    QString row_content_date_label_string(strID);

    row_content_date_label->setStyleSheet("QLabel { color : grey; font-size:11pt;}");
    row_content_date_label->setText(row_content_date_label_string);

    row_content_grid->addWidget(row_content_date_label, 0,0, 1,1, Qt::AlignLeft);
    // -------------------------------------------
    // Column two
    std::string str_desc("");
//  std::string str_desc("Description goes here");
    // ---------------------------------------
    //Status
    QLabel * row_content_status_label = new QLabel;
    QString row_content_status_string;

    row_content_status_string.append(QString::fromStdString(str_desc));
    // -------------------------------------------
    //add string to label
    row_content_status_label->setStyleSheet("QLabel { color : grey; font-size:11pt;}");
    row_content_status_label->setWordWrap(false);
    row_content_status_string.replace("\r\n"," ");
    row_content_status_string.replace("\n\r"," ");
    row_content_status_string.replace("\n",  " ");
    row_content_status_label->setText(row_content_status_string);

    //add to row_content grid
    row_content_grid->addWidget(row_content_status_label, 0,1, 1,1, Qt::AlignRight);
    // -------------------------------------------
    return row_widget;
}

