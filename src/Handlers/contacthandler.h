#ifndef CONTACTHANDLER_H
#define CONTACTHANDLER_H

#include <QMutex>
#include <QString>


class MTContactHandler;


class MTContactHandler
{
private:
  static MTContactHandler * _instance;

protected:
  MTContactHandler();

  QMutex m_Mutex;

public:
  static MTContactHandler * getInstance();

  int FindContactIDByNymID (QString nym_id_string);
  int FindContactIDByAcctID(QString acct_id_string);

  QString GetContactName(int nContactID);
  bool    SetContactName(int nContactID, QString contact_name_string);

  void NotifyOfNymServerPair(QString nym_id_string, QString server_id_string);

  int CreateContactBasedOnNym(QString nym_id_string, QString server_id_string=QString(""));
  int FindContactIDByAcctID(QString acct_id_string,
                            QString nym_id_string=QString(""),
                            QString server_id_string=QString(""),
                            QString asset_id_string=QString(""));

  public:
    ~MTContactHandler();
};

#endif // CONTACTHANDLER_H
