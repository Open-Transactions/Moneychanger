#ifndef DLGENCRYPT_H
#define DLGENCRYPT_H

#include <core/handlers/contacthandler.hpp>

#include <QDialog>

namespace Ui {
class DlgEncrypt;
}

class QListWidgetItem;

class DlgEncrypt : public QDialog
{
    Q_OBJECT

public:
    explicit DlgEncrypt(QWidget *parent = 0);
    ~DlgEncrypt();

    void dialog();

    void SetEncrypt(bool bEncrypt=true);
    void SetSign   (bool bSign   =true);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_checkBoxEncrypt_toggled(bool checked);

    void on_checkBoxSign_toggled(bool checked);

    void on_pushButtonClipboard_clicked();

    void on_pushButtonEncrypt_clicked();

    void on_pushButtonAdd_clicked();

    void on_pushButtonRemove_clicked();

    void on_comboBoxNym_currentIndexChanged(int index);

    void on_listWidgetNotAdded_itemDoubleClicked(QListWidgetItem *item);

    void on_listWidgetAdded_itemDoubleClicked(QListWidgetItem *item);

private:
    void PopulateCombo();
    void PopulateWidgetNotAdded();
    void SetCurrentNymIDBasedOnIndex(int index);

    void setEncryptBtnText();

    Ui::DlgEncrypt *ui;
    bool already_init;

    bool m_bEncrypt;
    bool m_bSign;

    QString   m_nymId;
    mapIDName m_mapNyms;
};

#endif // DLGENCRYPT_H
