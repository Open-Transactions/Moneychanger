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
};

#endif // CREDENTIALS_H
