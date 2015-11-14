#ifndef DLGTRADEARCHIVE_HPP
#define DLGTRADEARCHIVE_HPP

#include <core/handlers/contacthandler.hpp>

#include <QDialog>
#include <QMenu>
#include <QScopedPointer>
#include <QPointer>

namespace Ui {
class DlgTradeArchive;
}

class QListWidgetItem;
class QItemSelection;
class QString;

class DlgTradeArchive : public QDialog
{
    Q_OBJECT

public:
    explicit DlgTradeArchive(QWidget *parent = 0);
    ~DlgTradeArchive();

    void dialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void PopulateTableWidget(mapIDName & mapTitle, mapIDName & mapURL);

private slots:
    void on_tableView_customContextMenuRequested(const QPoint &pos);

    void tableViewSelectionChanged(const QItemSelection &, const QItemSelection &);

    void on_pushButton_clicked();

private:
    Ui::DlgTradeArchive *ui;
    bool already_init;

    QScopedPointer<QMenu> popupMenu_;

    QAction * pActionDelete;
    QAction * pActionShowAssetReceipt;
    QAction * pActionShowCurrencyReceipt;
    QAction * pActionShowFinalReceipt;
};

#endif // DLGTRADEARCHIVE_HPP
