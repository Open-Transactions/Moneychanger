#ifndef DETAILEDIT_H
#define DETAILEDIT_H

#include <QPointer>
#include <QMap>
#include <QMultiMap>
#include <QVariant>
#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>

#include "Handlers/contacthandler.h"

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
        DetailEditTypeNym,
        DetailEditTypeServer,
        DetailEditTypeAsset,
        DetailEditTypeAccount,
        DetailEditTypeMarket,
        DetailEditTypeOffer,
        DetailEditTypeAgreement,
        DetailEditTypeCorporation
    };

    explicit MTDetailEdit(QWidget *parent);
    ~MTDetailEdit();

    void SetPreSelected(QString strSelected);

    void EnableAdd   (bool bEnabled) { m_bEnableAdd    = bEnabled; }
    void EnableDelete(bool bEnabled) { m_bEnableDelete = bEnabled; }

    void    SetMarketMap(QMultiMap<QString, QVariant> & theMap);
    void    SetMarketNymID(QString qstrNymID) { m_qstrMarketNymID = qstrNymID; }
    QString GetMarketNymID() { return m_qstrMarketNymID; }
    // --------------------------------
    // Use for modeless or modal dialogs.
    void dialog(DetailEditType theType, bool bIsModal=false);

    // Use for widget that appears on a parent dialog.
    void show_widget(DetailEditType theType);
    // --------------------------------
    void FirstRun(MTDetailEdit::DetailEditType theType); // This only does something the first time you run it.
    // --------------------------------
    void RefreshRecords();
    // --------------------------------
    QMultiMap<QString, QVariant> * m_pmapMarkets; // do not delete. For reference only.
    // --------------------------------
    int         m_nCurrentRow;
    QString     m_qstrCurrentID;
    QString     m_qstrCurrentName;
    mapIDName   m_map; // qstr/qstr for id/name

signals:
    void balancesChanged();

public slots:
    void onBalancesChangedFromAbove();
    void onBalancesChangedFromBelow(QString qstrAcctID);

    void onRefreshRecords();

protected:
    // --------------------------------
    QString     m_qstrMarketNymID; // used by marketdetails and offerdetails.
    // ----------------------------------
    bool        m_bEnableAdd;
    bool        m_bEnableDelete;
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

    void on_addButton_clicked();
    void on_deleteButton_clicked();

private:
    DetailEditType m_Type;

    Ui::MTDetailEdit *ui;
};

#endif // DETAILEDIT_H
