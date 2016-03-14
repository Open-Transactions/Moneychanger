#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pagecreatecontract.hpp>
#include <ui_pagecreatecontract.h>

#include <gui/widgets/wizardaddcontract.hpp>

#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/detailedit.hpp>

#include <core/moneychanger.hpp>

#include <QClipboard>

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>


MTPageCreateContract::MTPageCreateContract(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MTPageCreateContract)
{
    ui->setupUi(this);

    QString style_sheet = "QPushButton{border: none; border-style: outset; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
            "QPushButton:pressed {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
            "QPushButton:hover {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

    ui->pushButtonSelect->setStyleSheet(style_sheet);

    ui->lineEditID->setStyleSheet("QLineEdit { background-color: lightgray }");

    this->registerField("NymID*",  ui->lineEditID);
    this->registerField("NymName", ui->pushButtonSelect, "text");

    this->registerField("contractXML*", ui->plainTextEdit, "plainText", "textChanged");
    // -----------------------------------------------
    connect(this, SIGNAL(SetDefaultNym(QString, QString)), Moneychanger::It(), SLOT(setDefaultNym(QString,QString)));
}

MTPageCreateContract::~MTPageCreateContract()
{
    delete ui;
}

void MTPageCreateContract::SetFieldsBlank()
{
    setField("NymID",   "");
    setField("NymName", QString("<%1>").arg(tr("Click to choose Nym")));
}


void MTPageCreateContract::initializePage() //virtual
{
    if (!Moneychanger::It()->expertMode())
    {
        ui->pushButtonManage->setVisible(false);
    }
    // -------------------------------------------
    std::string str_name;
    QString     qstr_id;
    // -------------------------------------------
    QString qstr_default_id = Moneychanger::It()->get_default_nym_id();
    // -------------------------------------------
    QString qstr_current_id = field("NymID").toString();
    // -------------------------------------------
    qstr_id = qstr_current_id.isEmpty() ? qstr_default_id : qstr_current_id;
    // -------------------------------------------
    if (qstr_id.isEmpty() && (opentxs::OTAPI_Wrap::It()->GetNymCount() > 0))
        qstr_id = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_ID(0));
    // -------------------------------------------
    if (!qstr_id.isEmpty())
        str_name = opentxs::OTAPI_Wrap::It()->GetNym_Name(qstr_id.toStdString());
    // -------------------------------------------
    QString qstrContractType = field("contractType").toString();

    if (0 == qstrContractType.compare("smart"))
    {
        ui->pushButton->setVisible(false);
        ui->plainTextEdit->setVisible(false);
        ui->label->setVisible(false);
    }
    // -------------------------------------------
    if (str_name.empty() || qstr_id.isEmpty())
        SetFieldsBlank();
    else
    {
        QString qstrName = QString::fromStdString(str_name);
        // ---------------------------
        setField("NymID",   qstr_id);
        setField("NymName", qstrName);
        // ---------------------------
        ui->lineEditID->home(false);
        // ---------------------------
        if (qstr_default_id.isEmpty())
            emit SetDefaultNym(qstr_id, qstrName);
    }
}

//virtual
int MTPageCreateContract::nextId() const
{
    // -1 turns it into the last page.
    //
//    return -1;

    MTWizardAddContract * pWizard  = dynamic_cast<MTWizardAddContract *>(wizard());

    if (nullptr != pWizard)
    {
        if (pWizard->serverMode())
        {
            return -1;
        }
    }

    if (wizard()->field("isImporting").toBool())
        return -1; // No need to collect the terms then.

    return QWizardPage::nextId();
 }


void MTPageCreateContract::on_pushButton_clicked()
{
    // Get text from the clipboard, and add it to the plainTextEdit widget
    //
    QClipboard *clipboard = QApplication::clipboard();

    if (NULL != clipboard)
    {
        QString qstrClipText = clipboard->text();

        if (!qstrClipText.isEmpty())
            ui->plainTextEdit->insertPlainText(qstrClipText);
    }
}

