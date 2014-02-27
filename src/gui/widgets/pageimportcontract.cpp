#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

#include "pageimportcontract.h"
#include "ui_pageimportcontract.h"

MTPageImportContract::MTPageImportContract(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MTPageImportContract)
{
    ui->setupUi(this);

    ui->radioButton_1->setChecked(true);

    registerField("isURL",      ui->radioButton_1);
    registerField("isFilename", ui->radioButton_2);
    registerField("isContents", ui->radioButton_3);

    registerField("URL",      ui->lineEditURL);
    registerField("Filename", ui->lineEditFilename);
}

MTPageImportContract::~MTPageImportContract()
{
    delete ui;
}


//virtual
int MTPageImportContract::nextId() const
{
    // -1 turns it into the last page.
    //
    return -1;
}


bool MTPageImportContract::validatePage()
{
    // ------------------------------------
    if (ui->radioButton_1->isChecked())
        return (!ui->lineEditURL->text().isEmpty());
    else if (ui->radioButton_2->isChecked())
        return (!ui->lineEditFilename->text().isEmpty());
    else if (ui->radioButton_3->isChecked())
        return (!ui->plainTextEditPaste->toPlainText().isEmpty());
    // ------------------------------------------
    return false;
}

// ------------------------------------------

void MTPageImportContract::on_pushButton_clicked()
{
    // -------------------------------------------------
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Import Contract"),
        "", //      "/home/au",
        tr("Contracts (*.otc)"));

    if (!fileName.isEmpty())
        ui->lineEditFilename->setText(fileName);
    // -----------------------------------------------
}

// ------------------------------------------

void MTPageImportContract::on_plainTextEditPaste_textChanged()
{
    m_qstrContents = ui->plainTextEditPaste->toPlainText();
}

// ------------------------------------------

//virtual
bool MTPageImportContract::isComplete() const
{
    return QWizardPage::isComplete();
}

void MTPageImportContract::on_radioButton_1_toggled(bool checked)
{
    ui->lineEditURL->setReadOnly(!checked);
}

void MTPageImportContract::on_radioButton_2_toggled(bool checked)
{
    ui->lineEditFilename->setReadOnly(!checked);

    ui->pushButton->setEnabled(checked);
}

void MTPageImportContract::on_radioButton_3_toggled(bool checked)
{
    ui->plainTextEditPaste->setReadOnly(!checked);
}





