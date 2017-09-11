#ifndef DETAILEDIT_HPP
#define DETAILEDIT_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"


#include <QPointer>
#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>

#include "core/mapidname.hpp"


namespace Ui {
class MTDetailEdit;
}

class MTEditDetails;
class Moneychanger;
class DlgMarkets;

class MTDetailEdit : public QWidget
{
    Q_OBJECT

    bool m_bFirstRun;

public:
    enum DetailEditType {
        DetailEditTypeError,
        DetailEditTypeContact,
        DetailEditTypeOpentxsContact,
        DetailEditTypeNym,
        DetailEditTypeServer,
        DetailEditTypeAsset,
        DetailEditTypeAccount,
        DetailEditTypeMarket,
        DetailEditTypeOffer,
        DetailEditTypeAgreement,
        DetailEditTypeCorporation,
        DetailEditTypeTransport
    };

    explicit MTDetailEdit(QWidget *parent);
    ~MTDetailEdit();

    void SetPreSelected(QString strSelected);

    void EnableAdd   (bool bEnabled) { m_bEnableAdd    = bEnabled; }
    void EnableDelete(bool bEnabled) { m_bEnableDelete = bEnabled; }
    // --------------------------------
    void    SetMarketMap(QMultiMap<QString, QVariant> & theMap);
    void    SetOfferMap(QMap<QString, QVariant> & theMap);
    // --------------------------------
    void    SetMarketNymID(QString qstrNymID) { m_qstrMarketNymID = qstrNymID; }
    QString GetMarketNymID() const { return m_qstrMarketNymID; }
    // --------------------------------
    // NOTE:    Used by marketdetails and offerdetails.
    // WARNING: THIS MAY CONTAIN "all" instead of a server ID!
    //
    void    SetMarketNotaryID(QString qstrNotaryID) { m_qstrMarketNotaryID = qstrNotaryID; }
    QString GetMarketNotaryID() const { return m_qstrMarketNotaryID; }
    // --------------------------------
    void    SetMarketID(QString qstrMarketID) { m_qstrMarketID = qstrMarketID; }
    QString GetMarketID() const { return m_qstrMarketID; }
    // --------------------------------
    void    SetLawyerID(QString qstrLawyerID) { m_qstrLawyerID = qstrLawyerID; }
    QString GetLawyerID() const { return m_qstrLawyerID; }
    // --------------------------------
    // Use for modeless or modal dialogs.
    void dialog(DetailEditType theType, bool bIsModal=false);

    // Use for widget that appears on a parent dialog.
    void show_widget(DetailEditType theType);
    // --------------------------------
    void FirstRun(MTDetailEdit::DetailEditType theType); // This only does something the first time you run it.
    // --------------------------------
    void RefreshRecords();
    void ClearRecords();
    void ClearContents();
    // --------------------------------
    void RefreshMarketCombo();
    void SetCurrentMarketIDBasedOnIndex(int index);
    // --------------------------------
    void RefreshLawyerCombo();
    void SetCurrentLawyerIDBasedOnIndex(int index);
    // --------------------------------
    bool getAccountIDs(QString & qstrAssetAcctID, QString & qstrCurrencyAcctID); // For a market offer.
    // --------------------------------
    QWidget * GetTab(int nTab);
    // --------------------------------
    QMultiMap<QString, QVariant> * m_pmapMarkets=nullptr; // do not delete. For reference only.
    QMap     <QString, QVariant> * m_pmapOffers=nullptr;  // do not delete. For reference only.
    // --------------------------------
    int         m_nCurrentRow = -1;
    QString     m_qstrCurrentID;
    QString     m_qstrCurrentName;
    mapIDName   m_map; // qstr/qstr for id/name
    // ----------------------------------
    // Only used in DetailEdit for Offer Details. m_mapMarkets is a Map that uniquely identifies each marketID/scale with a Name.
    // (It's a copy of m_map from the MarketDetails DetailEdit.)
    // Whereas m_pOwner->m_pmapMarkets is a MultiMap that contains multiple entries with the same id/scale (each for a different server.)
    // (It's a pointer to the DlgMarkets m_mapMarkets multimap.)
    // The first is used to populate the combo box, whereas the second is used to loop through actual MarketData pointers.
    //
    mapIDName m_mapMarkets;
    QString   m_qstrMarketID;
    // ----------------------------------
    // Only used in DetailEdit for Smart Contracts.
    //
    mapIDName m_mapLawyers;
    QString   m_qstrLawyerID;
    // ----------------------------------
    void SetType(DetailEditType theType) { m_Type = theType; }

signals:
    void balancesChanged();
    void CurrentMarketChanged(QString qstrMarketID);
    void NeedToLoadOrRetrieveOffers(QString qstrMarketID);

public slots:
    void onBalancesChangedFromAbove();
    void onBalancesChangedFromBelow(QString qstrAcctID);
    void onRefreshRecords();
    void onExpertModeUpdated(bool bExpertMode);
    void onMarketIDChangedFromAbove(QString qstrMarketID);
    void onSetNeedToRetrieveOfferTradeFlags();

protected:
    // --------------------------------
    QString     m_qstrMarketNymID;    // used by marketdetails and offerdetails.
    QString     m_qstrMarketNotaryID; // used by marketdetails and offerdetails.
    // ----------------------------------
    bool        m_bEnableAdd=false;
    bool        m_bEnableDelete=false;
    // ----------------------------------
    QString     m_PreSelected;
    // ----------------------------------
    QPointer<MTEditDetails> m_pDetailPane;
    QPointer<QVBoxLayout>   m_pDetailLayout;
    // ----------------------------------
    QPointer<QTabWidget>    m_pTabWidget;
    // ----------------------------------
    bool eventFilter(QObject *obj, QEvent *event);

    virtual void showEvent(QShowEvent * event);

private slots:
    void on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_comboBox_currentIndexChanged(int index);

    void on_addButton_clicked();
    void on_deleteButton_clicked();

private:
    DetailEditType m_Type;

    Ui::MTDetailEdit *ui;
};

#endif // DETAILEDIT_HPP
