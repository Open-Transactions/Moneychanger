#ifndef STABLE_H
#include <core/stable.h>
#endif

#include <QApplication>
#include <QMessageBox>
#include <QClipboard>
#include <QKeyEvent>
#include <QDebug>

#include "dlgexportedcash.h"
#include "ui_dlgexportedcash.h"

DlgExportedCash::DlgExportedCash(QWidget *parent, QString qstrHisCopy, QString qstrMyCopy) :
    QDialog(parent),
    ui(new Ui::DlgExportedCash)
{
    ui->setupUi(this);

    this->installEventFilter(this);

    ui->plainTextEdit    ->setPlainText(qstrHisCopy);
    ui->plainTextEditCopy->setPlainText(qstrMyCopy);
}

DlgExportedCash::~DlgExportedCash()
{
    delete ui;
}

void DlgExportedCash::on_buttonBox_accepted()
{
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this, "", QString("%1<br/><br/>%2").arg(tr("Are you sure you want to close this dialog?")).
                                  arg(tr("WARNING: You are strongly advised to first SAVE BOTH COPIES of the exported cash!")),
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
        accept();
}

void DlgExportedCash::on_pushButton_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        clipboard->setText(ui->plainTextEdit->toPlainText());

        QMessageBox::information(this, tr("Recipient Cash Copied"), tr("Copied recipient's cash to the clipboard"));
    }
}

void DlgExportedCash::on_pushButtonCopy_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        clipboard->setText(ui->plainTextEditCopy->toPlainText());

        QMessageBox::information(this, tr("Your Copy of Cash Copied"), tr("Your copy of the cash was copied to the Clipboard"));
    }
}

bool DlgExportedCash::eventFilter(QObject *obj, QEvent *event)
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

