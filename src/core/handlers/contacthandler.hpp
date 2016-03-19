#ifndef CONTACTHANDLER_HPP
#define CONTACTHANDLER_HPP

#include <vector>
#include <string>

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"
#include "core/mapidname.hpp"

#include <opentxs/client/OpenTransactions.hpp>
#include <opentxs/client/OTRecordList.hpp>
#include <opentxs/core/Nym.hpp>

#include <core/network/Network.h>

#include <QMutex>
#include <QString>
#include <QVariant>
#include <QMap>


#define DEFAULT_CHEQUE_EXPIRATION 60*60*24*30 // 2592000 seconds == 30 days

namespace opentxs {
    class OTPassword;
    class Nym;
    //class Claim;
}

int claimPolarityToInt(opentxs::OT_API::ClaimPolarity polarity);
opentxs::OT_API::ClaimPolarity intToClaimPolarity(int polarity);


class MTNameLookupQT : public opentxs::OTNameLookup
{
public:
    virtual ~MTNameLookupQT() {}

    virtual std::string GetNymName(const std::string & str_id,
                                   const std::string   p_notary_id) const;

    virtual std::string GetAcctName(const std::string & str_id,
                                    const std::string   p_nym_id,
                                    const std::string   p_notary_id,
                                    const std::string   p_asset_id) const;

    virtual std::string GetAddressName(const std::string & str_address) const; // Used for Bitmessage addresses (etc.)

    // Let's say that OTRecordList just deposited a cheque. (Which it does automatically.)
    // Or let's say the user just asked it to activate a smart contract. Whatever. RecordList
    // will call this and pass the server's "success" transaction contents, along with whatever
    // other useful IDs it's gleaned.
    //
    // That way, when Moneychanger overrides notifyOfSuccessfulNotarization, Moneychanger will
    // get a notification whenever the recordlist has deposited a cheque. Then Moneychanger can
    // take the cheque deposit (transaction reply from server) and add it to its internal database,
    // in its payment table.
    //
    virtual void notifyOfSuccessfulNotarization(const std::string & str_acct_id,
                                                const std::string   p_nym_id,
                                                const std::string   p_notary_id,
                                                const std::string   p_txn_contents,
                                                int64_t lTransactionNum,
                                                int64_t lTransNumForDisplay) const;
};



class MTContactHandler;

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
                            QString notary_id_string=QString(""),
                            QString asset_id_string=QString(""));

  QString GetContactName(int nContactID);
  bool    SetContactName(int nContactID, QString contact_name_string);

  void NotifyOfNymNamePair(QString nym_id_string, QString name_string);
  void NotifyOfNymServerPair(QString nym_id_string, QString notary_id_string);
  void NotifyOfNymServerUnpair(QString nym_id_string, QString notary_id_string);

  int  CreateContactBasedOnNym(QString nym_id_string, QString notary_id_string=QString(""), QString payment_code=QString(""));
  int  CreateContactBasedOnAddress(QString qstrAddress, QString qstrMethodType);

  int  CreateSmartContractTemplate(QString template_string);

  int CreateManagedPassphrase(const QString & qstrTitle, const QString & qstrUsername, const opentxs::OTPassword & thePassphrase,
                              const QString & qstrURL,   const QString & qstrNotes);

  bool UpdateManagedPassphrase(int nPassphraseID,
                               const QString & qstrTitle, const QString & qstrUsername, const opentxs::OTPassword & thePassphrase,
                               const QString & qstrURL,   const QString & qstrNotes);

  bool GetManagedPassphrase(int nPassphraseID,
                            QString & qstrTitle, QString & qstrUsername, opentxs::OTPassword & thePassphrase,
                            QString & qstrURL,   QString & qstrNotes);

  bool GetManagedPassphrases(mapIDName & mapTitle, mapIDName & mapURL, QString searchStr="");

protected:
  bool LowLevelUpdateManagedPassphrase(int nPassphraseID,
                                       const QString & qstrTitle, const QString & qstrUsername, const opentxs::OTPassword & thePassphrase,
                                       const QString & qstrURL,   const QString & qstrNotes);

