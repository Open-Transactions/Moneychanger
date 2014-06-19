#ifndef CONTACTHANDLER_HPP
#define CONTACTHANDLER_HPP

#include <vector>
#include <string>

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <opentxs/OTRecordList.hpp>

#include <core/network/Network.h>

#include <QMutex>
#include <QString>
#include <QMap>


#define DEFAULT_CHEQUE_EXPIRATION 60*60*24*30 // 2592000 seconds == 30 days


class MTNameLookupQT : public OTNameLookup
{
public:
    virtual ~MTNameLookupQT() {}

    virtual std::string GetNymName(const std::string & str_id,
                                   const std::string * p_server_id=NULL) const;

    virtual std::string GetAcctName(const std::string & str_id,
                                    const std::string * p_nym_id=NULL,
                                    const std::string * p_server_id=NULL,
                                    const std::string * p_asset_id=NULL) const;

    virtual std::string GetAddressName(const std::string & str_address) const; // Used for Bitmessage addresses (etc.)
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

  static const std::string s_key_id;

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

  bool AddNymToExistingContact   (int nContactID, QString nym_id_string);
  bool VerifyNymOnExistingContact(int nContactID, QString nym_id_string); // See if a given Contact ID is associated with a given NymID.

  bool ContactExists(int nContactID);
  bool DeleteContact(int nContactID);
  // ---------------------------------------------
  static QString Encode(QString plaintext);
  static QString Decode(QString encoded);
  // ---------------------------------------------
  static QString Encrypt(QString plaintext);
  static QString Decrypt(QString ciphertext);
  // ---------------------------------------------
  QString GetValueByIDLowLevel         (QString str_select);
  QString GetEncryptedValueByIDLowLevel(QString str_select);

  QString GetValueByID(QString qstrID,                 QString column, QString table, QString id_name);
  bool    SetValueByID(QString qstrID, QString value,  QString column, QString table, QString id_name);
  QString GetValueByID(int     nID,                    QString column, QString table, QString id_name);
  bool    SetValueByID(int     nID,    QString value,  QString column, QString table, QString id_name);

  QString GetEncryptedValueByID(QString qstrID,                 QString column, QString table, QString id_name);
  bool    SetEncryptedValueByID(QString qstrID, QString value,  QString column, QString table, QString id_name);
  QString GetEncryptedValueByID(int     nID,                    QString column, QString table, QString id_name);
  bool    SetEncryptedValueByID(int     nID,    QString value,  QString column, QString table, QString id_name);

  /*
  QString create_msg_method = "CREATE TABLE msg_method"
          " (method_id INTEGER PRIMARY KEY,"   // 1, 2, etc.
          "  method_display_name TEXT,"        // "Localhost"
          "  method_type TEXT,"                // "bitmessage"
          "  method_type_display TEXT,"        // "Bitmessage"
          "  method_connect TEXT)";            // "http://username:password@http://127.0.0.1:8332/"
  */
  // --------------------------------------------
  int  AddMsgMethod(QString display_name, QString type, QString type_display, QString connect);
  bool DeleteMsgMethod(int nMethodID);

  bool MethodExists               (int     nMethodID);
  bool MethodTypeExists           (QString method_type);
  bool MethodTypeFoundOnNym       (QString method_type, QString filterByNym);
  bool MethodTypeFoundOnContact   (QString method_type, int nFilterByContact);
  bool MethodTypeFoundOnAnyNym    (QString method_type);
  bool MethodTypeFoundOnAnyContact(QString method_type);

//QString create_nym_method
// = "CREATE TABLE nym_method(nym_id TEXT, method_id INTEGER, address TEXT, PRIMARY KEY(nym_id, method_id, address))";
//QString create_contact_method
// = "CREATE TABLE contact_method(contact_id INTEGER, method_type TEXT, address TEXT, PRIMARY KEY(contact_id, method_type, address))";

  bool AddMsgAddressToNym    (QString nym_id, int     nMethodID,      QString address);
  bool AddMsgAddressToContact(int nContactID, QString qstrMethodType, QString address);

  bool RemoveMsgAddressFromNym    (QString nym_id, int     nMethodID,      QString address);
  bool RemoveMsgAddressFromContact(int nContactID, QString qstrMethodType, QString address);
  // --------------------------------------------
  QString GetMethodDisplayName(int nMethodID);
  bool    SetMethodDisplayName(int nMethodID, QString input);

