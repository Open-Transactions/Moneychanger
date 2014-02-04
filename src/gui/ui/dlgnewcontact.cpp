#ifndef STABLE_H
#include <core/stable.h>
#endif

#include <QKeyEvent>

#include "dlgnewcontact.h"
#include "ui_dlgnewcontact.h"


MTDlgNewContact::MTDlgNewContact(QWidget *parent) :
    QDialog(parent),
    m_bFirstRun(true),
    m_pIdLayout(NULL),
    m_pIdWidget(NULL),
    ui(new Ui::MTDlgNewContact)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

MTDlgNewContact::~MTDlgNewContact()
{
    delete ui;
}



void MTDlgNewContact::on_buttonBox_accepted()
{
    if (NULL != m_pIdWidget)
        m_qstrNymID = m_pIdWidget->GetId();
}

//virtual
void MTDlgNewContact::showEvent(QShowEvent * event)
{
    if (m_bFirstRun)
    {
        m_bFirstRun = false;
        // --------------------------------------------------
        m_pIdLayout = new QGridLayout;
        m_pIdLayout->setAlignment(Qt::AlignTop);
        // --------------------------------------------------
        m_pIdWidget = new MTIdentifierWidget;
        m_pIdLayout->addWidget(m_pIdWidget);
        // --------------------------------------------------
        m_pIdLayout->setContentsMargins(0,0,0,0);

        ui->frame->setLayout(m_pIdLayout);
        // --------------------------------------------------
        QString qstrLabel = QString("%1 %2: ").arg(tr("Nym")).arg(tr("ID"));
        m_pIdWidget->SetLabel(qstrLabel);
    } // first run.
    // -------------------------------------
    // call inherited method
    //
    QDialog::showEvent(event);
}

bool MTDlgNewContact::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
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

