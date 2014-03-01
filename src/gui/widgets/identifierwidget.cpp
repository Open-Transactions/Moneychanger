#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/identifierwidget.hpp>
#include <ui_identifierwidget.h>


MTIdentifierWidget::MTIdentifierWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MTIdentifierWidget)
{
    ui->setupUi(this);
}

MTIdentifierWidget::~MTIdentifierWidget()
{
    delete ui;
}

void MTIdentifierWidget::on_lineEdit_textChanged(const QString &arg1)
{
    m_qstrId = arg1;
}

void MTIdentifierWidget::SetLabel(QString qstrLabelText)
{
    ui->label->setText(qstrLabelText);
}
