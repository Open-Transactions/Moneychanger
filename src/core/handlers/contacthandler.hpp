#ifndef CONTACTHANDLER_HPP
#define CONTACTHANDLER_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <core/MTRecordList.hpp>

#include <QMutex>
#include <QString>
#include <QMap>


#define DEFAULT_CHEQUE_EXPIRATION 60*60*24*30 // 2592000 seconds == 30 days


class MTNameLookupQT : public MTNameLookup
{
public:
    virtual ~MTNameLookupQT() {}

    virtual std::string GetNymName(const std::string & str_id,
                                   const std::string * p_server_id=NULL) const;

    virtual std::string GetAcctName(const std::string & str_id,
                                    const std::string * p_nym_id=NULL,
                                    const std::string * p_server_id=NULL,
                                    const std::string * p_asset_id=NULL) const;
};



class MTContactHandler;

typedef QMap<QString, QString> mapIDName; // ID, display name.

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
  int FindContactIDByAcctID(QString acct_id_string,
                            QString nym_id_string=QString(""),
                            QString server_id_string=QString(""),
                            QString asset_id_string=QString(""));

  QString GetContactName(int nContactID);
  bool    SetContactName(int nContactID, QString contact_name_string);

  void NotifyOfNymServerPair(QString nym_id_string, QString server_id_string);

  int  CreateContactBasedOnNym(QString nym_id_string, QString server_id_string=QString(""));

  bool AddNymToExistingContact(int nContactID, QString nym_id_string);

  bool ContactExists(int nContactID);
  bool DeleteContact(int nContactID);

  bool GetServers(mapIDName & theMap, QString filterByNym);
  bool GetAccounts(mapIDName & theMap, QString filterByNym, QString filterByServer, QString filterByAsset);

  bool GetContacts(mapIDName & theMap);
  bool GetNyms(mapIDName & theMap, int nFilterByContact);

  public:
    ~MTContactHandler();
};

#endif // CONTACTHANDLER_HPP
