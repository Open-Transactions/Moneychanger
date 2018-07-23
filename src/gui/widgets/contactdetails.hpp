#ifndef CONTACTDETAILS_HPP
#define CONTACTDETAILS_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <gui/widgets/editdetails.hpp>

#include <QGroupBox>

namespace Ui {
class MTContactDetails;
}

class QPlainTextEdit;
class QTreeWidget;
class QTreeView;
class MTCredentials;
class QStringList;
class QMenu;
class QTreeWidgetItem;

class ClaimsProxyModel;
class ModelClaims;

class MTContactDetails : public MTEditDetails
{
    Q_OBJECT

public:
    explicit MTContactDetails(QWidget *parent, MTDetailEdit & theOwner);
    virtual ~MTContactDetails();

    virtual void refresh(QString strID, QString strName);
    virtual void AddButtonClicked();
    virtual void DeleteButtonClicked();

    // ----------------------------------
    // Only used on construction (aka when dialog() is called for first time.)
    //
    virtual int       GetCustomTabCount();
    virtual QWidget * CreateCustomTab (int nTab);
    virtual QString   GetCustomTabName(int nTab);
    // ----------------------------------
    virtual void ClearContents();

    void ClearTree();
    void RefreshTree(int nContactId, QStringList & qstrlistNymIDs);

signals:
    void nymWasJustChecked(QString);

public slots:
    void onClaimsUpdatedForNym(QString nymId);
    void onClaimsUpdatedTimer();

protected:
    QGroupBox * createAddressGroupBox    (QString strContactID);
    QWidget   * createSingleAddressWidget(int nContactID, QString qstrType, QString qstrTypeDisplay, QString qstrAddress);
    QWidget   * createNewAddressWidget   (int nContactID);

private:
    QPointer<QPlainTextEdit> m_pPlainTextEdit;
    QPointer<QPlainTextEdit> m_pPlainTextEditNotes;

    QPointer<QTreeWidget>    treeWidgetClaims_;    

    QPointer<ModelClaims>      pModelClaims_;
    QPointer<ClaimsProxyModel> pProxyModelClaims_;

    QPointer<MTCredentials> m_pCredentials;

    QScopedPointer<QMenu> popupMenuProfile_; // For the tree showing the Nym's profile claims/verifications.

    QAction * pActionConfirm_ = nullptr;
    QAction * pActionRefute_ = nullptr;
    QAction * pActionNoComment_ = nullptr;

    QAction * pActionNewRelationship_ = nullptr;
    QAction * pActionDeleteRelationship_ = nullptr;

    QTreeWidgetItem * metInPerson_ = nullptr;

private slots:
    void on_lineEditName_editingFinished();

    void on_btnAddressAdd_clicked();
    void on_btnAddressDelete_clicked();
    void on_pushButtonMsg_clicked();
    void on_pushButtonPay_clicked();
    void on_pushButtonRefresh_clicked();

    void on_treeWidget_customContextMenuRequested(const QPoint &pos);

private:
    QPointer<QWidget>   m_pHeaderWidget;
//    QPointer<QGroupBox> m_pAddresses;

    bool bTimerFired_=false;

    Ui::MTContactDetails *ui;
};

#endif // CONTACTDETAILS_HPP