void MTPageCreateContract::on_pushButtonManage_clicked()
{
    MTDetailEdit * pWindow = new MTDetailEdit(this);

    pWindow->setAttribute(Qt::WA_DeleteOnClose);
    // -------------------------------------
    mapIDName & the_map = pWindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    QString qstrPreSelected   = field("NymID").toString();
    bool    bFoundPreselected = false;
    // -------------------------------------
    int32_t the_count = opentxs::OTAPI_Wrap::It()->GetNymCount();
    bool    bStartingWithNone = (the_count < 1);

    for (int32_t ii = 0; ii < the_count; ii++)
    {
        QString OT_id   = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_ID(ii));
        QString OT_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);

        if (!qstrPreSelected.isEmpty() && (0 == qstrPreSelected.compare(OT_id)))
            bFoundPreselected = true;
    } // for
    // -------------------------------------
    pWindow->setWindowTitle(tr("Manage My Identities"));
    // -------------------------------------
    if (bFoundPreselected)
        pWindow->SetPreSelected(qstrPreSelected);
    // -------------------------------------
    pWindow->dialog(MTDetailEdit::DetailEditTypeNym, true);
    // -------------------------------------
    if (bStartingWithNone && (opentxs::OTAPI_Wrap::It()->GetNymCount() > 0))
    {
        std::string str_id = opentxs::OTAPI_Wrap::It()->GetNym_ID(0);

        if (!str_id.empty())
        {
            std::string str_name = opentxs::OTAPI_Wrap::It()->GetNym_Name(str_id);

            if (str_name.empty())
                str_name = str_id;
            // --------------------------------
            setField("NymID",   QString::fromStdString(str_id));
            setField("NymName", QString::fromStdString(str_name));
            // --------------------------------
            ui->lineEditID->home(false);
        }
    }
    // -------------------------------------
    else if (opentxs::OTAPI_Wrap::It()->GetNymCount() < 1)
        SetFieldsBlank();
}

void MTPageCreateContract::on_pushButtonSelect_clicked()
{
    QString qstr_default_id = Moneychanger::It()->get_default_nym_id();
    // -------------------------------------------
    QString qstr_current_id = field("NymID").toString();
    // -------------------------------------------
    if (qstr_current_id.isEmpty())
        qstr_current_id = qstr_default_id;
    // -------------------------------------------
    if (qstr_current_id.isEmpty() && (opentxs::OTAPI_Wrap::It()->GetNymCount() > 0))
        qstr_current_id = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_ID(0));
    // -------------------------------------------
    // Select from Nyms in local wallet.
    //
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t the_count = opentxs::OTAPI_Wrap::It()->GetNymCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < the_count; ++ii)
    {
        QString OT_id = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_ID(ii));
        QString OT_name("");
        // -----------------------------------------------
        if (!OT_id.isEmpty())
        {
            if (!qstr_current_id.isEmpty() && (0 == qstr_current_id.compare(OT_id)))
                bFoundDefault = true;
            // -----------------------------------------------
            OT_name = QString::fromStdString(opentxs::OTAPI_Wrap::It()->GetNym_Name(OT_id.toStdString()));
            // -----------------------------------------------
            the_map.insert(OT_id, OT_name);
        }
     }
    // -----------------------------------------------
    if (bFoundDefault)
        theChooser.SetPreSelected(qstr_current_id);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Choose a Signer for the New Contract"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        if (!theChooser.m_qstrCurrentID  .isEmpty() &&
            !theChooser.m_qstrCurrentName.isEmpty())
        {
            setField("NymID",   theChooser.m_qstrCurrentID);
            setField("NymName", theChooser.m_qstrCurrentName);
            // -----------------------------------------
            ui->lineEditID->home(false);
            // -----------------------------------------
            if (qstr_default_id.isEmpty())
                emit SetDefaultNym(theChooser.m_qstrCurrentID, theChooser.m_qstrCurrentName);
            // ----------------------------------------
            return;
        }
    }
}
