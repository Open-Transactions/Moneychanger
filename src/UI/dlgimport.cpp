#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>
#include <QFileDialog>

#include "dlgimport.h"
#include "ui_dlgimport.h"

DlgImport::DlgImport(QWidget *parent) :
    QDialog(parent),
    m_bPasted(false),
    m_qstrFilename(""),
    m_qstrPaste(""),
    ui(new Ui::DlgImport)
{
    ui->setupUi(this);

    this->installEventFilter(this);
}

DlgImport::~DlgImport()
{
    delete ui;
}

void DlgImport::on_radioButton_clicked()
{
    m_bPasted = false;
    ui->plainTextEdit->setEnabled(false);
    ui->lineEdit     ->setEnabled(true);
}

void DlgImport::on_radioButton_2_clicked()
{
    m_bPasted = true;
    ui->plainTextEdit->setEnabled(true);
    ui->lineEdit     ->setEnabled(false);
}

void DlgImport::on_toolButton_clicked()
{
    // -------------------------------------------------
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Import Cash"),
        "", //      "/home/au",
        tr("Cash Purse (*.otc)"));

    if (!fileName.isEmpty())
        ui->lineEdit->setText(fileName);
    // -----------------------------------------------
}

void DlgImport::on_buttonBox_accepted()
{
    if (m_bPasted)
    {
        m_qstrFilename = QString("");
        m_qstrPaste    = ui->plainTextEdit->toPlainText();
        // ----------------------------------------
        if (m_qstrPaste.isEmpty())
        {
            QMessageBox::warning(this, tr("Contents are Blank"),
                                 tr("The contents are blank. Please paste a contract before continuing."));
            return;
        }
    }
    else
    {
        m_qstrFilename = ui->lineEdit->text();
        m_qstrPaste    = QString("");
        // ----------------------------------------
        if (m_qstrFilename.isEmpty())
        {
            QMessageBox::warning(this, tr("Filename is Blank"),
                                 tr("The filename is blank. Please choose a file before continuing."));
            return;
        }
    }
    // ----------------------------------
    accept();
}


bool DlgImport::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if(keyEvent->key() == Qt::Key_Escape)
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



void DlgImport::on_buttonBox_rejected()
{
    reject();
}