public:
  bool AddNymToExistingContact   (int nContactID, QString nym_id_string, QString payment_code="");
  bool VerifyNymOnExistingContact(int nContactID, QString nym_id_string); // See if a given Contact ID is associated with a given NymID.

  bool ContactExists(int nContactID);
  bool DeleteContact(int nContactID);

  bool ArchivedTradeReceiptExists(int64_t lReceiptID);
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
  bool GetMsgMethodTypesByContact(mapIDName & theMap, int nFilterByContact, bool bAddServers=false, QString filterByType="", bool bIncludeTypeInKey=true); // Method Types.
  bool GetMsgMethodTypesByNym    (mapIDName & theMap, QString filterByNym,  bool bAddServers=false);

  bool GetAddressesByContact     (mapIDName & theMap, int nFilterByContact, QString filterByType, bool bIncludeTypeInKey=true);
  bool GetAddressesByNym         (mapIDName & theMap, QString filterByNym,  QString filterByType);
  bool GetAddressesByNym         (mapIDName & theMap, QString filterByNym,  int filterByMethodID);

  bool GetMethodsAndAddrByNym    (mapIDName & theMap, QString filterByNym,  int filterByMethodID);
  bool GetMethodsAndAddrByNym    (mapIDName & theMap, QString filterByNym);

  QString GetNymByAddress    (QString qstrAddress); // Note: NOT to be confused with Payment Address! This refers to P2P addresses like Bitmessage.
  int     GetContactByAddress(QString qstrAddress);

  int  GetMethodIDByNymAndAddress(QString filterByNym, QString qstrAddress);

  bool GetServers (mapIDName & theMap,                       bool bPrependOTType=false);
  bool GetServers (mapIDName & theMap, QString filterByNym,  bool bPrependOTType=false);
  bool GetServers (mapIDName & theMap, int nFilterByContact, bool bPrependOTType=false);

  bool GetAccounts(mapIDName & theMap, QString filterByNym, QString filterByServer, QString filterByAsset);

  bool GetContacts(mapIDName & theMap);
  bool GetNyms    (mapIDName & theMap, int nFilterByContact);
  bool GetPaymentCodes(mapIDName & theMap, int nFilterByContact);

  bool GetSmartContracts(mapIDName & theMap);
  QString GetSmartContract    (int nID);
  bool DeleteSmartContract    (int nID);
  bool DeleteManagedPassphrase(int nID);


  bool LowLevelUpdateMessageBody(int nMessageID, const QString & qstrBody);
  bool CreateMessageBody(QString qstrBody);
  bool DeleteMessageBody(int nID);
  bool UpdateMessageBody(int nMessageID, const QString & qstrBody);
  QString GetMessageBody(int nID);

  int  GetPaymentIdByTxnDisplayId(int64_t lTxnDisplayId, QString qstrNymId);
  bool LowLevelUpdatePaymentBody(int nPaymentID, const QString qstrBody, const QString qstrPendingBody);
  bool CreatePaymentBody(QString qstrBody, QString qstrPendingBody);
  bool DeletePaymentBody(int nID);
  bool UpdatePaymentBody(int nPaymentID, const QString qstrBody, const QString qstrPendingBody);
  QString GetPaymentBody(int nID);
  QString GetPaymentPendingBody(int nID);

  bool UpdatePaymentRecord(int nPaymentID, QMap<QString, QVariant>& mapFinalValues);
  bool SetPaymentFlags(int nPaymentID, qint64 nFlags);
  // ----------------------------------------------------------
  int  GetOrCreateLiveAgreementId(const int64_t transNumDisplay, const QString & notaryID, const QString & qstrEncodedMemo, const int nFolder, int & lastKnownState); // returns nAgreementId
  bool UpdateLiveAgreementRecord(const int nAgreementId, const int64_t nNewestReceiptNum, const int nNewestKnownState, const int64_t timestamp, const QString qstrEncodedMemo);
  // ------------------------------
  // Why do we apparently have 2 receipt IDs? (ReceiptNum and AgreementReceiptKey)
  // The former comes from OT itself, and though it MAY be unique to OT (presuming the server is honest)
  // it's not unique across notaries in any case. But it's the number we'll need when dealing with OT.
  // Whereas the latter is an autonumber created here locally, in Moneychanger, which ensures that it's
  // unique to the local Moneychanger DB. So they are just used in slightly different ways and we ended up
  // needing both of them.
  int  DoesAgreementReceiptAlreadyExist(const int nAgreementId, const int64_t receiptNum, const QString & qstrNymId); // returns nAgreementReceiptKey
  // ------------------------------
  bool CreateAgreementReceiptBody(const int nAgreementReceiptKey, QString & qstrReceiptBody); // When this is called, we already know the specific receipt is being added for the first time.
  bool DeleteAgreementReceiptBody(const int nID); // nID is nAgreementReceiptKey
  QString GetAgreementReceiptBody(const int nID); // nID is nAgreementReceiptKey

  bool LowLevelUpdateReceiptBody(int nAgreementReceiptKey, const QString & qstrBody);
  // ----------------------------------------------------------
  // Claims and Claim Verifications (web of trust.)
  //
  bool claimRecordExists(const QString & claim_id);

  QString getBitmessageAddressFromClaims(const QString & claimant_nym_id);
  QString getNymIdFromClaimsByBtMsg(const QString & bitmessage_address);
  QString getDisplayNameFromClaims(const QString & claimant_nym_id);
  // ----------------------------------------------------------
  // Pass in a claimId, and see if there are any verifications for that claim
  // by verifier_nym_id. If there are, return the polarity.
  bool getPolarityIfAny(const QString & claim_id, const QString & verifier_nym_id, bool & bPolarity);

  // The bool return value here means, "FYI, I changed something based on this call" if true.
  // Otherwise it means, "FYI, I didn't need to change anything based on this call."
  //
  bool claimVerificationConfirm  (const QString & qstrClaimId, const QString & qstrClaimantNymId, const QString & qstrVerifierNymId);
  bool claimVerificationRefute   (const QString & qstrClaimId, const QString & qstrClaimantNymId, const QString & qstrVerifierNymId);
  bool claimVerificationNoComment(const QString & qstrClaimId, const QString & qstrClaimantNymId, const QString & qstrVerifierNymId);

//  bool notifyClaimConfirm(const QString & qstrClaimId, const QString & qstrVerifierNymID);
//  bool notifyClaimRefute(const QString & qstrClaimId, const QString & qstrVerifierNymID);
//  bool notifyClaimNoComment(const QString & qstrClaimId, const QString & qstrVerifierNymID);
  // ----------------------------------------------------------
  bool upsertClaim(const opentxs::Nym& nym, const opentxs::Claim& claim);

  bool upsertClaimVerification(const std::string & claimant_nym_id,
                               const std::string & verifier_nym_id,
                               const opentxs::OT_API::Verification & verification,
                               const bool bIsInternal=true);

  void clearClaimsForNym(const QString & qstrNymId);
protected:
  bool claimVerificationLowlevel(const QString & qstrClaimId, const QString & qstrClaimantNymId,
                                 const QString & qstrVerifierNymId, opentxs::OT_API::ClaimPolarity claimPolarity);

  // ----------------------------------------------------------
  public:
    ~MTContactHandler();
};

#endif // CONTACTHANDLER_HPP
