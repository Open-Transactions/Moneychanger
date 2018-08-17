#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/credentials.hpp>
#include <ui_credentials.h>
#include <core/moneychanger.hpp>

#include <core/handlers/contacthandler.hpp>
#include <core/handlers/DBHandler.hpp>

#include <QSqlField>

#include <opentxs/opentxs.hpp>

MTCredentials::MTCredentials(QWidget *parent, MTDetailEdit & theOwner) :
    QWidget(parent),
    m_pOwner(&theOwner),
    ui(new Ui::MTCredentials)
{
    ui->setupUi(this);

//    ui->treeWidget->horizontalHeader()->setStretchLastSection(true);
//    ui->treeWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);

//    ui->treeWidget->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignCenter);  // Nym
//    ui->treeWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignCenter);  // Credential
//    ui->treeWidget->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignCenter);  // Sub-credential

    ui->treeWidget->setSelectionMode    (QAbstractItemView::SingleSelection);
    ui->treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
}


void MTCredentials::on_treeWidget_itemSelectionChanged()
{
    QTreeWidgetItem * pItem = ui->treeWidget->currentItem();

    if (NULL == pItem)
    {
        ui->label->setVisible(false);
        ui->plainTextEdit->setVisible(false);
        // --------------------
        return;
    }
    // -----------------------------------------
    ui->label->setVisible(true);
    ui->plainTextEdit->setVisible(true);
    // -----------------------------------------
    bool bIsNym     = false;
    bool bIsCred    = false;
    bool bIsSubcred = false;
    // -----------------------------------------
    QTreeWidgetItem * pParent = pItem->parent();
    QTreeWidgetItem * pHigherParent = NULL;

    if (NULL == pParent)
        bIsNym = true;
    // -----------------------------------------
    else
    {
        pHigherParent = pParent->parent();

        if (NULL == pHigherParent)
            bIsCred = true;
        // -----------------------------------------
        else
            bIsSubcred = true;
    }
    // -----------------------------------------
    if (bIsNym)
    {
        QString qstrNymID = pItem->text(1);
        // --------------------------------
        const std::string str_source = Moneychanger::It()->OT().Exec().GetNym_SourceForID(qstrNymID.toStdString());
        // --------------------------------
        ui->label->setText(tr("Nym Source:"));
        ui->plainTextEdit->setPlainText(QString::fromStdString(str_source));
    }
    // -----------------------------------------
    else if (bIsCred)
    {
        QString qstrNymID  = pParent->text(1);
        QString qstrCredID = pItem  ->text(1);
        // --------------------------------
        const std::string str_contents = Moneychanger::It()->OT().Exec().GetNym_MasterCredentialContents(qstrNymID .toStdString(),
                                                                               qstrCredID.toStdString());
        // --------------------------------
        ui->label->setText(tr("Master Credential Contents:"));
        ui->plainTextEdit->setPlainText(QString::fromStdString(str_contents));
    }
    // -----------------------------------------
    else if (bIsSubcred)
    {
        QString qstrNymID  = pHigherParent->text(1);
        QString qstrCredID = pParent      ->text(1);
        QString qstrSubID  = pItem        ->text(1);
        // --------------------------------
        const std::string str_contents = Moneychanger::It()->OT().Exec().GetNym_ChildCredentialContents(qstrNymID .toStdString(),
                                                                                  qstrCredID.toStdString(),
                                                                                  qstrSubID .toStdString());
        // --------------------------------
        ui->label->setText(tr("Subcredential Contents:"));
        ui->plainTextEdit->setPlainText(QString::fromStdString(str_contents));
    }
    // -----------------------------------------
}

void MTCredentials::ClearContents()
{
    ui->treeWidget->clear();
    // ----------------------------------
    m_NymIDs.clear();
    // ----------------------------------
    ui->label->setVisible(false);
    ui->plainTextEdit->setVisible(false);
}

