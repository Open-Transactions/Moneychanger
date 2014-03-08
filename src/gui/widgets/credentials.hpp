#ifndef CREDENTIALS_HPP
#define CREDENTIALS_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWidget>
#include <QSqlRecord>
#include <QString>
#include <QStringList>

#include <nmcrpc/NamecoinInterface.hpp>

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

    /* Functor class to replace the lambda expression used when iterating over
       the query results for names.  */
    /* FIXME: Replace by lambda expression when we can be C++11-only.  */
    class NameStatusFunctor
    {

    private:

        nmcrpc::NamecoinInterface& nc;
        QString& res;
        bool& found;
        const std::string& nym;
        const std::string& cred;

        // No default constructor.
        NameStatusFunctor ();

    public:

        inline
        NameStatusFunctor (nmcrpc::NamecoinInterface& n, QString& r, bool& f,
                           const std::string& ny, const std::string& cr)
          : nc(n), res(r), found(f), nym(ny), cred(cr)
        {}

        void operator() (const QSqlRecord& rec);

    };

};

#endif // CREDENTIALS_HPP
