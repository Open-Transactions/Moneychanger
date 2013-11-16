#ifndef DETAILEDIT_H
#define DETAILEDIT_H

#include <QPointer>
#include <QMap>
#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>

#include "Handlers/contacthandler.h"

namespace Ui {
class MTDetailEdit;
}

namespace OTDB {
class MarketData;
}

class MTEditDetails;
class Moneychanger;

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

    explicit MTDetailEdit(QWidget *parent, Moneychanger & theMC);
    ~MTDetailEdit();

    void SetPreSelected(QString strSelected);

    void EnableAdd   (bool bEnabled) { m_bEnableAdd    = bEnabled; }
    void EnableDelete(bool bEnabled) { m_bEnableDelete = bEnabled; }

    void SetMarketMap(QMap<QString, OTDB::MarketData *> & theMap) { m_pmapMarkets = &theMap; }
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
    int         m_nCurrentRow;
    QString     m_qstrCurrentID;
    QString     m_qstrCurrentName;
    mapIDName   m_map; // qstr/qstr for id/name

signals:
    void balancesChanged();

public slots:
    void onBalancesChangedFromAbove();
    void onBalancesChangedFromBelow(QString qstrAcctID);

protected:
    QMap<QString, OTDB::MarketData *> * m_pmapMarkets; // do not delete. For reference only.
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

public:
    Moneychanger * GetMoneychanger();

private:
    QPointer<Moneychanger> m_pMoneychanger;

    DetailEditType m_Type;

    Ui::MTDetailEdit *ui;
};

#endif // DETAILEDIT_H
