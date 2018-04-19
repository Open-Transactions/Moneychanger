#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pageacct_server.hpp>
#include <ui_pageacct_server.h>

#include <gui/widgets/dlgchooser.hpp>
#include <gui/widgets/detailedit.hpp>

#include <core/moneychanger.hpp>

#include <opentxs/opentxs.hpp>

MTPageAcct_Server::MTPageAcct_Server(QWidget *parent) :
    QWizardPage(parent),
    m_bFirstRun(true),
    ui(new Ui::MTPageAcct_Server)
{
    ui->setupUi(this);

    QString style_sheet = "QPushButton{border: none; border-style: outset; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa);}"
            "QPushButton:pressed {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }"
            "QPushButton:hover {border: 1px solid black; text-align:left; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #dadbde, stop: 1 #f6f7fa); }";

    ui->pushButtonSelect->setStyleSheet(style_sheet);

    ui->lineEditID->setStyleSheet("QLineEdit { background-color: lightgray }");

    this->registerField("NotaryID*",  ui->lineEditID);
    this->registerField("ServerName", ui->pushButtonSelect, "text");
    // -----------------------------------------------
    connect(this, SIGNAL(SetDefaultServer(QString, QString)), Moneychanger::It(), SLOT(setDefaultServer(QString,QString)));
}



void MTPageAcct_Server::on_pushButtonSelect_clicked()
{
    QString qstr_default_id = Moneychanger::It()->get_default_notary_id();
    // -------------------------------------------
    QString qstr_current_id = field("NotaryID").toString();
    // -------------------------------------------
    if (qstr_current_id.isEmpty())
        qstr_current_id = qstr_default_id;
    // -------------------------------------------
    if (qstr_current_id.isEmpty() && (opentxs::OT::App().API().Exec().GetServerCount() > 0))
        qstr_current_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_ID(0));
    // -------------------------------------------
    // Select from Servers in local wallet.
    //
    DlgChooser theChooser(this);
    // -----------------------------------------------
    mapIDName & the_map = theChooser.m_map;

    bool bFoundDefault = false;
    // -----------------------------------------------
    const int32_t the_count = opentxs::OT::App().API().Exec().GetServerCount();
    // -----------------------------------------------
    for (int32_t ii = 0; ii < the_count; ++ii)
    {
        QString OT_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_ID(ii));
        QString OT_name("");
        // -----------------------------------------------
        if (!OT_id.isEmpty())
        {
            if (!qstr_current_id.isEmpty() && (0 == qstr_current_id.compare(OT_id)))
                bFoundDefault = true;
            // -----------------------------------------------
            OT_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_Name(OT_id.toStdString()));
            // -----------------------------------------------
            the_map.insert(OT_id, OT_name);
        }
     }
    // -----------------------------------------------
    if (bFoundDefault)
        theChooser.SetPreSelected(qstr_current_id);
    // -----------------------------------------------
    theChooser.setWindowTitle(tr("Select the Server"));
    // -----------------------------------------------
    if (theChooser.exec() == QDialog::Accepted)
    {
        if (!theChooser.m_qstrCurrentID  .isEmpty() &&
            !theChooser.m_qstrCurrentName.isEmpty())
        {
            setField("NotaryID",   theChooser.m_qstrCurrentID);
            setField("ServerName", theChooser.m_qstrCurrentName);
            // -----------------------------------------
            ui->lineEditID->home(false);
            // -----------------------------------------
            if (qstr_default_id.isEmpty())
                emit SetDefaultServer(theChooser.m_qstrCurrentID, theChooser.m_qstrCurrentName);
        }
    }
}

void MTPageAcct_Server::initializePage() //virtual
{
    if (!Moneychanger::It()->expertMode())
    {
        ui->pushButtonManage->setVisible(false);
    }
    // -------------------------------------------
    std::string str_name;
    QString     qstr_id;
    // -------------------------------------------
    QString qstr_default_id = Moneychanger::It()->get_default_notary_id();
    // -------------------------------------------
    QString qstr_current_id = field("NotaryID").toString();
    // -------------------------------------------
    qstr_id = qstr_current_id.isEmpty() ? qstr_default_id : qstr_current_id;
    // -------------------------------------------
    if (qstr_id.isEmpty() && (opentxs::OT::App().API().Exec().GetServerCount() > 0))
        qstr_id = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_ID(0));
    // -------------------------------------------
    if (!qstr_id.isEmpty())
        str_name = opentxs::OT::App().API().Exec().GetServer_Name(qstr_id.toStdString());
    // -------------------------------------------
    if (str_name.empty() || qstr_id.isEmpty())
        SetFieldsBlank();
    else
    {
        QString qstrName = QString::fromStdString(str_name);
        // ---------------------------
        setField("NotaryID",   qstr_id);
        setField("ServerName", qstrName);
        // ---------------------------
        ui->lineEditID->home(false);
        // ---------------------------
        if (qstr_default_id.isEmpty())
            emit SetDefaultServer(qstr_id, qstrName);
    }
}


void MTPageAcct_Server::on_pushButtonManage_clicked()
{
    MTDetailEdit * pWindow = new MTDetailEdit(this);

    pWindow->setAttribute(Qt::WA_DeleteOnClose);
    // -------------------------------------
    mapIDName & the_map = pWindow->m_map;
    // -------------------------------------
    the_map.clear();
    // -------------------------------------
    QString qstrPreSelected   = field("NotaryID").toString();
    bool    bFoundPreselected = false;
    // -------------------------------------
    int32_t the_count = opentxs::OT::App().API().Exec().GetServerCount();
    bool    bStartingWithNone = (the_count < 1);

    for (int32_t ii = 0; ii < the_count; ii++)
    {
        QString OT_id   = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_ID(ii));
        QString OT_name = QString::fromStdString(opentxs::OT::App().API().Exec().GetServer_Name(OT_id.toStdString()));

        the_map.insert(OT_id, OT_name);

        if (!qstrPreSelected.isEmpty() && (0 == qstrPreSelected.compare(OT_id)))
            bFoundPreselected = true;
    } // for
    // -------------------------------------
    pWindow->setWindowTitle(tr("Manage Servers"));
    // -------------------------------------
    if (bFoundPreselected)
        pWindow->SetPreSelected(qstrPreSelected);
    // -------------------------------------
    pWindow->dialog(MTDetailEdit::DetailEditTypeServer, true);
    // -------------------------------------
    if (bStartingWithNone && (opentxs::OT::App().API().Exec().GetServerCount() > 0))
    {
        std::string str_id = opentxs::OT::App().API().Exec().GetServer_ID(0);

        if (!str_id.empty())
        {
            std::string str_name = opentxs::OT::App().API().Exec().GetServer_Name(str_id);

            if (str_name.empty())
                str_name = str_id;
            // --------------------------------
            setField("NotaryID",   QString::fromStdString(str_id));
            setField("ServerName", QString::fromStdString(str_name));
            // --------------------------------
            ui->lineEditID->home(false);
        }
    }
    // -------------------------------------
    else if (opentxs::OT::App().API().Exec().GetServerCount() < 1)
        SetFieldsBlank();
}

void MTPageAcct_Server::SetFieldsBlank()
{
    setField("NotaryID",   "");
    setField("ServerName", QString("<%1>").arg(tr("Click to choose Server")));
}

MTPageAcct_Server::~MTPageAcct_Server()
{
    delete ui;
}