void MTCredentials::refresh(QStringList & qstrlistNymIDs)
{
    ui->treeWidget->clear();
    // -----------------------------------
    ui->label->setVisible(false);
    ui->plainTextEdit->setVisible(false);
    // -----------------------------------
    if ((NULL != ui) && (qstrlistNymIDs.size() > 0))
    {
        m_NymIDs = qstrlistNymIDs;
        // -----------------------------------
        for (int ii = 0; ii < m_NymIDs.size(); ++ii)
        {
            QString qstrNymID = m_NymIDs.at(ii);
            // ---------------------------------------
            if (qstrNymID.isEmpty()) // should never happen.
                continue;
            // ---------------------------------------
            std::string str_nym_id = qstrNymID.toStdString();
            // ---------------------------------------
            std::string str_nym_name = Moneychanger::It()->OT().Exec().GetNym_Name(qstrNymID.toStdString());
            QString     qstrNymName  = QString::fromStdString(str_nym_name);
            // ---------------------------------------
            // Insert Nym into Tree.
            //
            QTreeWidgetItem * topLevel = new QTreeWidgetItem;
            // ------------------------------------------
            topLevel->setText(0, qstrNymName);
            topLevel->setText(1, qstrNymID);
            // ------------------------------------------
            ui->treeWidget->addTopLevelItem(topLevel);
            ui->treeWidget->expandItem(topLevel);
            // ------------------------------------------
            // Next: any credentials under this Nym?
            //
            auto nym = Moneychanger::It()->OT().Wallet().Nym(opentxs::Identifier::Factory(str_nym_id));
            OT_ASSERT(nym);
            auto masterCredentialIDs = nym->GetMasterCredentialIDs();
            for (auto masterCredentialID : masterCredentialIDs)
            {
                // ---------------------------------------
                QString qstrCredID = QString::fromStdString(masterCredentialID->str());
                // ---------------------------------------
                // Add the credential ID to the tree.
                //
                QTreeWidgetItem * cred_item = new QTreeWidgetItem;
                // ---------------------------------------
                cred_item->setText(0, tr("Master Credential"));
                cred_item->setText(1, qstrCredID);
//                cred_item->setText(2, getNamecoinStatus(str_nym_id, masterCredentialID->str()));
                cred_item->setText(2, tr("Namecoin Deprecated"));
                // ---------------------------------------
                topLevel->addChild(cred_item);
                ui->treeWidget->expandItem(cred_item);
                // ---------------------------------------
                // If you need the credential contents later, you can use this:
                //
                // std::string Moneychanger::It()->OT().Exec().GetNym_CredentialContents(const std::string & NYM_ID, const std::string & CREDENTIAL_ID);
                // ---------------------------------------
                // Next: any subcredentials under this credential?
                //
                auto childCredentialIDs = nym->GetChildCredentialIDs(masterCredentialID->str());
                for (auto childCredentialID : childCredentialIDs)
                {
                    // ---------------------------------------
                    QString qstrSubcredID = QString::fromStdString(childCredentialID->str());
                    // ---------------------------------------
                    // Add the subcredential ID to the tree.
                    //
                    QTreeWidgetItem * sub_cred_item = new QTreeWidgetItem;
                    // ---------------------------------------
                    sub_cred_item->setText(0, tr("Subcredential"));
                    sub_cred_item->setText(1, qstrSubcredID);
                    // ---------------------------------------
                    cred_item->addChild(sub_cred_item);
                    // ---------------------------------------

                    // If you need the subcredential contents later, you can use this:
                    //
                    // std::string GetNym_SubCredentialContents(const std::string & NYM_ID, const std::string & MASTER_CRED_ID, const std::string & SUB_CRED_ID);
                    //
                    // Also useful:
                    // std::string Moneychanger::It()->OT().Exec().AddSubcredential(const std::string & NYM_ID, const std::string & MASTER_CRED_ID, const int32_t & nKeySize);
                    // bool        Moneychanger::It()->OT().Exec().RevokeSubcredential(const std::string & NYM_ID, const std::string & MASTER_CRED_ID, const std::string & SUB_CRED_ID);
                    //
                } // for (subcredentials.)
            } // for (credentials.)

//        QWidget     * wdg       = new QWidget;
//        QCheckBox   * pCheckbox = new QCheckBox(wdg);
//        QHBoxLayout * layout    = new QHBoxLayout(wdg);

//        layout->setContentsMargins(0,0,0,0);
//        layout->addWidget(pCheckbox);
//        layout->setAlignment( Qt::AlignCenter );
//        wdg->setLayout(layout);
//        ui->treeWidget->setCellWidget ( ii, 0, wdg);

//        ui->treeWidget->setCellWidget ( ii, 1, pLabelDenomination );
//        ui->treeWidget->setCellWidget ( ii, 2, pLabelExpires );
//        ui->treeWidget->setCellWidget ( ii, 3, pLabelSeries );
//        ui->treeWidget->setCellWidget ( ii, 4, pLabelTokenID );

//        connect(pCheckbox, SIGNAL(stateChanged(int) ), this, SLOT(checkboxClicked(int)));

        } // for (Nyms)
        // ------------------------------

    }

    // Also useful:
    //
//     int32_t     GetNym_RevokedCredCount     (const std::string & NYM_ID);
//     std::string GetNym_RevokedCredID        (const std::string & NYM_ID, const int32_t & nIndex);
//     std::string GetNym_RevokedCredContents  (const std::string & NYM_ID, const std::string & CREDENTIAL_ID);
//     bool        RevokeSubcredential         (const std::string & NYM_ID, const std::string & MASTER_CRED_ID, const std::string & SUB_CRED_ID);

}

MTCredentials::~MTCredentials()
{
    delete ui;
}

