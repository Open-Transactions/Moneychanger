#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/ui/msgheaderwidget.hpp>
#include <ui_msgheaderwidget.h>

MsgHeaderWidget::MsgHeaderWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MsgHeaderWidget)
{
    ui->setupUi(this);

    ui->labelSender->setStyleSheet("QLabel { font-weight: bold; }");
    // ----------------------------------
    ui->toolButtonSigned->setAutoRaise(true);
    ui->toolButtonSigned->setAutoRaise(true);
}

void MsgHeaderWidget::setSubject(QString qstrSubject)
{
    ui->labelSubject->setText(qstrSubject);
}

void MsgHeaderWidget::setSender(QString qstrSender)
{
    ui->labelSender->setText(qstrSender);
}

void MsgHeaderWidget::setRecipient(QString qstrRecipient)
{
    ui->labelRecipient->setText(qstrRecipient);
}

void MsgHeaderWidget::setTimestamp(QString qstrTimestamp)
{
    ui->labelTimestamp->setText(qstrTimestamp);
}

void MsgHeaderWidget::setFolder(QString qstrFolder)
{
    ui->labelFolder->setText(qstrFolder);
    ui->labelFolder->setAlignment(Qt::AlignRight);
}

MsgHeaderWidget::~MsgHeaderWidget()
{
    delete ui;
}
