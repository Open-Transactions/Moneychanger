#ifndef DLGCREATEPASSPHRASE_HPP
#define DLGCREATEPASSPHRASE_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <QDialog>

namespace Ui {
class DlgCreatePassphrase;
}

namespace opentxs {
class OTPassword;
}

class DlgCreatePassphrase : public QDialog
{
    Q_OBJECT

public:
    explicit DlgCreatePassphrase(QWidget *parent = 0);
    explicit DlgCreatePassphrase(const QString & qstrTitle, const QString & qstrUsername, const QString & qstrURL, const QString & qstrNotes,
                                 const opentxs::OTPassword & thePassword, QWidget *parent = 0);
    ~DlgCreatePassphrase();

    void InitializeDlg();

    QString m_qstrTitle;
    QString m_qstrUsername;
    opentxs::OTPassword * m_pPassphrase;
    QString m_qstrURL;
    QString m_qstrNotes;

public slots:
    void passphraseChangedSlot(QString text);
    void retypeChangedSlot(QString text);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_checkBox_toggled(bool checked);
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_pushButtonGenerate_clicked();
    void on_toolButtonTitle_clicked();
    void on_toolButtonUsername_clicked();
    void on_toolButtonURL_clicked();
    void on_toolButtonPassphrase_clicked();

private:
    Ui::DlgCreatePassphrase *ui;
};

#endif // DLGCREATEPASSPHRASE_HPP
