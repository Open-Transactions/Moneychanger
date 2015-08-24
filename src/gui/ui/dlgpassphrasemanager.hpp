#ifndef DLGPASSPHRASE_MANAGER_H
#define DLGPASSPHRASE_MANAGER_H

#include <core/handlers/contacthandler.hpp>

#include <QDialog>
#include <QMenu>
#include <QScopedPointer>
#include <QTimer>
#include <QPointer>

namespace Ui {
class DlgPassphraseManager;
}

class QListWidgetItem;
class QString;

class DlgPassphraseManager : public QDialog
{
    Q_OBJECT

public:
    explicit DlgPassphraseManager(QWidget *parent = 0);
    ~DlgPassphraseManager();

    void dialog();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    void PopulateTableWidget(mapIDName & mapTitle, mapIDName & mapURL);

    void doSearch(QString qstrInput);

private slots:
    void on_pushButtonAdd_clicked();
    void on_pushButtonDelete_clicked();
    void on_lineEdit_returnPressed();
    void on_pushButtonSearch_clicked();
    void on_tableWidget_customContextMenuRequested(const QPoint &pos);
    void on_tableWidget_itemSelectionChanged();
    void on_pushButtonEdit_clicked();
    void on_tableWidget_cellDoubleClicked(int row, int column);

    void on_lineEdit_textChanged(const QString &arg1);

private:
    Ui::DlgPassphraseManager *ui;
    bool already_init;

    QScopedPointer<QMenu> popupMenu_;

    QAction * pActionCopyUsername;
    QAction * pActionCopyPassword;
    QAction * pActionCopyURL;
};

class ClipboardWrapper : public QObject
{
    Q_OBJECT

public:
    static ClipboardWrapper * It();

    void set(const QString & strNewText);

private slots:
    void onDestroy();
    void clearContents();

private:
    static ClipboardWrapper * s_it; // The singleton instance.

    explicit ClipboardWrapper(QObject * parent = 0); // This class is a singleton. You can't construct it yourself; you have to call It().

    QString m_qstrPreviousSetContents;
    QPointer<QTimer> m_pTimer;
};

#endif // DLGPASSPHRASE_MANAGER_H
