#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/editdetails.hpp>
#include <ui_editdetails.h>

#include <QDebug>
#include <QLabel>


MTEditDetails::MTEditDetails(QWidget *parent, MTDetailEdit & theOwner) :
    QWidget(parent),
    m_pOwner(&theOwner),
    m_Type(MTDetailEdit::DetailEditTypeError)
//  ui(new Ui::MTEditDetails)
{
//  ui->setupUi(this);

    this->installEventFilter(this);

//  this->setContentsMargins(0, 0, 0, 0);
}

MTEditDetails::~MTEditDetails()
{
    // Subclasses do this -- this base class has no ui.
    //
//    delete ui;
}

void MTEditDetails::SetOwnerPointer(MTDetailEdit & theOwner)
{
    m_pOwner = &theOwner;
}


bool MTEditDetails::eventFilter(QObject *obj, QEvent *event)
{
//    if (event->type() == QEvent::KeyPress)
//    {
//        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

//        if (keyEvent->key() == Qt::Key_Escape)
//        {
//            close(); // This is caught by this same filter.
//            return true;
//        }
//        return true;
//    }
//    else
    {
        // standard event processing
        return QWidget::eventFilter(obj, event);
    }
}


// ----------------------------------
//virtual
int MTEditDetails::GetCustomTabCount()
{
    return 0;
}
// ----------------------------------
//virtual
QWidget * MTEditDetails::CreateCustomTab(int nTab)
{
    const int nCustomTabCount = this->GetCustomTabCount();
    // -----------------------------
    if ((nTab < 0) || (nTab >= nCustomTabCount))
        return NULL; // out of bounds.
    // -----------------------------
    QWidget * pReturnValue = NULL;
    // -----------------------------
    switch (nTab)
    {
    default:
        qDebug() << "Unexpected: MTEditDetails::CreateCustomTab was called instead of overridden version of same method.";
        return NULL;
    }
    // -----------------------------
    return pReturnValue;
}

QWidget * MTEditDetails::GetTab(int nTab)
{
    QWidget * pReturn = nullptr;

    if (!m_pOwner)
        return nullptr;

    pReturn = m_pOwner->GetTab(nTab);

    return pReturn;
}
// ---------------------------------
//virtual
QString  MTEditDetails::GetCustomTabName(int nTab)
{
    const int nCustomTabCount = this->GetCustomTabCount();
    // -----------------------------
    if ((nTab < 0) || (nTab >= nCustomTabCount))
        return QString(""); // out of bounds.
    // -----------------------------
    QString qstrReturnValue("");
    // -----------------------------
    switch (nTab)
    {
    default:
        qDebug() << "Unexpected: MTEditDetails::GetCustomTabName was called instead of overridden version of same method.";
        return QString("");
    }
    // -----------------------------
    return qstrReturnValue;
}
// ----------------------------------


//static
QWidget * MTEditDetails::CreateDetailHeaderWidget(MTDetailEdit::DetailEditType theType,
                                                  QString strID, QString strName,
                                                  QString strAmount/*=QString("")*/,
                                                  QString strStatus/*=QString("")*/,
                                                  QString strPixmap/*=QString("")*/,
                                                  bool bExternal/*=true*/)
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
//  QString tx_name = tr("Name goes here");
    QString tx_name = strName;

    if(tx_name.trimmed() == "")
    {
        //Tx has no name
        tx_name.clear();
        tx_name = "";
    }

    QLabel * header_of_row = new QLabel;

    if (!bExternal)
        header_of_row->setStyleSheet(QString("QLabel { font-size: 18px }"));
    // -----------------------------------------
    QString header_of_row_string = QString("");
    header_of_row_string.append(tx_name);

    header_of_row->setText(header_of_row_string);

    //Append header to layout
    row_widget_layout->addWidget(header_of_row, 0, 0, 1,1, Qt::AlignLeft);
    // -------------------------------------------
    // Amount (with currency tla)
    QLabel * currency_amount_label = new QLabel;
    QString currency_amount;

    if (!bExternal && (MTDetailEdit::DetailEditTypeMarket != theType))
        currency_amount_label->setStyleSheet(QString("QLabel { color : %1; font-size: 18px }").arg(strColor));
    else
        currency_amount_label->setStyleSheet(QString("QLabel { color : %1; font-size: 16px }").arg(strColor));
    // ----------------------------------------------------------------
//  currency_amount = tr("amount goes here");
//  currency_amount = strAmount;

    if (!bExternal && !strAmount.isEmpty() && (MTDetailEdit::DetailEditTypeAccount == theType))
        currency_amount = strAmount;
//      currency_amount = QString("<small><font color=grey>%1:</font></small> %2").arg(tr("Balance")).arg(strAmount);
    else
        currency_amount = strAmount;
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
//  QString row_content_date_label_string(tr("Date goes here"));
    QString row_content_date_label_string(strID);

    row_content_date_label->setStyleSheet("QLabel { color : grey; font-size:11pt;}");
    row_content_date_label->setText(row_content_date_label_string);

    row_content_grid->addWidget(row_content_date_label, 0,0, 1,1, Qt::AlignLeft);
    // -------------------------------------------
    // Column two
    std::string str_desc = strStatus.toStdString();
//  std::string str_desc(tr("Description goes here"));
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

    if (strPixmap.isEmpty())
        return row_widget;

    // -------------------------------------------
    QWidget     * pOverall     = new QWidget;
    QHBoxLayout * pFinalLayout = new QHBoxLayout;
    // ----------------------------------------------------------------
    QPixmap pixmap(strPixmap);
//  QPixmap pixmap(":/icons/icons/user.png");
    // ----------------------------------------------------------------
    QLabel * pPixmapLabel = new QLabel;
    pPixmapLabel->setPixmap(pixmap);
    pPixmapLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    row_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    if (bExternal)
        pOverall->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    else
        pOverall->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // ----------------------------------------------------------------
    pFinalLayout->setContentsMargins(3,0,0,0);
    // ----------------------------------------------------------------
    pFinalLayout->addWidget(pPixmapLabel);
    pFinalLayout->addWidget(row_widget);
    // ----------------------------------------------------------------
    pOverall->setLayout(pFinalLayout);
    // ----------------------------------------------------------------
//    if (!bExternal)
//        pOverall->setStyleSheet("QWidget#DetailHeader { border: 1px solid gray; }");

    return pOverall;
}