  QString GetMethodType       (int nMethodID);
  QString GetMethodType       (QString qstrAddress); // Get the method type based on the address.
  bool    SetMethodType       (int nMethodID, QString input);

  QString GetMethodTypeDisplay(int nMethodID);
  bool    SetMethodTypeDisplay(int nMethodID, QString input);

  QString GetMethodConnectStr (int nMethodID);
  bool    SetMethodConnectStr (int nMethodID, QString input);
  // --------------------------------------------
  // A messaging method has a type (like "bitmessage") and a bitmessage connect string,
  // such as "http://user:password@127.0.0.1:8332/", and a display name, like "Localhost".
  // There should also be a type display name, such as "Bitmessage."
  //
  // If it was an OT server, the type would be "otserver" and the connect string would be
  // a Server ID, such as "r1fUoHwJOWCuK3WBAAySjmKYqsG6G2TYIxdqY6YNuuG", and the display
  // name would be: "Transactions.com" (from the server contract.)
  // The type display name would be "OT Server"
  //
  // To pass this via mapIDName, for an OT server, the ID would be:
  // "otserver|r1fUoHwJOWCuK3WBAAySjmKYqsG6G2TYIxdqY6YNuuG"
  // Display name:  "OT Server: Transactions.com"
  //
  // Whereas for a Bitmessage address associated with one of my Nyms, the ID would be:
  // "bitmessage|METHOD_ID"
  // Display name:  "Bitmessage: Localhost"
  //
  // Whereas for a Bitmessage address associated with one of my Contacts, the ID would be:
  // "bitmessage|BITMESSAGE_ID"
  // Display name:  "Bitmessage: Localhost"
  //
  // Need a Method ID, like Contact ID.
  // Method should have: method ID, type string, connect string, display name, type display name.
  //
  // There should also be a table, associating message methods with nyms, and another table,
  // associating methods with contacts.

  // GetMsgMethods will return things like Bitmessage, Redis, ZMQ publisher, etc.
  // If bAddServers is set to true, then it will include the OT servers on that
  // list.
  //
  bool GetMsgMethods             (mapIDName & theMap,                       bool bAddServers=false, QString filterByType="");
  bool GetMsgMethodTypes         (mapIDName & theMap,                       bool bAddServers=false);
  bool GetMsgMethodsByNym        (mapIDName & theMap, QString filterByNym,  bool bAddServers=false, QString filterByType=""); // Methods.
  bool GetMsgMethodTypesByContact(mapIDName & theMap, int nFilterByContact, bool bAddServers=false, QString filterByType=""); // Method Types.
  bool GetMsgMethodTypesByNym    (mapIDName & theMap, QString filterByNym,  bool bAddServers=false);

  bool GetAddressesByContact     (mapIDName & theMap, int nFilterByContact, QString filterByType);
  bool GetAddressesByNym         (mapIDName & theMap, QString filterByNym,  QString filterByType);
  bool GetAddressesByNym         (mapIDName & theMap, QString filterByNym,  int filterByMethodID);

  bool GetMethodsAndAddrByNym    (mapIDName & theMap, QString filterByNym,  int filterByMethodID);
  bool GetMethodsAndAddrByNym    (mapIDName & theMap, QString filterByNym);

  QString GetNymByAddress    (QString qstrAddress);
  int     GetContactByAddress(QString qstrAddress);

  int  GetMethodIDByNymAndAddress(QString filterByNym, QString qstrAddress);

  bool GetServers (mapIDName & theMap,                       bool bPrependOTType=false);
  bool GetServers (mapIDName & theMap, QString filterByNym,  bool bPrependOTType=false);
  bool GetServers (mapIDName & theMap, int nFilterByContact, bool bPrependOTType=false);

  bool GetAccounts(mapIDName & theMap, QString filterByNym, QString filterByServer, QString filterByAsset);

  bool GetContacts(mapIDName & theMap);
  bool GetNyms    (mapIDName & theMap, int nFilterByContact);

  public:
    ~MTContactHandler();
};

#endif // CONTACTHANDLER_HPP
