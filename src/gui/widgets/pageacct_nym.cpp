#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pageacct_nym.hpp>
#include <ui_pageacct_nym.h>

#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/detailedit.hpp>

#include <core/moneychanger.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>


MTPageAcct_Nym::MTPageAcct_Nym(QWidget *parent) :
    QWizardPage(parent),
    m_bFirstRun(true),
    ui(new Ui::MTPageAcct_Nym)
{
    ui->setupUi(this);

    QString style_sheet = "QPushButton{border: none; border-style: outset; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
            "QPushButton:pressed {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
            "QPushButton:hover {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

    ui->pushButtonSelect->setStyleSheet(style_sheet);

    ui->lineEditID->setStyleSheet("QLineEdit { background-color: lightgray }");

    this->registerField("NymID*",  ui->lineEditID);
    this->registerField("NymName", ui->pushButtonSelect, "text");
    // -----------------------------------------------
    connect(this, SIGNAL(SetDefaultNym(QString, QString)), Moneychanger::It(), SLOT(setDefaultNym(QString,QString)));
}


void MTPageAcct_Nym::on_pushButtonSelect_clicked()
{
    QString qstr_default_id = Moneychanger::It()->get_default_nym_id();
    // -------------------------------------------
    QString qstr_current_id = field("NymID").toString();
    // -------------------------------------------
    if (qstr_current_id.isEmpty())
        qstr_current_id = qstr_default_id;
    // -------------------------------------------
    if (qstr_current_id.isEmpty() && (OTAPI_Wrap::It()->GetNymCount() > 0))
        qstr_current_id = QString::fromStdString(OTAPI_Wrap::It()->GetNym_ID(0));
    // -------------------------------------------
    // Select from Nyms in local wallet.
    //
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t the_count = OTAPI_Wrap::It()->GetNymCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < the_count; ++ii)
    {
        QString OT_id = QString::fromStdString(OTAPI_Wrap::It()->GetNym_ID(ii));
        QString OT_name("");
        // -----------------------------------------------
        if (!OT_id.isEmpty())
        {
            if (!qstr_current_id.isEmpty() && (OT_id == qstr_current_id))
                bFoundDefault = true;
            // -----------------------------------------------
            OT_name = QString::fromStdString(OTAPI_Wrap::It()->GetNym_Name(OT_id.toStdString()));
            // -----------------------------------------------
            the_map.insert(OT_id, OT_name);
        }
     }
    // -----------------------------------------------
    if (bFoundDefault)
        theChooser.SetPreSelected(qstr_current_id);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Choose an Owner for the New Account"));
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
    // -------------------------------------------
}

void MTPageAcct_Nym::initializePage() //virtual
{
    std::string str_name;
    QString     qstr_id;
    // -------------------------------------------
    QString qstr_default_id = Moneychanger::It()->get_default_nym_id();
    // -------------------------------------------
    QString qstr_current_id = field("NymID").toString();
    // -------------------------------------------
    qstr_id = qstr_current_id.isEmpty() ? qstr_default_id : qstr_current_id;
    // -------------------------------------------
    if (qstr_id.isEmpty() && (OTAPI_Wrap::It()->GetNymCount() > 0))
        qstr_id = QString::fromStdString(OTAPI_Wrap::It()->GetNym_ID(0));
    // -------------------------------------------
    if (!qstr_id.isEmpty())
        str_name = OTAPI_Wrap::It()->GetNym_Name(qstr_id.toStdString());
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



void MTPageAcct_Nym::on_pushButtonManage_clicked()
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
    int32_t the_count = OTAPI_Wrap::It()->GetNymCount();
    bool    bStartingWithNone = (the_count < 1);

    for (int32_t ii = 0; ii < the_count; ii++)
    {
        QString OT_id   = QString::fromStdString(OTAPI_Wrap::It()->GetNym_ID(ii));
        QString OT_name = QString::fromStdString(OTAPI_Wrap::It()->GetNym_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);

        if (!qstrPreSelected.isEmpty() && (qstrPreSelected == OT_id))
            bFoundPreselected = true;
    } // for
    // -------------------------------------
    pWindow->setWindowTitle(tr("Manage Nyms (Identities)"));
    // -------------------------------------
    if (bFoundPreselected)
        pWindow->SetPreSelected(qstrPreSelected);
    // -------------------------------------
    pWindow->dialog(MTDetailEdit::DetailEditTypeNym, true);
    // -------------------------------------
    if (bStartingWithNone && (OTAPI_Wrap::It()->GetNymCount() > 0))
    {
        std::string str_id = OTAPI_Wrap::It()->GetNym_ID(0);

        if (!str_id.empty())
        {
            std::string str_name = OTAPI_Wrap::It()->GetNym_Name(str_id);

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
    else if (OTAPI_Wrap::It()->GetNymCount() < 1)
        SetFieldsBlank();
}

void MTPageAcct_Nym::SetFieldsBlank()
{
    setField("NymID",   "");
    setField("NymName", QString("<%1>").arg(tr("Click to choose Nym")));
}

MTPageAcct_Nym::~MTPageAcct_Nym()
{
    delete ui;
}
