#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include <QTreeWidgetItem>
#include <QWidget>
#include <QString>
#include <QStringList>

namespace Ui {
class MTCredentials;
}

class MTDetailEdit;

class MTCredentials : public QWidget
{
    Q_OBJECT

public:
    explicit MTCredentials(QWidget *parent, MTDetailEdit & theOwner);
    ~MTCredentials();

    void ClearContents();

    void refresh(QStringList & qstrlistNymIDs);

private slots:
    void on_treeWidget_itemSelectionChanged();

private:
    QStringList   m_NymIDs;

    MTDetailEdit * m_pOwner;

    Ui::MTCredentials *ui;

    /**
     * For a given Nym ID and credential ID, find the Namecoin status text
     * to display for it.
     * @param nym Nym ID.
     * @param cred Master credential hash.
     * @return The string to display as status text.
     */
    QString getNamecoinStatus (const std::string& nym, const std::string& cred);

};

#endif // CREDENTIALS_H
