

#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/handlers/contacthandler.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/mtcomms.h>
#include <core/moneychanger.hpp>

#include <opentxs/core/OTStorage.hpp>
#include <opentxs/client/OTAPI_Wrap.hpp>
#include <opentxs/client/OTAPI_Exec.hpp>
#include <opentxs/client/OT_API.hpp>
#include <opentxs/client/OTWallet.hpp>
#include <opentxs/core/NumList.hpp>
#include <opentxs/core/Proto.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>

#include <QDebug>
#include <QObject>
#include <QStringList>
#include <QSqlField>
#include <QFlags>

#include <tuple>

void MTNameLookupQT::notifyOfSuccessfulNotarization(const std::string & str_acct_id,
                                                    const std::string   p_nym_id,
                                                    const std::string   p_notary_id,
                                                    const std::string   p_txn_contents,
                                                    int64_t lTransactionNum,
                                                    int64_t lTransNumForDisplay) const
{
    // Add/update record to payments table for whatever
    // transaction just occurred.

    Moneychanger::It()->AddPaymentBasedOnNotification(str_acct_id,
                                                      p_nym_id, p_notary_id,
                                                      p_txn_contents, lTransactionNum, lTransNumForDisplay);
}



std::string MTNameLookupQT::GetNymName(const std::string & str_id,
                                       const std::string   p_notary_id) const
{
    // ------------------------
//    qDebug() << QString("Attempting Name Lookup on: ") << QString(str_id.c_str());

    std::string str_result = this->OTNameLookup::GetNymName(str_id, p_notary_id);

//    qDebug() << QString("Result of Name Lookup: ") << QString(str_result.c_str());

    // ------------------------
    if (str_result.empty())
    {
        int nContactID = MTContactHandler::getInstance()->FindContactIDByNymID(QString::fromStdString(str_id));

        if (nContactID > 0)
        {
            QString contact_name = MTContactHandler::getInstance()->GetContactName(nContactID);

            if (!contact_name.isEmpty())
                str_result = contact_name.toStdString();
            else
            {
                QString qstrName = MTContactHandler::getInstance()->getDisplayNameFromClaims(QString::fromStdString(str_id));

                if (qstrName.isEmpty())
                    qstrName = MTContactHandler::getInstance()->GetValueByID(QString::fromStdString(str_id), "nym_display_name", "nym", "nym_id");

                if (!qstrName.isEmpty())
                {
                    MTContactHandler::getInstance()->SetContactName(nContactID, qstrName);
                    str_result = qstrName.toStdString();
                }
            }
            // -----------------------------------------------
            if (p_notary_id != "")
                MTContactHandler::getInstance()->NotifyOfNymServerPair(QString::fromStdString(str_id),
                                                                       QString::fromStdString(p_notary_id));
        }
        else // No contact found.
        {
            QString qstrName = MTContactHandler::getInstance()->getDisplayNameFromClaims(QString::fromStdString(str_id));

            if (qstrName.isEmpty())
                qstrName = MTContactHandler::getInstance()->GetValueByID(QString::fromStdString(str_id), "nym_display_name", "nym", "nym_id");

            if (!qstrName.isEmpty())
            {
                str_result = qstrName.toStdString();
                // -----------------------------------------------
                if (p_notary_id != "")
                    MTContactHandler::getInstance()->NotifyOfNymServerPair(QString::fromStdString(str_id),
                                                                           QString::fromStdString(p_notary_id));
            }
        }
    }
    // ------------------------
    return str_result;
}

std::string MTNameLookupQT::GetAcctName(const std::string & str_id,
                                        const std::string   p_nym_id,
                                        const std::string   p_notary_id,
                                        const std::string   p_asset_id) const
{
    std::string str_result("");
    // ------------------------
    str_result = this->OTNameLookup::GetAcctName(str_id, p_nym_id, p_notary_id, p_asset_id);
    // ------------------------
    if (str_result.empty())
    {
        int nContactID = MTContactHandler::getInstance()->FindContactIDByAcctID(QString::fromStdString(str_id),
                                                                                QString::fromStdString(p_nym_id),
                                                                                QString::fromStdString(p_notary_id),
                                                                                QString::fromStdString(p_asset_id));
        if (nContactID > 0)
        {
            QString contact_name = MTContactHandler::getInstance()->GetContactName(nContactID);

            if (!contact_name.isEmpty())
                str_result = contact_name.toStdString();
        }
    }
    // ------------------------
    return str_result;
}


//virtual
std::string MTNameLookupQT::GetAddressName(const std::string & str_address) const // Used for Bitmessage addresses (etc.)
{
    std::string str_result("");
    // ------------------------
    if (!str_address.empty())
    {
        int nContactID = MTContactHandler::getInstance()->GetContactByAddress(QString::fromStdString(str_address));

        if (nContactID > 0)
        {
            QString contact_name = MTContactHandler::getInstance()->GetContactName(nContactID);

            if (!contact_name.isEmpty())
                str_result = contact_name.toStdString();
            else
            {
                QString qstrNymId = MTContactHandler::getInstance()->GetNymByAddress(QString::fromStdString(str_address));

                if (qstrNymId.isEmpty())
                    qstrNymId = MTContactHandler::getInstance()->getNymIdFromClaimsByBtMsg(QString::fromStdString(str_address));

                if (!qstrNymId.isEmpty())
                    str_result = this->GetNymName(qstrNymId.toStdString(), "");
            }
        }
        else
        {
            QString qstrNymId = MTContactHandler::getInstance()->GetNymByAddress(QString::fromStdString(str_address));

            if (qstrNymId.isEmpty())
                qstrNymId = MTContactHandler::getInstance()->getNymIdFromClaimsByBtMsg(QString::fromStdString(str_address));

            if (!qstrNymId.isEmpty())
                str_result = this->GetNymName(qstrNymId.toStdString(), "");
        }
    }
    // ------------------------
    return str_result;
}


//static
MTContactHandler * MTContactHandler::_instance = NULL;

//static
const std::string MTContactHandler::s_key_id("mc_sqlite");


//protected
MTContactHandler::MTContactHandler() : m_Mutex(QMutex::Recursive) // allows us to call it multiple times from same thread.
{
}


//static
MTContactHandler * MTContactHandler::getInstance()
{
    if (NULL == _instance)
    {
        _instance = new MTContactHandler;
    }
    return _instance;
}


bool MTContactHandler::claimRecordExists(const QString & claim_id)
{
    QMutexLocker locker(&m_Mutex);
    QString str_select_count = QString("SELECT claim_section FROM `claim` WHERE `claim_id`='%1' LIMIT 0,1").arg(claim_id);
    int nRows = DBHandler::getInstance()->querySize(str_select_count);
    return (nRows > 0);
}


// --------------------------------------------
//QString create_claim_verification_table = "CREATE TABLE IF NOT EXISTS claim_verification"
//       "(ver_id TEXT PRIMARY KEY,"
//       " ver_claimant_nym_id TEXT,"
//       " ver_verifier_nym_id TEXT,"
//       " ver_claim_id TEXT,"
//       " ver_polarity INTEGER,"
//       " ver_start INTEGER,"
//       " ver_end INTEGER,"
//       " ver_signature TEXT,"
//       " ver_signature_verified INTEGER"
//       ")";


// Doesn't lock the mutex.
// Also, the calling function knows already whether the record exists, and what it contains.
//bool MTContactHandler::lowLevelUpdateVerificationPolarity(const QString & qstrClaimId,        const QString & qstrClaimantNymId,
//                                                          const QString & qstrVerificationId, const QString & qstrVerifierNymID,
//                                                          const QString & qstrSignature,
//                                                          const bool bPolarity,
//                                                          const bool bInsertingNew,
//                                                          const bool bSignatureIsVerified)
//{
//    // NOTE: This is all wrong. This is a nice function and all, but the values have to be set
//    // in the Nym itself, and then re-imported into the database.

//    // FYI Upsert is only possible if you replace ALL fields.
//    //
//    QString queryStr;
//    if (bInsertingNew)
//        queryStr = QString("INSERT INTO `claim_verification`"
//                           " (`ver_id`, `ver_claimant_nym_id`, `ver_verifier_nym_id`, `ver_claim_id`, `ver_polarity`, "
//                           "  `ver_start`, `ver_end`, `ver_signature`, `ver_signature_verified`)"
//                           " VALUES(:ver_idBlah, :ver_claimant_nym_idBlah, :ver_verifier_nym_idBlah, :ver_claim_idBlah, :ver_polarityBlah,"
//                           " :ver_startBlah , :ver_endBlah, :ver_signatureBlah, :ver_signature_verifiedBlah)");
//    else
//        queryStr = QString("UPDATE `claim_verification` SET"
//                           " `ver_claimant_nym_id`=:ver_claimant_nym_idBlah,`ver_verifier_nym_id`=:ver_verifier_nym_idBlah,"
//                           " `ver_claim_id`=:ver_claim_idBlah,`ver_polarity`=:ver_polarityBlah,`ver_start`=:ver_startBlah,"
//                           " `ver_end`=:ver_endBlah,`ver_signature`=:ver_signatureBlah,"
//                           " `ver_signature_verified`=:ver_signature_verifiedBlah WHERE `ver_id`=:ver_idBlah");
//    bool bRan = false;

//    try
//    {
//    #ifdef CXX_11
//        std::unique_ptr<DBHandler::PreparedQuery> qu;
//    #else /* CXX_11?  */
//        std::auto_ptr<DBHandler::PreparedQuery> qu;
//    #endif /* CXX_11?  */
//        qu.reset (DBHandler::getInstance ()->prepareQuery (queryStr));
//        // ---------------------------------------------
//        qu->bind (":ver_idBlah", qstrVerificationId);
//        qu->bind (":ver_claimant_nym_idBlah", qstrClaimantNymId);
//        qu->bind (":ver_verifier_nym_idBlah", qstrVerifierNymID);
//        qu->bind (":ver_claim_idBlah", qstrClaimId);
//        qu->bind (":ver_polarityBlah", (bPolarity ? 1 : 0));
//        qu->bind (":ver_startBlah", 0);
//        qu->bind (":ver_endBlah", 0);
//        qu->bind (":ver_signatureBlah", qstrSignature);
//        qu->bind (":ver_signature_verifiedBlah", (bSignatureIsVerified ? 1 : 0));

//        bRan = DBHandler::getInstance ()->runQuery (qu.release ());
//    }
//    catch (const std::exception& exc)
//    {
//        qDebug () << "Error: " << exc.what ();
//        return false;
//    }
//    // -----------------------------
//    if (!bInsertingNew)
//    {
//        if (bRan)
//            return true;
//        else
//            return false;
//    }
//    // -----------------------------
//    const int nRowId = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `claim_verification`", 0, 0);
//    return (nRowId > 0);
//}


static void blah()
{
//resume
//todo

// OT_API.hpp
//EXPORT VerificationSet GetVerificationSet(const Nym& fromNym) const;
// EXPORT bool SetVerifications(Nym& onNym,
//                            const proto::VerificationSet&) const;

// Nym.hpp
//    std::shared_ptr<proto::VerificationSet> VerificationSet() const;
//    bool SetVerificationSet(const proto::VerificationSet& data);

//    proto::Verification Sign(
//        const std::string& claim,
//        const bool polarity,
//        const int64_t start = 0,
//        const int64_t end = 0,
//        const OTPasswordData* pPWData = nullptr) const;
//    bool Verify(const proto::Verification& item) const;

    // VerificationSet has 2 groups, internal and external.
    // Internal is for your signatures on other people's claims.
    // External is for other people's signatures on your claims.
    // When you find that in the external, you copy it to your own credential.
    // ==> So external is for re-publishing other people's verifications of your claims.

    // If we've repudiated any claims, you can add their IDs to the repudiated field in the verification set.
}
//bool MTContactHandler::updateVerificationPolarity(const QString & qstrClaimId,        const QString & qstrClaimantNymId,
//                                                  const QString & qstrVerificationId, const QString & qstrVerifierNymId,
//                                                  const QString & qstrSignature,
//                                                  const bool bPolarity,
//                                                  const bool bInsertingNew,
//                                                  const bool bSignatureIsVerified)
//{

////    typedef std::set<Claim> ClaimSet;

////    // verification identifier, claim identifier, polarity, start time, end time, signature
////    typedef std::tuple<std::string, std::string, bool, int64_t, int64_t, std::string>   Verification;

////    // nymID, verifications
////    typedef std::map<std::string, std::set<Verification>>   VerificationMap;
////
////    // internal verifications, external verifications, repudiated IDs
////    typedef std::tuple<
////        VerificationMap,
////        VerificationMap,
////        std::set<std::string>>   VerificationSet;






//    if (qstrVerifierNymId.isEmpty())
//        return false;

//    opentxs::String strVerifierNymId = qstrVerifierNymId.toStdString();
//    opentxs::Identifier verifierNymId(strVerifierNymId);

//    opentxs::Nym * pVerifierNym = opentxs::OTAPI_Wrap::OTAPI()->GetOrLoadPrivateNym(verifierNymId, false, __FUNCTION__);

//    if (nullptr == pVerifierNym)
//    {
//        qDebug() << __FUNCTION__ << ": Private Nym not found for verifier: " << qstrVerifierNymId;
//        return false;
//    }
//    // ------------------------------


//    VerificationSet SetVerification(
//                Nym& onNym,
//                bool success,
//                const std::string& claimID,
//                const bool polarity,
//                const int64_t start = 0,
//                const int64_t end = 0) const;







//    opentxs::proto::VerificationSet   verification_set = opentxs::OTAPI_Wrap::OTAPI()->GetVerificationSet(*pVerifierNym);
//    opentxs::proto::VerificationSetMap & internal         = std::get<0>(verification_set);
//    opentxs::proto::VerificationSetMap & external         = std::get<1>(verification_set);
//    std::set<std::string>            & repudiatedIDs    = std::get<2>(verification_set);
//    // ------------------------------

//    if (bInsertingNew)
//    {
//        opentxs::proto::VerificationSet verification;

//        internal.insert(std::pair<std::string, std::set<opentxs::proto::VerificationSet>>());
//    }
//    else // Editing existing.
//    {


//    }

//    //    // verification identifier, claim identifier, polarity, start time, end time, signature
//    //    typedef std::tuple<std::string, std::string, bool, int64_t, int64_t, std::string>   Verification;

//    //    proto::Verification Sign(
//    //        const std::string& claim,
//    //        const bool polarity,
//    //        const int64_t start = 0,
//    //        const int64_t end = 0,
//    //        const OTPasswordData* pPWData = nullptr) const;

////    QString create_claim_verification_table = "CREATE TABLE IF NOT EXISTS claim_verification"
////           "(ver_id TEXT PRIMARY KEY,"
////           " ver_claimant_nym_id TEXT,"
////           " ver_verifier_nym_id TEXT,"
////           " ver_claim_id TEXT,"
////           " ver_polarity INTEGER,"
////           " ver_start INTEGER,"
////           " ver_end INTEGER,"
////           " ver_signature TEXT,"
////           " ver_signature_verified INTEGER"
////           ")";

//}

//// --------------------------------------------
//// The bool return value here means, if true, "FYI, I changed something based on this call."
//// Otherwise if false, it means, "FYI, I didn't need to change anything based on this call."
////
//bool MTContactHandler::notifyClaimConfirm(const QString & qstrClaimId, const QString & qstrVerifierNymID)
//{
//    // See if the verifier has verified ClaimId.

//    // If so, get current status and if need be, change Polarity to True.


//    QMutexLocker locker(&m_Mutex);

//    bool bReturnValue = false;

//    QString str_select = QString("SELECT `ver_id`,`ver_polarity` FROM `claim_verification` WHERE `ver_claim_id`='%1' AND `ver_verifier_nym_id`='%2' LIMIT 0,1").
//            arg(qstrClaimId).arg(qstrVerifierNymID);

//    int nRows = 0;
//    try
//    {
//       nRows = DBHandler::getInstance()->querySize(str_select);
//    }
//    catch (const std::exception& exc)
//    {
//        qDebug () << "Error: " << exc.what ();
//        return bReturnValue;
//    }
//    // ---------------------------------------
//    if (nRows > 0)
//    {
//        const bool bCurrentPolarity = (0 == DBHandler::getInstance()->queryInt(str_select, 1, 0)) ? false : true;

//        if (bCurrentPolarity) // It was already confirmed, so we don't need to do anything.
//            return false; // False means, no action was necessary (since it was already confirmed.)
//        // ---------------------------
//        // Else, the polarity was set to "REFUTE" on an existing verification.
//        // So we need to get the verifications on the Nym, change this one,
//        // and then set the verifications again.
//        // Then we return true (had to make a change) so the calling function can re-import
//        // the Nym back into the local database and refresh the UI based on that.

//        bReturnValue = true;
//    }
//    else // No rows found.
//    {
//        // No existing confirmation or refutation was found. Therefore we create a new one,
//        // in a similar process to that seen above.
//        // Basically we need to get the verifications on the Nym, add this one,
//        // and then set the verifications again.



//        // Return true since we had to add a verification (we had to "change" something.)
//        bReturnValue = true;
//    }
//    // ---------------------------------------
//    return bReturnValue;
//}


bool MTContactHandler::claimVerificationLowlevel(const QString & qstrClaimId, const QString & qstrClaimantNymId,
                                                 const QString & qstrVerifierNymId, opentxs::ClaimPolarity claimPolarity)
{
    if (qstrVerifierNymId.isEmpty() || qstrClaimId.isEmpty())
        return false;


    qDebug() << "DEBUGGING: claimVerificationLowlevel: USER selected claimPolarity (and setting in OT): " << claimPolarityToInt(claimPolarity);

    bool bChanged = false; // So we know if OT had to change anything when it set the verification. (Maybe it was already there.)

    opentxs::OTPasswordData thePWData(QString(QObject::tr("We've almost bubbled up to the top!! Confirming/refuting a claim.")).toStdString().c_str());

    auto data = opentxs::OTAPI_Wrap::Exec()->SetVerification(
                bChanged,
                qstrVerifierNymId.toStdString(),
                qstrClaimantNymId.toStdString(),
                qstrClaimId.toStdString(),
                claimPolarity,
                0, // start. todo.
                0); // end. todo. bitemporal.

    const auto the_set =
        opentxs::proto::DataToProto<opentxs::proto::VerificationSet>(
            opentxs::Data(data.c_str(), static_cast<uint32_t>(data.length())));


    qDebug() << QString("%1").arg(QString(bChanged ? "YES, I CHANGED A VALUE (ACCORDING TO OT)" : "**NO** didn't CHANGE A VALUE, ACCORDING TO OT." ));

    // Internal verifications:
    // Here I'm looping through pCurrentNym's verifications of other people's claims.
    for (auto& claimant: the_set.internal().identity()) {
        // Here we're looping through those other people. (Claimants.)
        for (auto& verification : claimant.verification()) {
            const bool ver_polarity = verification.valid();

            qDebug() << QString("%1").arg(QString(ver_polarity ? "==>Polarity is now positive" : "==>Polarity is now negative" ));
        }
    }

    return bChanged;
}

bool MTContactHandler::claimVerificationConfirm(const QString & qstrClaimId, const QString & qstrClaimantNymId, const QString & qstrVerifierNymId)
{
    return claimVerificationLowlevel(qstrClaimId, qstrClaimantNymId, qstrVerifierNymId, opentxs::ClaimPolarity::POSITIVE);
}

bool MTContactHandler::claimVerificationRefute(const QString & qstrClaimId, const QString & qstrClaimantNymId, const QString & qstrVerifierNymId)
{
    return claimVerificationLowlevel(qstrClaimId, qstrClaimantNymId, qstrVerifierNymId, opentxs::ClaimPolarity::NEGATIVE);
}

bool MTContactHandler::claimVerificationNoComment(const QString & qstrClaimId, const QString & qstrClaimantNymId, const QString & qstrVerifierNymId)
{
    return claimVerificationLowlevel(qstrClaimId, qstrClaimantNymId, qstrVerifierNymId, opentxs::ClaimPolarity::NEUTRAL);
}

bool MTContactHandler::getPolarityIfAny(const QString & claim_id, const QString & verifier_nym_id, bool & bPolarity)
{
    QMutexLocker locker(&m_Mutex);

    bool bReturnValue = false;

    QString str_select = QString("SELECT `ver_id`,`ver_polarity` FROM `claim_verification` WHERE `ver_claim_id`='%1' AND `ver_verifier_nym_id`='%2' LIMIT 0,1").
            arg(claim_id).arg(verifier_nym_id);

    int nRows = 0;
    try
    {
       nRows = DBHandler::getInstance()->querySize(str_select);
       // ---------------------------------------
       if (nRows > 0)
       {
           const int polarity = DBHandler::getInstance()->queryInt(str_select, 1, 0);
           opentxs::ClaimPolarity claimPolarity = intToClaimPolarity(polarity);

           if (opentxs::ClaimPolarity::NEUTRAL == claimPolarity)
               qDebug() << __FUNCTION__ << ": ERROR! A claim verification can't have neutral polarity, since that "
                           "means no verification exists. How did it get into the database this way?";
           else
           {
               bReturnValue = true;
               bPolarity = (opentxs::ClaimPolarity::NEGATIVE == claimPolarity) ? false : true;
           }


//           qDebug() << "DEBUGGING: getPolarityIfAny (from database): bPolarity: " << QString(bPolarity ? "True" : "False");

       }
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return bReturnValue;
    }
    // ---------------------------------------
    return bReturnValue;
}


QString MTContactHandler::getDisplayNameFromClaims(const QString & claimant_nym_id)
{
    QMutexLocker locker(&m_Mutex);

    QString qstrReturnVal;
    QString str_select = QString("SELECT `claim_value`, `claim_att_active`, `claim_att_primary` FROM `claim` WHERE `claim_nym_id`='%1' AND `claim_section`=%2").
            arg(claimant_nym_id).arg(opentxs::proto::CONTACTSECTION_IDENTIFIER);

    int nRows = 0;
    try
    {
       nRows = DBHandler::getInstance()->querySize(str_select);
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return qstrReturnVal;
    }
    // ---------------------------------------
    if (nRows > 0)
    {
        QString qstrName, qstrFirstName, qstrInactiveName;
        bool bActive  = false;
        bool bPrimary = false;

        for (int nCurrentRow = 0; nCurrentRow < nRows; ++nCurrentRow)
        {
            const QString temp = DBHandler::getInstance()->queryString(str_select, 0, nCurrentRow);
            const int nActive  = DBHandler::getInstance()->queryInt(str_select, 1, nCurrentRow);
            const int nPrimary = DBHandler::getInstance()->queryInt(str_select, 2, nCurrentRow);

            bActive  = !(0 == nActive);
            bPrimary = !(0 == nPrimary);
            // --------------------------
            if (temp.isEmpty())
                continue;
            qstrName = temp;
            // --------------------------
            if (!bActive)
                qstrInactiveName = qstrName;
            else if (0 == nCurrentRow)
                qstrFirstName = qstrName;
            // --------------------------
            if (bPrimary)
                break;
        }

        qstrReturnVal = bPrimary ? qstrName :
                                   (!qstrFirstName.isEmpty() ? qstrFirstName : qstrInactiveName);
    }
    // ---------------------------------------
    return qstrReturnVal;
}

QString MTContactHandler::getNymIdFromClaimsByBtMsg(const QString & bitmessage_address)
{
    QMutexLocker locker(&m_Mutex);

    QString qstrReturnVal;
    QString str_select = QString("SELECT `claim_nym_id` FROM `claim` WHERE `claim_value`='%1' AND `claim_section`=%2 AND `claim_type`=%3")
            .arg(bitmessage_address)
            .arg(opentxs::proto::CONTACTSECTION_COMMUNICATION)
            .arg(opentxs::proto::CITEMTYPE_BITMESSAGE);

    int nRows = 0;
    try
    {
       nRows = DBHandler::getInstance()->querySize(str_select);
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return qstrReturnVal;
    }
    // ---------------------------------------
    if (nRows > 0)
    {
        QString qstrNymId;

        int nFound = 0;
        for (int nCurrentRow = 0; nCurrentRow < nRows; ++nCurrentRow)
        {
            const QString temp = DBHandler::getInstance()->queryString(str_select, 0, nCurrentRow);
            // --------------------------
            if (temp.isEmpty())
                continue;
            nFound++;
            // --------------------------
            if (1 == nFound)
                qstrNymId = temp;
            else
                qDebug() << "WARNING JUSTUS: Right now we're just grabbing the first NymId that matches a Bitmessage address, BUT there were multiple matches! CLAIM RETURNED MAY BE FALSE. Need rules "
                            "here so we only return a verified claim!";
        }

        qstrReturnVal = qstrNymId;
    }
    // ---------------------------------------
    return qstrReturnVal;
}

QString MTContactHandler::getBitmessageAddressFromClaims(const QString & claimant_nym_id)
{
    QMutexLocker locker(&m_Mutex);

    QString qstrReturnVal;
    QString str_select = QString("SELECT `claim_value`, `claim_att_active`, `claim_att_primary` FROM `claim` WHERE `claim_nym_id`='%1' AND `claim_section`=%2 AND `claim_type`=%3")
            .arg(claimant_nym_id)
            .arg(opentxs::proto::CONTACTSECTION_COMMUNICATION)
            .arg(opentxs::proto::CITEMTYPE_BITMESSAGE);

    int nRows = 0;
    try
    {
       nRows = DBHandler::getInstance()->querySize(str_select);
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return qstrReturnVal;
    }
    // ---------------------------------------
    if (nRows > 0)
    {
        QString qstrAddress, qstrFirstAddress, qstrInactiveAddress;
        bool bActive  = false;
        bool bPrimary = false;

        int nFound = 0;
        for (int nCurrentRow = 0; nCurrentRow < nRows; ++nCurrentRow)
        {
            const QString temp = DBHandler::getInstance()->queryString(str_select, 0, nCurrentRow);
            const int nActive  = DBHandler::getInstance()->queryInt(str_select, 1, nCurrentRow);
            const int nPrimary = DBHandler::getInstance()->queryInt(str_select, 2, nCurrentRow);

            bActive  = !(0 == nActive);
            bPrimary = !(0 == nPrimary);
            // --------------------------
            if (temp.isEmpty())
                continue;
            qstrAddress = temp;
            nFound++;
            // --------------------------
            if (!bActive)
                qstrInactiveAddress = qstrAddress;
            else if (1 == nFound)
                qstrFirstAddress = qstrAddress;
            // --------------------------
            if (bPrimary)
                break;
        }

        qstrReturnVal = bPrimary ? qstrAddress :
                                   (!qstrFirstAddress.isEmpty() ? qstrFirstAddress : qstrInactiveAddress);
    }
    // ---------------------------------------
    return qstrReturnVal;
}


void MTContactHandler::clearClaimsForNym(const QString & qstrNymId)
{
    QMutexLocker locker(&m_Mutex);

    if (qstrNymId.isEmpty())
        return;
    // ------------------------------------------------------------
    QString str_delete_claim  = QString("DELETE FROM `claim` WHERE `claim_nym_id`='%1'").arg(qstrNymId);
    QString str_delete_verify = QString("DELETE FROM `claim_verification` WHERE `ver_verifier_nym_id`='%1'").arg(qstrNymId);

    try {
        DBHandler::getInstance()->runQuery(str_delete_claim);
        DBHandler::getInstance()->runQuery(str_delete_verify);
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return;
    }
}

bool MTContactHandler::upsertClaim(
    const opentxs::Nym& nym,
    const uint32_t section,
    const opentxs::proto::ContactItem& claim)
{
    QMutexLocker locker(&m_Mutex);

    const opentxs::Identifier nym_id(nym);
    const opentxs::String     strNym(nym_id);
    const std::string         str_nym_id(strNym.Get());
    const QString             qstrNymId(QString::fromStdString(str_nym_id));

    const QString claim_id = QString::fromStdString(claim.id());
    const uint32_t claim_section = section;
    const uint32_t claim_type = claim.type();
    const QString claim_value = QString::fromStdString(claim.value());
    const int64_t claim_start = claim.start();
    const int64_t claim_end = claim.end();

    bool claim_att_active  = false;
    bool claim_att_primary = false;

    opentxs::NumList numlistAttributes;
    for (const auto& attribute: claim.attribute()) {
        numlistAttributes.Add(attribute);

        if (opentxs::proto::CITEMATTR_ACTIVE  == attribute) {
            claim_att_active  = true;
        }

        if (opentxs::proto::CITEMATTR_PRIMARY == attribute) {
            claim_att_primary = true;
        }
    }

    opentxs::String strAttributes;
    numlistAttributes.Output(strAttributes);
    const std::string str_attributes(strAttributes.Get());
    const QString qstrAttributes(QString::fromStdString(str_attributes));
    // ------------------------------------------------------------
    QString str_select_count = QString("SELECT claim_section FROM `claim` WHERE `claim_id`='%1' LIMIT 0,1").arg(claim_id);

    const bool bClaimExists = (DBHandler::getInstance()->querySize(str_select_count) > 0);
    // ------------------------------------------------------------
    // TODO: Do a real upsert here instead of this crap.
    // UPDATE: Upsert is only possible if you replace ALL fields.
    //
    QString queryStr;
    if (!bClaimExists)
        queryStr = QString("INSERT INTO `claim`"
                           " (`claim_id`, `claim_nym_id`, `claim_section`, `claim_type`, `claim_value`,"
                           "  `claim_start`, `claim_end`, `claim_attributes`, `claim_att_active`, `claim_att_primary`)"
                           "  VALUES(:claim_idBlah, :claim_nym_idBlah, :claim_sectionBlah, :claim_typeBlah, :claim_valueBlah,"
                           ":claim_startBlah , :claim_endBlah, :claim_attributesBlah, :claim_att_activeBlah, :claim_att_primaryBlah)");

    else
        queryStr = QString("UPDATE `claim` SET"
                           " `claim_nym_id`=:claim_nym_idBlah, `claim_section`=:claim_sectionBlah,`claim_type`=:claim_typeBlah,"
                           " `claim_value`=:claim_valueBlah,`claim_start`=:claim_startBlah,`claim_end`=:claim_endBlah,"
                           " `claim_attributes`=:claim_attributesBlah,`claim_att_active`=:claim_att_activeBlah,"
                           " `claim_att_primary`=:claim_att_primaryBlah WHERE `claim_id`=:claim_idBlah");
    bool bRan = false;

    try
    {
    #ifdef CXX_11
        std::unique_ptr<DBHandler::PreparedQuery> qu;
    #else /* CXX_11?  */
        std::auto_ptr<DBHandler::PreparedQuery> qu;
    #endif /* CXX_11?  */
        qu.reset (DBHandler::getInstance ()->prepareQuery (queryStr));
        // ---------------------------------------------
        qu->bind (":claim_idBlah", claim_id);
        qu->bind (":claim_nym_idBlah", qstrNymId);
        qu->bind (":claim_sectionBlah", claim_section);
        qu->bind (":claim_typeBlah", claim_type);
        qu->bind (":claim_valueBlah", claim_value);
        qu->bind (":claim_startBlah", QVariant::fromValue(claim_start));
        qu->bind (":claim_endBlah", QVariant::fromValue(claim_end));
        qu->bind (":claim_attributesBlah", qstrAttributes);
        qu->bind (":claim_att_activeBlah", (claim_att_active ? 1 : 0));
        qu->bind (":claim_att_primaryBlah", (claim_att_primary ? 1 : 0));

        bRan = DBHandler::getInstance ()->runQuery (qu.release ());
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return false;
    }
    // -----------------------------
    if (bClaimExists)
    {
        if (bRan)
            return true;
        else
            return false;
    }
    // -----------------------------
    const int nRowId = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `claim`", 0, 0);
    return (nRowId > 0);
}




bool MTContactHandler::upsertClaimVerification(const std::string & claimant_nym_id,
                                               const std::string & verifier_nym_id,
                                               const opentxs::proto::Verification & verification,
                                               const bool bIsInternal/*=true*/)
{
    QMutexLocker locker(&m_Mutex);

    const QString qstrVerifierNymId(QString::fromStdString(verifier_nym_id));
    // ---------------------------------------------------
    // verification identifier, claim identifier, polarity, start time, end time, signature
//  typedef std::tuple<std::string, std::string, bool, int64_t, int64_t, std::string> Verification;

    const QString  ver_id       = QString::fromStdString(verification.id());
    const QString  ver_claim_id = QString::fromStdString(verification.claim());
    const bool     ver_polarity = verification.valid();
    const int64_t  ver_start    = verification.start();
    const int64_t  ver_end      = verification.end();

    qDebug() << "DEBUGGING: upsertClaimVerification: ver_polarity according to OT is NOT neutral!: " << QString(ver_polarity ? "True" : "False");


    QString  ver_sig("");

    if (!bIsInternal)
        ver_sig = QString::fromStdString(verification.sig().signature());
    // NOTE: Signature is always an empty string for internal verifications.
    // That's because OT already verified it, before even allowing it onto the
    // internal list in the first place. So we wouldn't have even seen this at all,
    // if it hadn't already been known to be verified.
    // Therefore OT just passes an empty string for the signature, and we mark in the database
    // table that the signature has verified (because according to OT, it has.)
    // So then why have "signature" and "signature verified" fields at all? Because when we
    // import the EXTERNAL claim verifications, the signature is not necessarily verified yet,
    // so we will need to store it -- and mark it as not verified -- until such time as we are
    // able to verify it, probably in a background process, download the related Nym, verify his
    // signature, then mark it as verified in the DB.
    // ---------------------------------------------------
    QString str_select_count = QString("SELECT ver_id FROM `claim_verification` WHERE `ver_id`='%1' LIMIT 0,1").arg(ver_id);

    const bool bVerificationExists = (DBHandler::getInstance()->querySize(str_select_count) > 0);
    // ------------------------------------------------------------
//    QString create_claim_verification_table = "CREATE TABLE IF NOT EXISTS claim_verification"
//           "(ver_id TEXT PRIMARY KEY,"
//           " ver_claimant_nym_id TEXT,"
//           " ver_verifier_nym_id TEXT,"
//           " ver_claim_id TEXT,"
//           " ver_polarity INTEGER,"
//           " ver_start INTEGER,"8 AM
//           " ver_end INTEGER,"
//           " ver_signature TEXT,"
//           " ver_signature_verified INTEGER"
//           ")";

    // TODO: Do a real upsert here instead of this crap.
    //
    QString str_insert;
    if (!bVerificationExists)
        str_insert = QString("INSERT INTO `claim_verification`"
                                 " (`ver_id`, `ver_claimant_nym_id`, `ver_verifier_nym_id`, `ver_claim_id`, `ver_polarity`,"
                                 "  `ver_start`, `ver_end`, `ver_signature`, `ver_signature_verified`)"
                                 "  VALUES('%1', '%2', '%3', '%4', %5, %6, %7, '%8', %9)").
                arg(ver_id).arg(QString::fromStdString(claimant_nym_id)).arg(qstrVerifierNymId).arg(ver_claim_id).
                arg(ver_polarity ? claimPolarityToInt(opentxs::ClaimPolarity::POSITIVE) : claimPolarityToInt(opentxs::ClaimPolarity::NEGATIVE)).
                arg(ver_start).
                arg(ver_end).arg(ver_sig).
                arg(bIsInternal ? 1 : 0);
    else
        str_insert = QString("UPDATE `claim_verification` SET"
                             " `ver_claimant_nym_id`='%1', `ver_verifier_nym_id`='%2',`ver_claim_id`='%3',`ver_polarity`=%4,`ver_start`=%5,`ver_end`=%6,"
                             " `ver_signature`='%7' WHERE `ver_id`='%8'").
                arg(QString::fromStdString(claimant_nym_id)).arg(qstrVerifierNymId).arg(ver_claim_id).
                arg(ver_polarity ? claimPolarityToInt(opentxs::ClaimPolarity::POSITIVE) : claimPolarityToInt(opentxs::ClaimPolarity::NEGATIVE)).
                arg(ver_start).
                arg(ver_end).arg(ver_sig).
                arg(ver_id);
    const bool bRan = DBHandler::getInstance()->runQuery(str_insert);

    if (bVerificationExists)
    {
        if (bRan)
            return true;
        else
            return false;
    }

    const int nRowId = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `claim_verification`", 0, 0);
    return (nRowId > 0);
}

bool MTContactHandler::ArchivedTradeReceiptExists(int64_t lOfferID)
{
    QMutexLocker locker(&m_Mutex);
    QString str_select = QString("SELECT * FROM `trade_archive` WHERE `offer_id`=%1 LIMIT 0,1").arg(lOfferID);
    int nRows = DBHandler::getInstance()->querySize(str_select);
    return (nRows > 0);
}

bool MTContactHandler::ContactExists(int nContactID)
{
    QMutexLocker locker(&m_Mutex);
    QString str_select = QString("SELECT * FROM `contact` WHERE `contact_id`=%1 LIMIT 0,1").arg(nContactID);
    int nRows = DBHandler::getInstance()->querySize(str_select);
    return (nRows > 0);
}


bool MTContactHandler::DeleteContact(int nContactID)
{
    QMutexLocker locker(&m_Mutex);

    QString str_delete_nym     = QString("DELETE FROM `nym` WHERE `contact_id`=%1").arg(nContactID);
    QString str_delete_method  = QString("DELETE FROM `contact_method` WHERE `contact_id`=%1").arg(nContactID);
    QString str_delete_contact = QString("DELETE FROM `contact` WHERE `contact_id`=%1").arg(nContactID);

    return (DBHandler::getInstance()->runQuery(str_delete_nym)     &&
            DBHandler::getInstance()->runQuery(str_delete_method)  &&
            DBHandler::getInstance()->runQuery(str_delete_contact));
}

// See if a given Contact ID is associated with a given NymID.
bool MTContactHandler::VerifyNymOnExistingContact(int nContactID, QString nym_id_string)
{
    const int nFoundContactID = this->FindContactIDByNymID(nym_id_string);

    if ((nFoundContactID > 0) && (nFoundContactID == nContactID))
        return true;

    return false;
}



bool MTContactHandler::AddNymToExistingContact(int nContactID, QString nym_id_string, QString payment_code/*=""*/)
{
    QMutexLocker locker(&m_Mutex);

    // Todo security: Make sure we don't need to encode the NymID here to prevent a SQL injection vulnerability...

    if ((nContactID > 0) && !nym_id_string.isEmpty())
    {
        // First, see if a contact already exists for this Nym, and if so,
        // save its ID and return at the bottom.
        //
        QString str_select = QString("SELECT `contact_id` FROM `nym` WHERE `nym_id`='%1'").arg(nym_id_string);

//      qDebug() << QString("Running query: %1").arg(str_select);

        int  nRows      = DBHandler::getInstance()->querySize(str_select);
        bool bNymExists = (nRows > 0); // Whether the contact ID was good or not, the Nym itself DOES exist.
        // ----------------------------------------
        QString str_insert_nym;

        if (!bNymExists)
            str_insert_nym = QString("INSERT INTO `nym` "
                                     "(`nym_id`, `contact_id`, `nym_payment_code`) "
                                     "VALUES('%1', %2, '%3')").arg(nym_id_string).arg(nContactID).arg(payment_code);
        else if (!payment_code.isEmpty())
            str_insert_nym = QString("UPDATE `nym` SET `contact_id`=%1,`nym_payment_code`='%2' WHERE `nym_id`='%3'")
                    .arg(nContactID).arg(payment_code).arg(nym_id_string);
        else
            str_insert_nym = QString("UPDATE `nym` SET `contact_id`=%1 WHERE `nym_id`='%2'").arg(nContactID).arg(nym_id_string);

        if (!str_insert_nym.isEmpty())
        {
            //qDebug() << QString("Running query: %1").arg(str_insert_nym);

            return DBHandler::getInstance()->runQuery(str_insert_nym);
        }
    }

    return false;
}


// TODO:
// Function to delete a contact (I think also: leaving behind the nym/account/server records for that contact...?
// Otherwise cleaning those out. Either way, still needs to wipe the nym table's record of the contact ID that got wiped.)
// Function to return a list of servers, filtered by Nym.
// Function to return a list of accounts, filtered by Nym.
// Function to return a list of accounts, filtered by Server.
// Function to return a list of accounts, filtered by Server and Asset Type.
// Function to return a list of accounts, filtered by Nym and Server.
// Function to return a list of accounts, filtered by Nym and Asset Type.
// Function to return a list of accounts, filtered by Nym, Server, and Asset Type.
// Function to return a list of asset types, filtered by Nym (account join).
// Function to return a list of asset types, filtered by Server (account join).

// NOTE: if the caller wants a list of servers NOT filtered by Nym, then he can
// just directly use the OT API. So you'd only call THIS function if you DID
// want it filtered by Nym (that's why the nym parameter isn't optional.)
//
bool MTContactHandler::GetServers(mapIDName & theMap, QString filterByNym, bool bPrependOTType/*=false*/)
{
    QMutexLocker locker(&m_Mutex);

    QString str_select = QString("SELECT (`notary_id`) FROM `nym_server` WHERE `nym_id`='%1'").arg(filterByNym);

    bool bFoundAny = false;
    int  nRows     = DBHandler::getInstance()->querySize(str_select);

    for(int ii=0; ii < nRows; ii++)
    {
        QString notary_id = DBHandler::getInstance()->queryString(str_select, 0, ii);

        if (!notary_id.isEmpty())
        {
            QString server_name = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetServer_Name(notary_id.toStdString()));

            if (!server_name.isEmpty())
            {
                bFoundAny = true;

                // Some callers need "otserver|notary_id" instead of just "notary_id"
                // We prepend here, if necessary.
                //
                QString qstrFinalID   = notary_id;
                QString qstrFinalName = server_name;

                if (bPrependOTType)
                {
                    qstrFinalID   = QString("%1|%2").arg("otserver").arg(notary_id);
                    qstrFinalName = QString("%1: %2").arg(QObject::tr("OT Server")).arg(server_name);
                }
                // At this point we have the server ID *and* the server name.
                // So we can add them to our map...
                theMap.insert(qstrFinalID, qstrFinalName);
            }
        }
    }
    // ---------------------------------------------------------------------
    return bFoundAny;
}


//QString create_contact = "CREATE TABLE contact(contact_id INTEGER PRIMARY KEY, contact_display_name TEXT)";
//QString create_nym     = "CREATE TABLE nym(nym_id TEXT PRIMARY KEY, contact_id INTEGER, nym_display_name TEXT)";
//QString create_server  = "CREATE TABLE nym_server(nym_id TEXT, notary_id TEXT, PRIMARY KEY(nym_id, notary_id))";
//QString create_account = "CREATE TABLE nym_account(account_id TEXT PRIMARY KEY, notary_id TEXT, nym_id TEXT, asset_id TEXT, account_display_name TEXT)";

// ---------------------------------------------------------------------
bool MTContactHandler::GetServers(mapIDName & theMap, bool bPrependOTType/*=false*/)
{
    bool    bFoundAny    = false;
    int32_t nServerCount = opentxs::OTAPI_Wrap::Exec()->GetServerCount();

    for (int32_t ii = 0; ii < nServerCount; ++ii)
    {
        std::string str_notary_id   = opentxs::OTAPI_Wrap::Exec()->GetServer_ID(ii);
        std::string str_server_name = opentxs::OTAPI_Wrap::Exec()->GetServer_Name(str_notary_id);

        QString qstrNotaryID   = QString::fromStdString(str_notary_id);
        QString qstrServerName = QString::fromStdString(str_server_name);

        QString qstrFinalID   = qstrNotaryID;
        QString qstrFinalName = qstrServerName;

        if (bPrependOTType)
        {
            qstrFinalID   = QString("%1|%2").arg("otserver").arg(qstrNotaryID);
            qstrFinalName = QString("%1: %2").arg(QObject::tr("OT Server")).arg(qstrServerName);
        }

        // At this point we have the server ID *and* the server name.
        // So we can add them to our map...
        if (!qstrServerName.isEmpty())
        {
            theMap.insert(qstrFinalID, qstrFinalName);
            bFoundAny = true;
        }
    }

    return bFoundAny;
}

bool MTContactHandler::GetServers(mapIDName & theMap, int nFilterByContact, bool bPrependOTType/*=false*/)
{
    QMutexLocker locker(&m_Mutex);

    QString str_select = QString("SELECT (`notary_id`) "
                                 "FROM `nym_server` "
                                 "INNER JOIN `nym` "
                                 "ON nym.nym_id=nym_server.nym_id "
                                 "WHERE nym.contact_id=%1").arg(nFilterByContact);

    bool bFoundAny = false;
    int  nRows     = DBHandler::getInstance()->querySize(str_select);

    for(int ii=0; ii < nRows; ii++)
    {
        QString notary_id = DBHandler::getInstance()->queryString(str_select, 0, ii);

        if (!notary_id.isEmpty())
        {
            QString server_name = QString::fromStdString(opentxs::OTAPI_Wrap::Exec()->GetServer_Name(notary_id.toStdString()));

            if (!server_name.isEmpty())
            {
                bFoundAny = true;

                // Some callers need "otserver|notary_id" instead of just "notary_id"
                // We prepend here, if necessary.
                //
                QString qstrFinalID   = notary_id;
                QString qstrFinalName = server_name;

                if (bPrependOTType)
                {
                    qstrFinalID   = QString("%1|%2").arg("otserver").arg(notary_id);
                    qstrFinalName = QString("%1: %2").arg(QObject::tr("OT Server")).arg(server_name);
                }

                // At this point we have the server ID *and* the server name.
                // So we can add them to our map...
                theMap.insert(qstrFinalID, qstrFinalName);
            }
        }
    }
    // ---------------------------------------------------------------------
    return bFoundAny;
}


bool MTContactHandler::GetSmartContracts(mapIDName & theMap)
{
    QMutexLocker locker(&m_Mutex);

    QString str_select = QString("SELECT `template_id`,`template_display_name` FROM `smart_contract`");

    bool bFoundAny = false;
    int  nRows     = DBHandler::getInstance()->querySize(str_select);

    for(int ii=0; ii < nRows; ii++)
    {
        int     template_id   = DBHandler::getInstance()->queryInt   (str_select, 0, ii);
        QString template_name = DBHandler::getInstance()->queryString(str_select, 1, ii);

        if (template_id > 0)
        {
            bFoundAny = true;

            QString str_template_id;
            str_template_id = QString("%1").arg(template_id);

            if (!template_name.isEmpty())
            {
//              qDebug() << QString("About to decode name: %1").arg(template_name);

                //Decode base64.
                template_name = Decode(template_name);
            }
            // --------------------------------------------------
            // At this point we have the template ID (in string form) *and* the template name.
            // So we can add them to our map...
            theMap.insert(str_template_id, template_name);
        }
    }
    // ---------------------------------------------------------------------
    return bFoundAny;
}

bool MTContactHandler::DeleteSmartContract(int nID)
{
    QMutexLocker locker(&m_Mutex);

    QString str_delete = QString("DELETE FROM `smart_contract` WHERE `template_id`=%1").arg(nID);

    return DBHandler::getInstance()->runQuery(str_delete);
}

bool MTContactHandler::DeleteManagedPassphrase(int nID)
{
    QMutexLocker locker(&m_Mutex);

    QString str_delete = QString("DELETE FROM `managed_passphrase` WHERE `passphrase_id`=%1").arg(nID);

    return DBHandler::getInstance()->runQuery(str_delete);
}

QString MTContactHandler::GetSmartContract(int nID)
{
    return MTContactHandler::GetValueByID(nID, "template_contents", "smart_contract", "template_id");
}

// -------------------------------------------------

QString MTContactHandler::GetMessageBody(int nID)
{
    return MTContactHandler::GetEncryptedValueByID(nID, "body", "message_body", "message_id");
}

bool MTContactHandler::UpdateMessageBody(int nMessageID, const QString & qstrBody)
{
    QMutexLocker locker(&m_Mutex);

    return LowLevelUpdateMessageBody(nMessageID, qstrBody);
}

bool MTContactHandler::DeleteMessageBody(int nID)
{
    QMutexLocker locker(&m_Mutex);

    QString str_delete = QString("DELETE FROM `message_body` WHERE `message_id`=%1").arg(nID);

    return DBHandler::getInstance()->runQuery(str_delete);
}


// Since there is a message table, the message_id is already pre-existing by the time
// the entry is added to the message_body table. (FYI.) This function assumes that
// the message was JUST added, and that the body we're about to add corresponds to that
// same message. So we simply look up the message_id for the last row inserted and then
// add the body to the message_body table using that same ID.
//
bool MTContactHandler::CreateMessageBody(QString qstrBody)
{
    QMutexLocker locker(&m_Mutex);

    const int nID = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `message`", 0, 0);
    // ----------------------------------------
    if (nID > 0)
    {
        QString str_insert = QString("INSERT INTO `message_body` "
                                     "(`message_id`) "
                                     "VALUES(%1)").arg(nID);
        DBHandler::getInstance()->runQuery(str_insert);
        // ----------------------------------------
        const int nMessageID = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `message_body`", 0, 0);

        if (nMessageID == nID)
        {
            bool bUpdated = LowLevelUpdateMessageBody(nMessageID, qstrBody);

            if (!bUpdated)
            {
                qDebug() << QString("Failed updating message body for message_id: %1").arg(nMessageID);
                return false;
            }
            return true;
        }
    }
    return false;
}

bool MTContactHandler::LowLevelUpdateMessageBody(int nMessageID, const QString & qstrBody)
{
//  NOTE: This function ASSUMES that the calling function already locked the Mutex.
//  QMutexLocker locker(&m_Mutex);

    try
    {
        // The body is encrypted.
        //
        bool bSetValue = SetEncryptedValueByID(nMessageID, qstrBody, "body", "message_body", "message_id");
        Q_UNUSED(bSetValue);
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return false;
    }

    return true;
}


// --------------------------------------------

int MTContactHandler::GetOrCreateLiveAgreementId(const int64_t transNumDisplay, const QString & notaryID,
                                                 const QString & qstrEncodedMemo, const int nFolder, int & lastKnownState) // returns nAgreementId
{
    QMutexLocker locker(&m_Mutex);

    QString str_select = QString("SELECT `agreement_id`, `newest_known_state` FROM `agreement` WHERE `txn_id_display`=%1 AND `notary_id`='%2' LIMIT 0,1").arg(transNumDisplay).arg(notaryID);
    const int nRows = DBHandler::getInstance()->querySize(str_select);

    if (nRows > 0)
    {
        // 0 Error, 1 Paid, 2 Payment failed, 3 Contract not active.
        lastKnownState = DBHandler::getInstance()->queryInt(str_select, 1, 0);

        return DBHandler::getInstance()->queryInt(str_select, 0, 0);
    }
    // ----------------------------------
    // Let's create it then.
    //
    const int have_read = 0;
    try
    {
        QString memo1 = qstrEncodedMemo.isEmpty() ? "" : "`memo`, ";
        QString memo2 = qstrEncodedMemo.isEmpty() ? "" : ":blah_memo, ";

        QString queryStr = QString("INSERT INTO `agreement` "
                           "(`agreement_id`, `have_read`, `txn_id_display`, `notary_id`, "
                                   "%1"
                                   "`folder`) "
                           "VALUES(NULL, :blah_have_read, :blah_txn_id_display, :blah_notary_id, "
                                   "%2"
                                   ":blah_folder)").arg(memo1).arg(memo2);
    #ifdef CXX_11
        std::unique_ptr<DBHandler::PreparedQuery> qu;
    #else /* CXX_11?  */
        std::auto_ptr<DBHandler::PreparedQuery> qu;
    #endif /* CXX_11?  */
        qu.reset (DBHandler::getInstance ()->prepareQuery (queryStr));
        qu->bind (":blah_have_read", have_read);
        qu->bind (":blah_txn_id_display", QVariant::fromValue(transNumDisplay));
        qu->bind (":blah_notary_id", notaryID);
        if (!qstrEncodedMemo.isEmpty())
            qu->bind (":blah_memo", qstrEncodedMemo);
        qu->bind (":blah_folder", nFolder);
        DBHandler::getInstance ()->runQuery (qu.release ());
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return 0;
    }
    // ----------------------------------
//    QString str_insert = QString("INSERT INTO `agreement` "
//                                 "(`agreement_id`, `have_read`, `txn_id_display`, `notary_id`, `memo`, `folder`) "
//                                 "VALUES(NULL, %1, %2, '%3', '%4', %5)").arg(0).arg(transNumDisplay).arg(notaryID).arg(qstrEncodedMemo).arg(nFolder);
//    DBHandler::getInstance()->runQuery(str_insert);
    // ----------------------------------------
    return DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `agreement`", 0, 0);
}


bool MTContactHandler::UpdateLiveAgreementRecord(const int nAgreementId, const int64_t nNewestReceiptNum, const int nNewestKnownState,
                                                 const int64_t timestamp, const QString qstrEncodedMemo)
{
    QMutexLocker locker(&m_Mutex);

    try
    {
        QString qstrKnownState, qstrMemo;

        if (nNewestKnownState > 0)
            qstrKnownState = " `newest_known_state` = :blah_newest_known_state,";
        else
            qstrKnownState = "";

        if (!qstrEncodedMemo.isEmpty())
            qstrMemo = " `memo` = :blah_memo,";
        else
            qstrMemo = "";

        QString queryStr =
                 QString("UPDATE `agreement` SET"
                         " `have_read` = 0,"
                         " `newest_receipt_id` = :blah_newest_receipt_id,"
                         "%1" // newest_known_state
                         "%2" // memo
                         " `timestamp` = :blah_timestamp"
                         " WHERE `agreement_id` = :blah_agreement_id").arg(qstrKnownState).arg(qstrMemo);
    #ifdef CXX_11
        std::unique_ptr<DBHandler::PreparedQuery> qu;
    #else /* CXX_11?  */
        std::auto_ptr<DBHandler::PreparedQuery> qu;
    #endif /* CXX_11?  */
        qu.reset (DBHandler::getInstance ()->prepareQuery (queryStr));
        // ---------------------------------------------
        qu->bind (":blah_newest_receipt_id", QVariant::fromValue(nNewestReceiptNum));
        if (nNewestKnownState > 0)
            qu->bind (":blah_newest_known_state", nNewestKnownState);
        if (!qstrEncodedMemo.isEmpty())
            qu->bind (":blah_memo", qstrEncodedMemo);
        qu->bind (":blah_timestamp", QVariant::fromValue(timestamp));
        qu->bind (":blah_agreement_id", nAgreementId);

        DBHandler::getInstance ()->runQuery (qu.release ());
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return false;
    }
//  QString str_update= QString("UPDATE `agreement` SET `have_read`=0, `newest_receipt_id`=%1,`newest_known_state`=%2,`timestamp`=%3 WHERE `agreement_id`=%4").
//          arg(nNewestReceiptNum).arg(nNewestKnownState).arg(timestamp).arg(nAgreementId);
//  qDebug() << QString("Running query: %1").arg(str_update);
//  DBHandler::getInstance()->runQuery(str_update);

    return true;
}

// ----------------------------------------------------------

// returns nAgreementReceiptKey
int MTContactHandler::DoesAgreementReceiptAlreadyExist(const int nAgreementId, const int64_t receiptNum, const QString & qstrNymId,
                                                       const int64_t transNumDisplay/*=0*/) // Last parameter only used in case of cancellation.
{
    QMutexLocker locker(&m_Mutex);

    QString str_select = QString("SELECT `agreement_receipt_key` FROM `agreement_receipt` WHERE `agreement_id`=%1 AND `receipt_id`=%2 AND `my_nym_id`='%3' LIMIT 0,1").
            arg(nAgreementId).arg(receiptNum).arg(qstrNymId);
    int nRows = DBHandler::getInstance()->querySize(str_select);

    if (0 >= nRows && transNumDisplay > 0)
    {
        // In this case, ONLY if it has already failed, and ONLY if transNumDisplay is provided,
        // then we know it's a cancelation/activation, and the receiptNum for the notice may not
        // match the receiptNum for the pending incoming payment plan or smart contract.
        // Therefore we look it up instead based on the transNumDisplay, which is passed in
        // specifically in cases where the pending incoming is being canceled by a server notice.

        str_select = QString("SELECT `agreement_receipt_key` FROM `agreement_receipt` WHERE `agreement_id`=%1 AND `txn_id_display`=%2 AND `my_nym_id`='%3' LIMIT 0,1").
                arg(nAgreementId).arg(transNumDisplay).arg(qstrNymId);
        nRows = DBHandler::getInstance()->querySize(str_select);

        if (0 >= nRows)
            return 0;
    }

    return DBHandler::getInstance()->queryInt(str_select, 0, 0);
}

bool MTContactHandler::CreateAgreementReceiptBody(const int nAgreementReceiptKey, QString & qstrReceiptBody) // When this is called, we already know the specific receipt is being added for the first time.
{
    QMutexLocker locker(&m_Mutex);
    // ----------------------------------------
    if (nAgreementReceiptKey > 0)
    {
        QString str_insert = QString("INSERT INTO `receipt_body` "
                                     "(`agreement_receipt_key`) "
                                     "VALUES(%1)").arg(nAgreementReceiptKey);
        DBHandler::getInstance()->runQuery(str_insert);
        // ----------------------------------------
        const int nReceiptKey = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `receipt_body`", 0, 0);

        if (nReceiptKey == nAgreementReceiptKey)
        {
            const bool bUpdated = LowLevelUpdateReceiptBody(nReceiptKey, qstrReceiptBody);

            if (!bUpdated)
            {
                qDebug() << QString("Failed updating receipt body for agreement_receipt_key: %1").arg(nReceiptKey);
                return false;
            }
            return true;
        }
    }
    return false;
}


bool MTContactHandler::UpdateAgreementReceiptBody(int nAgreementReceiptKey, const QString qstrBody)
{
    QMutexLocker locker(&m_Mutex);

    return LowLevelUpdateReceiptBody(nAgreementReceiptKey, qstrBody);
}


bool MTContactHandler::LowLevelUpdateReceiptBody(int nAgreementReceiptKey, const QString & qstrBody)
{
//  NOTE: This function ASSUMES that the calling function already locked the Mutex.
//  QMutexLocker locker(&m_Mutex);

    if (!qstrBody.isEmpty())
    {
        try
        {
            // The body is encrypted.
            //
            bool bSetValue = SetEncryptedValueByID(nAgreementReceiptKey, qstrBody, "body", "receipt_body", "agreement_receipt_key");
            Q_UNUSED(bSetValue);
        }
        catch (const std::exception& exc)
        {
            qDebug () << "Error: " << exc.what ();
            return false;
        }
    }
    // --------------------------
    return true;
}

bool MTContactHandler::DeleteAgreementReceiptBody(const int nID) // nID is nAgreementReceiptKey
{
    QMutexLocker locker(&m_Mutex);

    QString str_delete = QString("DELETE FROM `receipt_body` WHERE `agreement_receipt_key`=%1").arg(nID);

    return DBHandler::getInstance()->runQuery(str_delete);
}

QString MTContactHandler::GetAgreementReceiptBody(const int nID) // nID is nAgreementReceiptKey
{
    return MTContactHandler::GetEncryptedValueByID(nID, "body", "receipt_body", "agreement_receipt_key");
}

// ----------------------------------------------------------

QString MTContactHandler::GetPaymentBody(int nID)
{
    return MTContactHandler::GetEncryptedValueByID(nID, "body", "payment_body", "payment_id");
}

QString MTContactHandler::GetPaymentPendingBody(int nID)
{
    return MTContactHandler::GetEncryptedValueByID(nID, "pending_body", "payment_body", "payment_id");
}

bool MTContactHandler::UpdatePaymentBody(int nPaymentID, const QString qstrBody, const QString qstrPendingBody)
{
    QMutexLocker locker(&m_Mutex);

    return LowLevelUpdatePaymentBody(nPaymentID, qstrBody, qstrPendingBody);
}

bool MTContactHandler::DeletePaymentBody(int nID)
{
    QMutexLocker locker(&m_Mutex);

    QString str_delete = QString("DELETE FROM `payment_body` WHERE `payment_id`=%1").arg(nID);

    return DBHandler::getInstance()->runQuery(str_delete);
}

// Returns 0 if not found.
//
int MTContactHandler::GetPaymentIdByTxnDisplayId(int64_t lTxnDisplayId, QString qstrNymId)
{
    QMutexLocker locker(&m_Mutex);

    QString str_select = QString("SELECT `payment_id` FROM `payment` WHERE `txn_id_display`=%1 AND `my_nym_id`='%2' LIMIT 0,1").arg(lTxnDisplayId).arg(qstrNymId);
    int nRows = DBHandler::getInstance()->querySize(str_select);

    if (0 >= nRows)
        return 0;

    return DBHandler::getInstance()->queryInt(str_select, 0, 0);
}

// Since there is a payment table, the payment_id is already pre-existing by the time
// the entry is added to the payment_body table. (FYI.) This function assumes that
// the payment was JUST added, and that the body we're about to add corresponds to that
// same payment. So we simply look up the payment_id for the last row inserted and then
// add the body to the payment_body table using that same ID.
//
bool MTContactHandler::CreatePaymentBody(QString qstrBody, QString qstrPendingBody)
{
    QMutexLocker locker(&m_Mutex);

    const int nID = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `payment`", 0, 0);
    // ----------------------------------------
    if (nID > 0)
    {
        QString str_insert = QString("INSERT INTO `payment_body` "
                                     "(`payment_id`) "
                                     "VALUES(%1)").arg(nID);
        DBHandler::getInstance()->runQuery(str_insert);
        // ----------------------------------------
        const int nPaymentID = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `payment_body`", 0, 0);

        if (nPaymentID == nID)
        {
            bool bUpdated = LowLevelUpdatePaymentBody(nPaymentID, qstrBody, qstrPendingBody);

            if (!bUpdated)
            {
                qDebug() << QString("Failed updating payment body for payment_id: %1").arg(nPaymentID);
                return false;
            }
            return true;
        }
    }
    return false;
}

bool MTContactHandler::LowLevelUpdatePaymentBody(int nPaymentID, const QString qstrBody, const QString qstrPendingBody)
{
//  NOTE: This function ASSUMES that the calling function already locked the Mutex.
//  QMutexLocker locker(&m_Mutex);

    if (!qstrBody.isEmpty())
    {
        try
        {
            // The body is encrypted.
            //
            bool bSetValue = SetEncryptedValueByID(nPaymentID, qstrBody, "body", "payment_body", "payment_id");
            Q_UNUSED(bSetValue);
        }
        catch (const std::exception& exc)
        {
            qDebug () << "Error: " << exc.what ();
            return false;
        }
    }
    // --------------------------
    if (!qstrPendingBody.isEmpty())
    {
        try
        {
            // The pending body is encrypted.
            //
            bool bSetValue = SetEncryptedValueByID(nPaymentID, qstrPendingBody, "pending_body", "payment_body", "payment_id");
            Q_UNUSED(bSetValue);
        }
        catch (const std::exception& exc)
        {
            qDebug () << "Error: " << exc.what ();
            return false;
        }
    }
    return true;
}


// ----------------------------------------------------------

// The contact ID (unlike all the other IDs) is an int instead of a string.
// Therefore we just convert it to a string and return it in a map in the same
// format as all the others.
// (FYI.)
//
bool MTContactHandler::GetContacts(mapIDName & theMap)
{
    QMutexLocker locker(&m_Mutex);

    QString str_select = QString("SELECT * FROM contact");

    bool bFoundAny = false;
    int  nRows     = DBHandler::getInstance()->querySize(str_select);

    for(int ii=0; ii < nRows; ii++)
    {
        int     contact_id   = DBHandler::getInstance()->queryInt   (str_select, 0, ii);
        QString contact_name = DBHandler::getInstance()->queryString(str_select, 1, ii);

        if (contact_id > 0)
        {
            bFoundAny = true;

            QString str_contact_id;
            str_contact_id = QString("%1").arg(contact_id);

            if (!contact_name.isEmpty())
            {
//              qDebug() << QString("About to decode name: %1").arg(contact_name);

                //Decode base64.
                contact_name = Decode(contact_name);
            }
            // --------------------------------------------------
            // At this point we have the contact ID (in string form) *and* the contact name.
            // So we can add them to our map...
            theMap.insert(str_contact_id, contact_name);
        }
    }
    // ---------------------------------------------------------------------
    return bFoundAny;
}

// ---------------------------------------------------------------------

bool MTContactHandler::GetPaymentCodes(mapIDName & theMap, int nFilterByContact)
{
    QMutexLocker locker(&m_Mutex);

    QString str_select = QString("SELECT * FROM `nym` WHERE `contact_id`=%1").arg(nFilterByContact);
//  QString str_select = QString("SELECT * FROM `nym` WHERE `contact_id`=%1 LIMIT 0,1").arg(nFilterByContact);

    bool bFoundAny = false;
    int  nRows     = DBHandler::getInstance()->querySize(str_select);

    for (int ii=0; ii < nRows; ii++)
    {
        QString nym_id       = DBHandler::getInstance()->queryString(str_select, 0, ii);
        QString nym_name     = DBHandler::getInstance()->queryString(str_select, 2, ii);
        QString payment_code = DBHandler::getInstance()->queryString(str_select, 3, ii);

        if (!nym_id.isEmpty() && !payment_code.isEmpty())
        {
            bFoundAny = true;

            if (!nym_name.isEmpty())
            {
                //Decode base64.
//                nym_name = Decode(nym_name);
            }
            else
            {
                MTNameLookupQT theLookup;
                nym_name = QString::fromStdString(theLookup.GetNymName(nym_id.toStdString(), ""));
            }
            // ----------------------------
            // At this point we have the payment code *and* the nym name.
            // So we can add them to our map...
            theMap.insert(payment_code, nym_name);
        }
    }
    // ---------------------------------------------------------------------
    return bFoundAny;
}

bool MTContactHandler::GetNyms(mapIDName & theMap, int nFilterByContact)
{
    QMutexLocker locker(&m_Mutex);

    QString str_select = QString("SELECT * FROM `nym` WHERE `contact_id`=%1").arg(nFilterByContact);
//  QString str_select = QString("SELECT * FROM `nym` WHERE `contact_id`=%1 LIMIT 0,1").arg(nFilterByContact);

    bool bFoundAny = false;
    int  nRows     = DBHandler::getInstance()->querySize(str_select);

    for (int ii=0; ii < nRows; ii++)
    {
        QString nym_id   = DBHandler::getInstance()->queryString(str_select, 0, ii);
        QString nym_name = DBHandler::getInstance()->queryString(str_select, 2, ii);

        if (!nym_id.isEmpty())
        {
            bFoundAny = true;

            if (!nym_name.isEmpty())
            {
//                qDebug() << QString("About to decode name: %1").arg(nym_name);

                //Decode base64.
//              nym_name = Decode(nym_name);
            }
            else
            {
                MTNameLookupQT theLookup;
                nym_name = QString::fromStdString(theLookup.GetNymName(nym_id.toStdString(), ""));
            }
            // ----------------------------
            // At this point we have the nym ID *and* the nym name.
            // So we can add them to our map...
            theMap.insert(nym_id, nym_name);
        }
    }
    // ---------------------------------------------------------------------
    return bFoundAny;
}

// ---------------------------------------------------------------------

bool MTContactHandler::GetAccounts(mapIDName & theMap, QString filterByNym, QString filterByServer, QString filterByAsset)
{
    QMutexLocker locker(&m_Mutex);
    // -------------------------
    mapIDName parameters;
    // -------------------------
    if (!filterByNym.isEmpty())
        parameters.insert("nym_id", filterByNym);
    // -------------------------
    if (!filterByServer.isEmpty())
        parameters.insert("notary_id", filterByServer);
    // -------------------------
    if (!filterByAsset.isEmpty())
        parameters.insert("asset_id", filterByAsset);
    // -------------------------
    // Construct the WHERE clause.
    //
    QString strParams;

    int nIteration = 0;
    while (!parameters.empty())
    {
        nIteration++; // 1 on first iteration.
        // ----------------------------------
        mapIDName::iterator the_beginning = parameters.begin();

        if (parameters.end() == the_beginning)
            break; // Done.
        // ----------------------------------
        QString strKey   = the_beginning.key();
        QString strValue = the_beginning.value();
        // ----------------------------------
        if (1 == nIteration) // first iteration
        {
            strParams = QString(" WHERE `%1`='%2'").arg(strKey).arg(strValue);
        }
        else // subsequent iterations.
        {
            strParams += QString(" AND `%1`='%2'").arg(strKey).arg(strValue);
        }
        // ----------------------------------
        parameters.remove(strKey);
    } // while
    // ---------------------------------
    // Construct the SELECT statement and append the WHERE clause.
    //
    QString str_select = QString("SELECT * FROM `nym_account`");

    if (!strParams.isEmpty())
        str_select += strParams;
    // ---------------------------------
    bool bFoundAccounts = false;
    int  nRows = DBHandler::getInstance()->querySize(str_select);

    for(int ii=0; ii < nRows; ii++)
    {
        QString account_id     = DBHandler::getInstance()->queryString(str_select, 0, ii);
        QString account_nym_id = DBHandler::getInstance()->queryString(str_select, 2, ii);
        QString display_name   = DBHandler::getInstance()->queryString(str_select, 4, ii);

        if (!display_name.isEmpty())
        {
//          qDebug() << QString("About to decode name: %1").arg(display_name);

            //Decode base64.
            display_name = Decode(display_name);
        }
        //---------------------------------------------------
        if (!account_id.isEmpty()) // Account ID is present.
        {
            if (display_name.isEmpty()) // Display name isn't.
            {
                // Look up the display name for the contact who owns the Nym who owns the Acct.
                //
                if (!account_nym_id.isEmpty())
                {
                    int nContactID = this->FindContactIDByNymID(account_nym_id);

                    if (nContactID > 0)
                    {
                        display_name = this->GetContactName(nContactID);
                    }
                }
            }
            // ----------------------
            if (display_name.isEmpty()) // Display name isn't.
            {
                display_name = account_id;
            }
            // ----------------------
            bFoundAccounts = true;

            theMap.insert(account_id, display_name);
        }
    } // for
    // ---------------------------------
    return bFoundAccounts;
}

int MTContactHandler::CreateManagedPassphrase(const QString & qstrTitle, const QString & qstrUsername, const opentxs::OTPassword & thePassphrase,
                                              const QString & qstrURL,   const QString & qstrNotes)
{
    QMutexLocker locker(&m_Mutex);

    QString str_insert = QString("INSERT INTO `managed_passphrase` "
                                 "(`passphrase_id`) "
                                 "VALUES(NULL)");

    DBHandler::getInstance()->runQuery(str_insert);
    // ----------------------------------------
    int nPassphraseID = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `managed_passphrase`", 0, 0);

    if (nPassphraseID > 0)
    {
        bool bUpdated = LowLevelUpdateManagedPassphrase(nPassphraseID, qstrTitle, qstrUsername, thePassphrase, qstrURL,  qstrNotes);

        if (!bUpdated)
            qDebug() << QString("Failed updating passphrase entry: %1").arg(nPassphraseID);
    }

    return nPassphraseID;
}


bool MTContactHandler::SetPaymentFlags(int nPaymentID, qint64 nFlags)
{
    QMutexLocker locker(&m_Mutex);

    qint64 & storedFlags = nFlags;

    try
    {
        QString queryStr("UPDATE `payment`"
                         " SET `flags` = :strdflags"
                         " WHERE `payment_id` = :pymntid");
    #ifdef CXX_11
        std::unique_ptr<DBHandler::PreparedQuery> qu;
    #else /* CXX_11?  */
        std::auto_ptr<DBHandler::PreparedQuery> qu;
    #endif /* CXX_11?  */
        qu.reset (DBHandler::getInstance ()->prepareQuery (queryStr));
        // ---------------------------------------------
        qu->bind (":strdflags", storedFlags);
        qu->bind (":pymntid", nPaymentID);

        DBHandler::getInstance ()->runQuery (qu.release ());
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return false;
    }

    return true;
}




bool MTContactHandler::UpdateAgreementReceiptRecord(int nAgreementReceiptID, QMap<QString, QVariant>& mapFinalValues)
{
    QMutexLocker locker(&m_Mutex);

    try
    {
        QString queryDetails("");
        int nValuesAddedToQuery = 0;

        for (QMap<QString, QVariant>::iterator it_map = mapFinalValues.begin();
             it_map != mapFinalValues.end(); ++it_map)
        {
            const QString  & qstrKey = it_map.key();
            // If this isn't the first value being added to the query, then add a comma first.
            if (nValuesAddedToQuery++ > 0) queryDetails += QString(", ");
            queryDetails += QString("`%1` = :key_%2").arg(qstrKey).arg(qstrKey);
        }
        // ---------------------------------------------
        QString queryStr = QString("UPDATE `agreement_receipt`"
                                   " SET %1"
                                   " WHERE `agreement_receipt_key` = :agrectid").arg(queryDetails);
    #ifdef CXX_11
        std::unique_ptr<DBHandler::PreparedQuery> qu;
    #else /* CXX_11?  */
        std::auto_ptr<DBHandler::PreparedQuery> qu;
    #endif /* CXX_11?  */
        qu.reset (DBHandler::getInstance ()->prepareQuery (queryStr));
        // ---------------------------------------------
        // Now we bind all the values.
        //
        for (QMap<QString, QVariant>::iterator it_map = mapFinalValues.begin();
             it_map != mapFinalValues.end(); ++it_map)
        {
            const QString  & qstrKey = it_map.key();
            const QVariant & qValue  = it_map.value();

            const QString qstrSqlSubstitute = QString(":key_%1").arg(qstrKey);

//            bool bIsInt = qValue.type() == QVariant::Int;
//
//            if (bIsInt)
//                qDebug() << "BINDING: KEY: " << qstrKey << " VALUE: " << qValue.toInt();
//            else
//                qDebug() << "BINDING: KEY: " << qstrKey << " VALUE: " << qValue.toString();

            qu->bind (qstrSqlSubstitute, qValue);
        }
        // ---------------------------------------------
        qu->bind (":agrectid", nAgreementReceiptID);

        DBHandler::getInstance ()->runQuery (qu.release ());
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return false;
    }

    return true;
}

// This function assumes that any data needing to be encrypted and/or encoded,
// has already been so-transformed before passed to this function.
//
bool MTContactHandler::UpdatePaymentRecord(int nPaymentID, QMap<QString, QVariant>& mapFinalValues)
{
    QMutexLocker locker(&m_Mutex);

    try
    {
        QString queryDetails("");
        int nValuesAddedToQuery = 0;

        for (QMap<QString, QVariant>::iterator it_map = mapFinalValues.begin();
             it_map != mapFinalValues.end(); ++it_map)
        {
            const QString  & qstrKey = it_map.key();
            // If this isn't the first value being added to the query, then add a comma first.
            if (nValuesAddedToQuery++ > 0) queryDetails += QString(", ");
            queryDetails += QString("`%1` = :key_%2").arg(qstrKey).arg(qstrKey);
        }
        // ---------------------------------------------
        QString queryStr = QString("UPDATE `payment`"
                                   " SET %1"
                                   " WHERE `payment_id` = :pymntid").arg(queryDetails);
    #ifdef CXX_11
        std::unique_ptr<DBHandler::PreparedQuery> qu;
    #else /* CXX_11?  */
        std::auto_ptr<DBHandler::PreparedQuery> qu;
    #endif /* CXX_11?  */
        qu.reset (DBHandler::getInstance ()->prepareQuery (queryStr));
        // ---------------------------------------------
        // Now we bind all the values.
        //
        for (QMap<QString, QVariant>::iterator it_map = mapFinalValues.begin();
             it_map != mapFinalValues.end(); ++it_map)
        {
            const QString  & qstrKey = it_map.key();
            const QVariant & qValue  = it_map.value();

            const QString qstrSqlSubstitute = QString(":key_%1").arg(qstrKey);

//            bool bIsInt = qValue.type() == QVariant::Int;
//
//            if (bIsInt)
//                qDebug() << "BINDING: KEY: " << qstrKey << " VALUE: " << qValue.toInt();
//            else
//                qDebug() << "BINDING: KEY: " << qstrKey << " VALUE: " << qValue.toString();

            qu->bind (qstrSqlSubstitute, qValue);
        }
        // ---------------------------------------------
        qu->bind (":pymntid", nPaymentID);

        DBHandler::getInstance ()->runQuery (qu.release ());
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return false;
    }

    return true;
}

bool MTContactHandler::LowLevelUpdateManagedPassphrase(int nPassphraseID,
                                                       const QString & qstrTitle, const QString & qstrUsername, const opentxs::OTPassword & thePassphrase,
                                                       const QString & qstrURL,   const QString & qstrNotes)
{
//  NOTE: This function ASSUMES that the calling function already locked the Mutex.
//  QMutexLocker locker(&m_Mutex);

    try
    {
        QString queryStr = "UPDATE `managed_passphrase`"
                           "  SET `passphrase_title` = :passtitle,`passphrase_username` = :passusername,`passphrase_url` = :passurl"
                           "  WHERE `passphrase_id` = :passid";
    #ifdef CXX_11
        std::unique_ptr<DBHandler::PreparedQuery> qu;
    #else /* CXX_11?  */
        std::auto_ptr<DBHandler::PreparedQuery> qu;
    #endif /* CXX_11?  */
        qu.reset (DBHandler::getInstance ()->prepareQuery (queryStr));
        qu->bind (":passid", nPassphraseID);
        qu->bind (":passtitle", qstrTitle);
        qu->bind (":passusername", qstrUsername);
        qu->bind (":passurl", qstrURL);
        DBHandler::getInstance ()->runQuery (qu.release ());
        // ----------------------------------
        // Set the passphrase itself (encrypted.)
        //
        bool bSetValue = SetEncryptedValueByID(nPassphraseID, QString::fromUtf8(thePassphrase.getPassword()),
                                               "passphrase_passphrase", "managed_passphrase", "passphrase_id");
        bSetValue = SetEncryptedValueByID(nPassphraseID, qstrNotes,
                                          "passphrase_notes", "managed_passphrase", "passphrase_id");
        Q_UNUSED(bSetValue);
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return false;
    }

    return true;
}

bool MTContactHandler::UpdateManagedPassphrase(int nPassphraseID,
                                               const QString & qstrTitle, const QString & qstrUsername, const opentxs::OTPassword & thePassphrase,
                                               const QString & qstrURL,   const QString & qstrNotes)
{
    QMutexLocker locker(&m_Mutex);

    return LowLevelUpdateManagedPassphrase(nPassphraseID, qstrTitle, qstrUsername, thePassphrase, qstrURL,  qstrNotes);
}

bool MTContactHandler::GetManagedPassphrase(int nPassphraseID,
                                            QString & qstrTitle, QString & qstrUsername, opentxs::OTPassword & thePassphrase,
                                            QString & qstrURL,   QString & qstrNotes)
{
    QMutexLocker locker(&m_Mutex);

    try
    {
        DBHandler & db = *DBHandler::getInstance();

        QString queryStr = "SELECT `passphrase_title`, `passphrase_username`, `passphrase_url` FROM `managed_passphrase`"
                           "  WHERE `passphrase_id` = :passid";
#ifdef CXX_11
        std::unique_ptr<DBHandler::PreparedQuery> qu;
#else /* CXX_11?  */
        std::auto_ptr<DBHandler::PreparedQuery> qu;
#endif /* CXX_11?  */
        qu.reset (db.prepareQuery (queryStr));
        qu->bind (":passid", nPassphraseID);
        QSqlRecord rec = db.queryOne (qu.release ());

        qstrTitle    = rec.field ("passphrase_title")   .value ().toString ();
        qstrUsername = rec.field ("passphrase_username").value ().toString ();
        qstrURL      = rec.field ("passphrase_url")     .value ().toString ();
        // ----------------------------------------
        QString qstrPassphrase = GetEncryptedValueByID(nPassphraseID, "passphrase_passphrase", "managed_passphrase", "passphrase_id");
        thePassphrase.setPassword(qstrPassphrase.toUtf8(), qstrPassphrase.length());
        // ----------------------------------------
        qstrNotes = GetEncryptedValueByID(nPassphraseID, "passphrase_notes", "managed_passphrase", "passphrase_id");
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
        return false;
    }
    // ----------------------------------------
    return true;
}

class ManagedPassphraseFunctor
{
private:
    mapIDName & m_mapTitle;
    mapIDName & m_mapURL;
    bool      & m_bFound;

    // No default constructor.
    ManagedPassphraseFunctor();

public:
    inline
    ManagedPassphraseFunctor(mapIDName & mapTitle, mapIDName & mapURL, bool & bFound)
      : m_mapTitle(mapTitle), m_mapURL(mapURL), m_bFound(bFound)
    {}

    void operator() (const QSqlRecord& rec)
    {
        m_bFound = true;

        const int     nPassphraseID = rec.field ("passphrase_id")   .value().toInt();
        const QString qstrTitle     = rec.field ("passphrase_title").value().toString();
        const QString qstrURL       = rec.field ("passphrase_url")  .value().toString();
        // -------------------------------------------------------------
        const QString qstrID        = QString("%1").arg(nPassphraseID);
        // -------------------------------------------------------------
        m_mapTitle.insert(qstrID, qstrTitle);
        m_mapURL  .insert(qstrID, qstrURL);
    }
};


bool MTContactHandler::GetManagedPassphrases(mapIDName & mapTitle, mapIDName & mapURL, QString searchStr/*=""*/)
{
    // Get ALL managed passphrases, unless searchStr is provided, in which case use that to filter the results.
    // Return TWO mapIDNames. One is passphraseID mapped to Title, and the other is passphraseID mapped to URL.
    //
    QMutexLocker locker(&m_Mutex);
    // ----------------------------
    bool bFound = false, bSearchStringExists = !searchStr.isEmpty();
    DBHandler& db = *DBHandler::getInstance ();
    // ----------------------------
    // This is a hack I'm doing to sanitize searchStr since the way it's SUPPOSED to work
    // (named bound values) isn't working at all.
    //
    QSqlField sqlField;

    if (bSearchStringExists)
    {
        sqlField.setType(QVariant::String);
        sqlField.setValue(searchStr);

        searchStr = db.formatValue(sqlField);
        searchStr.remove(QChar('\''));
        searchStr.remove(QChar('\"'));
    }
    // ----------------------------
    ManagedPassphraseFunctor passphraseHandler(mapTitle, mapURL, bFound);
    QString queryStr;

    if (bSearchStringExists)
        queryStr = QString("SELECT `passphrase_id`,`passphrase_title`,`passphrase_url` FROM `managed_passphrase` "
                             "WHERE `passphrase_title` LIKE '%1%2%1' "
                             "OR `passphrase_username` LIKE '%1%2%1' "
                             "OR `passphrase_url` LIKE '%1%2%1' "
                             ).arg("%").arg(searchStr);
    else
        queryStr = QString("SELECT `passphrase_id`,`passphrase_title`,`passphrase_url` FROM `managed_passphrase` ");

  #ifdef CXX_11
    std::unique_ptr<DBHandler::PreparedQuery> qu;
  #else /* CXX_11?  */
    std::auto_ptr<DBHandler::PreparedQuery> qu;
  #endif /* CXX_11?  */
    qu.reset (db.prepareQuery (queryStr));

    try
    {
        db.queryMultiple (qu.release(), passphraseHandler);
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
    }

    return bFound;
}

int  MTContactHandler::CreateSmartContractTemplate(QString template_string)
{
    QMutexLocker locker(&m_Mutex);

    QString str_insert = QString("INSERT INTO `smart_contract` "
                             "(`template_id`) "
                             "VALUES(NULL)");

    //qDebug() << QString("Running query: %1").arg(str_insert);

    DBHandler::getInstance()->runQuery(str_insert);
    // ----------------------------------------
    int nTemplateID = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `smart_contract`", 0, 0);

    if (nTemplateID > 0)
    {
        QString encoded_value = Encode(template_string);

        // TODO: CHANGE THIS to use bind (there are examples in this file) so we aren't literally copying the full
        // smart contract directly into this update statement!
        //
        str_insert = QString("UPDATE smart_contract SET template_contents='%1' WHERE template_id='%2'").arg(encoded_value).arg(nTemplateID);

        //qDebug() << QString("Running query: %1").arg(str_insert);

        DBHandler::getInstance()->runQuery(str_insert);
    }

    return nTemplateID;
}

// Notice there is no "CreateContactBasedOnAcct" because you can call this first,
// and then just call FindContactIDByAcctID.
//
int MTContactHandler::CreateContactBasedOnNym(QString nym_id_string, QString notary_id_string/*=QString("")*/, QString payment_code/*=QString("")*/)
{
    QMutexLocker locker(&m_Mutex);

    // First, see if a contact already exists for this Nym, and if so,
    // save its ID and return at the bottom.
    //
    // If no contact exists for this Nym, then create the contact and Nym.
    // (And save the contact ID, and return at the bottom.)
    //
    // Finally, do the same actions found in NotifyOfNymServerPair, only if notary_id_string
    // !isEmpty(), to make sure we record the server as well, when appropriate.
    //
    // -----------------------------------------------------------------------
    int nContactID = 0;

    // First, see if a contact already exists for this Nym, and if so,
    // save its ID and return at the bottom.
    //
    QString str_select = QString("SELECT `contact_id` FROM `nym` WHERE `nym_id`='%1'").arg(nym_id_string);

//    qDebug() << QString("Running query: %1").arg(str_select);

    int  nRows      = DBHandler::getInstance()->querySize(str_select);
    bool bNymExists = false;

    for(int ii=0; ii < nRows; ii++)
    {
        nContactID = DBHandler::getInstance()->queryInt(str_select, 0, ii);

        bNymExists = true; // Whether the contact ID was good or not, the Nym itself DOES exist.

        break; // In practice there should only be one row.
    }
    // ---------------------------------------------------------------------
    // If no contact exists for this Nym, then create the contact and Nym.
    // (And save the contact ID, and return at the bottom.)
    //
    bool bHadToCreateContact = false;
    if (!(nContactID > 0))
    {
        bHadToCreateContact = true;
        QString str_insert_contact = QString("INSERT INTO `contact` "
                                             "(`contact_id`) "
                                             "VALUES(NULL)");

        //qDebug() << QString("Running query: %1").arg(str_insert_contact);

        DBHandler::getInstance()->runQuery(str_insert_contact);
        // ----------------------------------------
        nContactID = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `contact`", 0, 0);
    }
    // ---------------------------------------------------------------------
    // Here we create or update the Nym...
    //
    if (nContactID > 0)
    {
        QString str_insert_nym;

        // todo: add "upsert" code to consolidate to a single sql statement, if possible.

        if (!bNymExists)
            str_insert_nym = QString("INSERT INTO `nym` "
                                     "(`nym_id`, `contact_id`, `nym_payment_code`) "
                                     "VALUES('%1', %2, '%3')").arg(nym_id_string).arg(nContactID).arg(payment_code);
        else if (!payment_code.isEmpty())
            str_insert_nym = QString("UPDATE `nym` SET `contact_id`=%1,`nym_payment_code`='%2' WHERE `nym_id`='%3'")
                    .arg(nContactID).arg(payment_code).arg(nym_id_string);
        else
            str_insert_nym = QString("UPDATE `nym` SET `contact_id`=%1 WHERE `nym_id`='%2'").arg(nContactID).arg(nym_id_string);

        if (!str_insert_nym.isEmpty())
        {
            //qDebug() << QString("Running query: %1").arg(str_insert_nym);

            DBHandler::getInstance()->runQuery(str_insert_nym);
        }
    }
    // ---------------------------------------------------------------------
    // Finally, do the same actions found in NotifyOfNymServerPair, only if notary_id_string
    // !isEmpty(), to make sure we record the server as well, when appropriate.
    //
    if (!notary_id_string.isEmpty())
    {
        QString str_select_server = QString("SELECT `notary_id` FROM `nym_server` WHERE `nym_id`='%1' AND `notary_id`='%2' LIMIT 0,1").arg(nym_id_string).arg(notary_id_string);
        int nRowsServer = DBHandler::getInstance()->querySize(str_select_server);

        if (0 == nRowsServer) // It wasn't already there. (Add it.)
        {
            QString str_insert_server = QString("INSERT INTO `nym_server` "
                                                "(`nym_id`, `notary_id`) "
                                                "VALUES('%1', '%2')").arg(nym_id_string).arg(notary_id_string);

            //qDebug() << QString("Running query: %1").arg(str_insert_server);

            DBHandler::getInstance()->runQuery(str_insert_server);
        }
    }
    // ---------------------------------------------------------------------
    return nContactID;
}





int MTContactHandler::CreateContactBasedOnAddress(QString qstrAddress, QString qstrMethodType)
{
    QMutexLocker locker(&m_Mutex);

    QString qstrEncodedAddress = Encode(qstrAddress);
    QString qstrEncodedMethodType = Encode(qstrMethodType);

    // First, see if a contact already exists for this Address, and if so,
    // save its ID and return at the bottom.
    //
    // If no contact exists for this Address, then create the contact and Address.
    // (And save the contact ID, and return at the bottom.)
    //
    // -----------------------------------------------------------------------
    int nContactID = 0;

    // First, see if a contact already exists for this Address, and if so,
    // save its ID and return at the bottom.
    //
    QString str_select = QString("SELECT `contact_id` FROM `contact_method` WHERE `address`='%1'").arg(qstrEncodedAddress);

    int  nRows      = DBHandler::getInstance()->querySize(str_select);
    bool bAddressExists = false;

    for(int ii=0; ii < nRows; ii++)
    {
        nContactID = DBHandler::getInstance()->queryInt(str_select, 0, ii);

        bAddressExists = true; // Whether the contact ID was good or not, the Address itself DOES exist.

        break; // In practice there should only be one row.
    }
    // ---------------------------------------------------------------------
    // If no contact exists for this Address, then create the contact and Address.
    // (And save the contact ID, and return at the bottom.)
    //
    bool bHadToCreateContact = false;
    if (!(nContactID > 0))
    {
        bHadToCreateContact = true;
        QString str_insert_contact = QString("INSERT INTO `contact` "
                                             "(`contact_id`) "
                                             "VALUES(NULL)");

        //qDebug() << QString("Running query: %1").arg(str_insert_contact);

        DBHandler::getInstance()->runQuery(str_insert_contact);
        // ----------------------------------------
        nContactID = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `contact`", 0, 0);
    }
    // ---------------------------------------------------------------------
    // Here we create or update the Address...
    //
    if (nContactID > 0)
    {
        QString str_insert_addr;

        if (!bAddressExists)
            str_insert_addr = QString("INSERT INTO `contact_method` "
                                     "(`contact_id`, `method_type`, `address`) "
                                     "VALUES(%1, '%2', '%3')").arg(nContactID).arg(qstrEncodedMethodType).arg(qstrEncodedAddress);
        else
            str_insert_addr = QString("UPDATE contact_method SET contact_id=%1 WHERE address='%2'").arg(nContactID).arg(qstrEncodedAddress);

        if (!str_insert_addr.isEmpty())
        {
            DBHandler::getInstance()->runQuery(str_insert_addr);
        }
    }
    // ---------------------------------------------------------------------
    return nContactID;
}




/*
int MTContactHandler::FindOrCreateContactByNym(QString nym_id_string)
{
    int nContactID = this->FindContactIDByNymID(nym_id_string);

    if (nContactID > 0)
        return nContactID; // It already exists.
    // ----------------------------------------------
    "INSERT INTO `address_book` (`id`, `nym_id`, `nym_display_name`) VALUES(NULL, '%1', '%2')"
}
// NOTE: above might be unnecessary. See below comment.
*/

// Either you create a new Contact, or you use a pre-existing Contact.
//
// If you choose to add to a pre-existing Contact, then you have already
// selected him from a list, and you have his contact ID already. IN THAT
// CASE, you know the ContactID, and you have either a Nym, or a Nym/Server
// pair, or an Account (including nym, server, and asset) to add to that
// contact.
//
// Whereas if you choose to create a new Contact, you must choose to enter
// his name and then you get his new contact ID (the same as if you had
// selected him from a list) and then you can do the above, as normal.
//
// Therefore either way, we will know the Contact ID in advance, if we are
// DEFINITELY adding the data. And if we do NOT know the contact ID, that
// means we ONLY want to add the data in the case where it is found indirectly.
// (Otherwise we don't want to add it.)
//
// To some degree, we can figure out certain things automatically. For example,
// Let's say I have a receipt with some new account number on it. I might not
// have any record of that account in my contacts list. HOWEVER, the account
// might be owned by a Nym who IS in my contacts list, in which case I should
// be smart enough to add that account to the contact for that same Nym.
//
// Let's say the account isn't found in there at all, even indirectly. The function
// would then return a failure, which should be signal enough to the caller,
// to ask the user if he wants to add it to his address book. But that's not our
// place to do -- the caller will add it to his address book if he wishes. Until
// then we either just return a failure, or if we can find it indirectly, we add
// it to our records.

QString MTContactHandler::GetValueByIDLowLevel(QString str_select)
{
    QMutexLocker locker(&m_Mutex);

    int nRows = DBHandler::getInstance()->querySize(str_select);

    for(int ii=0; ii < nRows; ii++)
    {
        //Extract data
        QString qstr_value = DBHandler::getInstance()->queryString(str_select, 0, ii);

        if (!qstr_value.isEmpty())
        {
            //Decode base64.
            qstr_value = Decode(qstr_value);
        }
        //---------------------------------------------------
        return qstr_value; // In practice there should only be one row.
    }

    return ""; // Didn't find any.
}


// Warning: this call only works after OT LoadWallet is finished.
// (Because it uses keys from the wallet.)
QString MTContactHandler::GetEncryptedValueByIDLowLevel(QString str_select)
{
    QMutexLocker locker(&m_Mutex);

    int nRows = DBHandler::getInstance()->querySize(str_select);

    for(int ii=0; ii < nRows; ii++)
    {
        //Extract data
        QString qstr_value = DBHandler::getInstance()->queryString(str_select, 0, ii);

        if (!qstr_value.isEmpty())
        {
            //Decrypt
            qstr_value = Decrypt(qstr_value);
        }
        //---------------------------------------------------
        return qstr_value; // In practice there should only be one row.
    }

    return ""; // Didn't find any.
}


// Warning: this call only works after OT LoadWallet is finished.
// (Because it uses keys from the wallet.)
QString MTContactHandler::GetEncryptedValueByID(QString qstrID, QString column, QString table, QString id_name)
{
    // NOTE: No need to lock mutex, since GetEncryptedValueByIDLowLevel already locks it.
    // -----------------------------------
    // What do the parameters mean? They are for loading generic values from tables by ID.
    //
    // For example id might be 5,
    // column might be 'nym_display_name',
    // table might be 'nym'
    // and id_name might be 'nym_id'
    // For something like:
    // QString("SELECT `nym_display_name` FROM `nym` WHERE `nym_id`='%1' LIMIT 0,1").arg(qstrNymID);
    // ----------------------------------
    QString str_select = QString("SELECT `%1` FROM `%2` WHERE `%3`='%4' LIMIT 0,1")
            .arg(column)   // "nym_display_name"
            .arg(table)    // "nym"
            .arg(id_name)  // "nym_id"
            .arg(qstrID);     // (actual Nym ID goes here as string)

    return this->GetEncryptedValueByIDLowLevel(str_select);
}

// Warning: this call only works after OT LoadWallet is finished.
// (Because it uses keys from the wallet.)
bool MTContactHandler::SetEncryptedValueByID(QString qstrID, QString value, QString column, QString table, QString id_name)
{
    QMutexLocker locker(&m_Mutex);

    QString encrypted_value = Encrypt(value);
    // ------------------------------------------
    QString str_update = QString("UPDATE `%1` SET `%2`='%3' WHERE `%4`='%5'")
            .arg(table)         // "contact"
            .arg(column)        // "contact_display_name"
            .arg(encrypted_value) // (encrypted bitmessage connect string (for example))
            .arg(id_name)       // "contact_id"
            .arg(qstrID);       // (actual contact ID goes here)

    return DBHandler::getInstance()->runQuery(str_update);
}


// Warning: this call only works after OT LoadWallet is finished.
// (Because it uses keys from the wallet.)
QString MTContactHandler::GetEncryptedValueByID(int nID, QString column, QString table, QString id_name)
{
    // NOTE: No need to lock mutex, since GetEncryptedValueByIDLowLevel already locks it.
    // -----------------------------------
    // What do the parameters mean? They are for loading generic values from tables by ID.
    //
    // For example id might be 5,
    // column might be 'contact_display_name',
    // table might be 'contact'
    // and id_name might be 'contact_id'
    // For something like:
    // QString("SELECT `contact_display_name` FROM `contact` WHERE `contact_id`=%1 LIMIT 0,1").arg(nContactID);
    // ----------------------------------
    QString str_select = QString("SELECT `%1` FROM `%2` WHERE `%3`=%4 LIMIT 0,1")
            .arg(column)   // "contact_display_name"
            .arg(table)    // "contact"
            .arg(id_name)  // "contact_id"
            .arg(nID);     // (actual integer ID goes here)

    return this->GetEncryptedValueByIDLowLevel(str_select);
}


// Warning: this call only works after OT LoadWallet is finished.
// (Because it uses keys from the wallet.)
bool MTContactHandler::SetEncryptedValueByID(int nID, QString value, QString column, QString table, QString id_name)
{
    QMutexLocker locker(&m_Mutex);

    QString encrypted_value = Encrypt(value);
    // ------------------------------------------
    QString str_update = QString("UPDATE `%1` SET `%2`='%3' WHERE `%4`=%5")
            .arg(table)         // "contact"
            .arg(column)        // "contact_display_name"
            .arg(encrypted_value) // (encrypted bitmessage connect string, say.)
            .arg(id_name)       // "contact_id"
            .arg(nID);          // (actual contact ID goes here)

    return DBHandler::getInstance()->runQuery(str_update);
}



QString MTContactHandler::GetValueByID(QString qstrID, QString column, QString table, QString id_name)
{
    // NOTE: No need to lock mutex, since GetValueByIDLowLevel already locks it.
    // -----------------------------------
    // What do the parameters mean? They are for loading generic values from tables by ID.
    //
    // For example id might be 5,
    // column might be 'nym_display_name',
    // table might be 'nym'
    // and id_name might be 'nym_id'
    // For something like:
    // QString("SELECT `nym_display_name` FROM `nym` WHERE `nym_id`='%1' LIMIT 0,1").arg(qstrNymID);
    // ----------------------------------
    QString str_select = QString("SELECT `%1` FROM `%2` WHERE `%3`='%4' LIMIT 0,1")
            .arg(column)   // "nym_display_name"
            .arg(table)    // "nym"
            .arg(id_name)  // "nym_id"
            .arg(qstrID);     // (actual Nym ID goes here as string)

    return this->GetValueByIDLowLevel(str_select);
}


bool MTContactHandler::SetValueByID(QString qstrID, QString value, QString column, QString table, QString id_name)
{
    QMutexLocker locker(&m_Mutex);

    QString encoded_value = Encode(value);
    // ------------------------------------------
    QString str_update = QString("UPDATE `%1` SET `%2`='%3' WHERE `%4`='%5'")
            .arg(table)         // "contact"
            .arg(column)        // "contact_display_name"
            .arg(encoded_value) // (base64-encoded display name)
            .arg(id_name)       // "contact_id"
            .arg(qstrID);       // (actual contact ID goes here)

    return DBHandler::getInstance()->runQuery(str_update);
}


QString MTContactHandler::GetValueByID(int nID, QString column, QString table, QString id_name)
{
    // NOTE: No need to lock mutex, since GetValueByIDLowLevel already locks it.
    // -----------------------------------
    // What do the parameters mean? They are for loading generic values from tables by ID.
    //
    // For example id might be 5,
    // column might be 'contact_display_name',
    // table might be 'contact'
    // and id_name might be 'contact_id'
    // For something like:
    // QString("SELECT `contact_display_name` FROM `contact` WHERE `contact_id`=%1 LIMIT 0,1").arg(nContactID);
    // ----------------------------------
    QString str_select = QString("SELECT `%1` FROM `%2` WHERE `%3`=%4 LIMIT 0,1")
            .arg(column)   // "contact_display_name"
            .arg(table)    // "contact"
            .arg(id_name)  // "contact_id"
            .arg(nID);     // (actual integer ID goes here)

    return this->GetValueByIDLowLevel(str_select);
}


bool MTContactHandler::SetValueByID(int nID, QString value, QString column, QString table, QString id_name)
{
    QMutexLocker locker(&m_Mutex);

    QString encoded_value = Encode(value);
    // ------------------------------------------
    QString str_update = QString("UPDATE `%1` SET `%2`='%3' WHERE `%4`=%5")
            .arg(table)         // "contact"
            .arg(column)        // "contact_display_name"
            .arg(encoded_value) // (base64-encoded display name)
            .arg(id_name)       // "contact_id"
            .arg(nID);          // (actual contact ID goes here)

    return DBHandler::getInstance()->runQuery(str_update);
}


QString MTContactHandler::GetContactName(int nContactID)
{
    return this->GetValueByID(nContactID, "contact_display_name", "contact", "contact_id");
}

bool MTContactHandler::SetContactName(int nContactID, QString contact_name_string)
{
    return this->SetValueByID(nContactID, contact_name_string, "contact_display_name", "contact", "contact_id");
}

// ---------------------------------------------------

//static
QString MTContactHandler::Encrypt(QString plaintext)
{
    QString encrypted_value("");

    if (!plaintext.isEmpty())
    {
        opentxs::OTWallet * pWallet = opentxs::OTAPI_Wrap::OTAPI()->GetWallet("MTContactHandler::Encrypt"); // This logs and ASSERTs already.

        if (NULL != pWallet)
        {
            opentxs::String strOutput, strPlaintext(plaintext.toStdString().c_str());

            if (pWallet->Encrypt_ByKeyID(s_key_id, strPlaintext, strOutput))
            {
                std::string str_temp(strOutput.Get());
                encrypted_value = QString::fromStdString(str_temp);
            }

        }
    }

    return encrypted_value;
}

//static
QString MTContactHandler::Decrypt(QString ciphertext)
{
//  qDebug() << QString("Decrypting ciphertext: %1").arg(ciphertext);

    QString decrypted_value("");

    if (!ciphertext.isEmpty())
    {
        opentxs::OTWallet * pWallet = opentxs::OTAPI_Wrap::OTAPI()->GetWallet("MTContactHandler::Decrypt"); // This logs and ASSERTs already.

        if (NULL != pWallet)
        {
            opentxs::String strOutput, strCiphertext(ciphertext.toStdString().c_str());

            if (pWallet->Decrypt_ByKeyID(s_key_id, strCiphertext, strOutput))
            {
                std::string str_temp(strOutput.Get());
                decrypted_value = QString::fromStdString(str_temp);
            }

        }
    }

    return decrypted_value;
}

// ---------------------------------------------------

//static
QString MTContactHandler::Encode(QString plaintext)
{
    QString encoded_value("");

    if (!plaintext.isEmpty())
    {
        // Encode base64.
        opentxs::String        strValue(plaintext.toStdString());
        opentxs::OTASCIIArmor    ascValue;
        ascValue.SetString(strValue, false); //bLineBreaks=true by default
        encoded_value = QString(ascValue.Get());
    }

    return encoded_value;
}

//static
QString MTContactHandler::Decode(QString encoded)
{
    QString decoded_value("");

    if (!encoded.isEmpty())
    {
        // Decode base64.
        opentxs::OTASCIIArmor ascValue;
        ascValue.Set(encoded.toStdString().c_str());
        opentxs::String strValue;
        ascValue.GetString(strValue, false); //bLineBreaks=true by default
        decoded_value = QString(strValue.Get());
    }

    return decoded_value;
}

bool MTContactHandler::MethodTypeFoundOnNym(QString method_type, QString filterByNym)
{
    QMutexLocker locker(&m_Mutex);

    int     nReturnValue = 0;
    QString encoded_type = Encode(method_type);

    if (!encoded_type.isEmpty())
    {
        QString str_select = QString("SELECT * "                // Select all rows...
                                     "FROM `nym_method` "       // ...from the nym_method table...
                                     "INNER JOIN `msg_method` " // ...where it matches the 'msg_method' table...
                                     "ON nym_method.method_id=msg_method.method_id " // ...on the method_id column. (So we only see methods that are attached to a nym.)
                                     "WHERE msg_method.method_type='%1' AND nym_method.nym_id='%2' LIMIT 0,1").
                             arg(encoded_type).arg(filterByNym); // ...filtered by a method_type of 'encoded_type' function parameter. (And NymID.)

        nReturnValue = DBHandler::getInstance()->querySize(str_select);
    }

    return (nReturnValue > 0);
}

bool MTContactHandler::MethodTypeFoundOnContact(QString method_type, int nFilterByContact)
{
    QMutexLocker locker(&m_Mutex);

    int     nReturnValue = 0;
    QString encoded_type = Encode(method_type);

    if (!encoded_type.isEmpty())
    {
        QString str_select = QString("SELECT * "                // Select all rows...
                                     "FROM `contact_method` "   // ...from the contact_method table...
                                     "WHERE method_type='%1' AND contact_id=%2 LIMIT 0,1").
                arg(encoded_type).arg(nFilterByContact); // ...filtered by a method_type of 'encoded_type' function parameter.

        nReturnValue = DBHandler::getInstance()->querySize(str_select);
    }

    return (nReturnValue > 0);
}

// = "CREATE TABLE nym_method(nym_id TEXT, method_id INTEGER, address TEXT, PRIMARY KEY(nym_id, method_id, address))";
//
bool MTContactHandler::MethodTypeFoundOnAnyNym(QString method_type)
{
    QMutexLocker locker(&m_Mutex);

    int     nReturnValue = 0;
    QString encoded_type = Encode(method_type);

    if (!encoded_type.isEmpty())
    {
        QString str_select = QString("SELECT * "                // Select all rows...
                                     "FROM `nym_method` "       // ...from the nym_method table...
                                     "INNER JOIN `msg_method` " // ...where it matches the 'msg_method' table...
                                     "ON nym_method.method_id=msg_method.method_id " // ...on the method_id column. (So we only see methods that are attached to a nym.)
                                     "WHERE msg_method.method_type='%1' LIMIT 0,1").arg(encoded_type); // ...filtered by a method_type of 'encoded_type' function parameter.

        nReturnValue = DBHandler::getInstance()->querySize(str_select);
    }

    return (nReturnValue > 0);
}

// = "CREATE TABLE contact_method(contact_id INTEGER, method_type TEXT, address TEXT, PRIMARY KEY(contact_id, method_id, address))";
//
bool MTContactHandler::MethodTypeFoundOnAnyContact(QString method_type)
{
    QMutexLocker locker(&m_Mutex);

    int     nReturnValue = 0;
    QString encoded_type = Encode(method_type);

    if (!encoded_type.isEmpty())
    {
        QString str_select = QString("SELECT * "                // Select all rows...
                                     "FROM `contact_method` "   // ...from the contact_method table...
                                     "WHERE method_type='%1' LIMIT 0,1").arg(encoded_type); // ...filtered by a method_type of 'encoded_type' function parameter.

        nReturnValue = DBHandler::getInstance()->querySize(str_select);
    }

    return (nReturnValue > 0);
}


bool MTContactHandler::MethodExists(int nMethodID)
{
    QMutexLocker locker(&m_Mutex);
    QString str_select = QString("SELECT * FROM `msg_method` WHERE `method_id`=%1 LIMIT 0,1").arg(nMethodID);
    int nRows = DBHandler::getInstance()->querySize(str_select);
    return (nRows > 0);
}

/*
    static bool      add(const std::string type, const std::string commstring);
    static bool      get(const std::string type, mapOfNetModules & mapOutput);
    static bool      types(std::vector<std::string> & vecTypes);
    static MTComms * it ();
*/

bool MTContactHandler::MethodTypeExists(QString method_type)
{
    const std::string str_method_type = method_type.toStdString();
    mapOfNetModules   mapOutput;

    if (MTComms::get(str_method_type, mapOutput))
        return true;

    return false;
}


//QString create_nym_method = "CREATE TABLE nym_method(nym_id TEXT, method_id INTEGER, address TEXT, PRIMARY KEY(nym_id, method_id, address))";

bool MTContactHandler::AddMsgAddressToNym(QString nym_id, int nMethodID, QString address)
{
    QMutexLocker locker(&m_Mutex);

    QString encoded_address = Encode(address);

    QString str_select = QString("SELECT `method_id` FROM `nym_method` "
                                 "WHERE `nym_id`='%1' AND `method_id`=%2 AND `address`='%3' LIMIT 0,1").
            arg(nym_id).arg(nMethodID).arg(encoded_address);

    int nRows = DBHandler::getInstance()->querySize(str_select);

    if (0 == nRows) // It wasn't already there. (Add it.)
    {
        QString str_insert = QString("INSERT INTO `nym_method` "
                                     "(`nym_id`, `method_id`, `address`) "
                                     "VALUES('%1', %2, '%3')").arg(nym_id).arg(nMethodID).arg(encoded_address);
        return DBHandler::getInstance()->runQuery(str_insert);
    }

    return false;
}

bool MTContactHandler::RemoveMsgAddressFromNym(QString nym_id, int nMethodID, QString address)
{
    QMutexLocker locker(&m_Mutex);

    QString encoded_address = Encode(address);

    QString str_delete = QString("DELETE FROM `nym_method` "
                                 "WHERE `nym_id`='%1' AND `method_id`=%2 AND `address`='%3'")
            .arg(nym_id).arg(nMethodID).arg(encoded_address);

    return (DBHandler::getInstance()->runQuery(str_delete));
}

//QString create_contact_method  = "CREATE TABLE contact_method(contact_id INTEGER, method_type TEXT, address TEXT, PRIMARY KEY(contact_id, method_id, address))";

bool MTContactHandler::AddMsgAddressToContact(int nContactID, QString qstrMethodType, QString address)
{
    QMutexLocker locker(&m_Mutex);

    QString encoded_type    = Encode(qstrMethodType);
    QString encoded_address = Encode(address);

    // See if qstrMethodType and address are already there for nContactID.
    // Like, does Contact #5 already have "bitmessage" and "lkjsdfkjdffd" in the table?
    // (If so, no need to re-add them.)
    //
    QString str_select = QString("SELECT `method_type` FROM `contact_method` "
                                 "WHERE `contact_id`=%1 AND `method_type`='%2' AND `address`='%3' LIMIT 0,1").
            arg(nContactID).arg(encoded_type).arg(encoded_address);

    int nRows = DBHandler::getInstance()->querySize(str_select);

    if (0 == nRows) // It wasn't already there. (Add it.)
    {
        QString str_insert = QString("INSERT INTO `contact_method` "
                                     "(`contact_id`, `method_type`, `address`) "
                                     "VALUES(%1, '%2', '%3')").arg(nContactID).arg(encoded_type).arg(encoded_address);
        return DBHandler::getInstance()->runQuery(str_insert);
    }

    return false;
}

bool MTContactHandler::RemoveMsgAddressFromContact(int nContactID, QString qstrMethodType, QString address)
{
    QMutexLocker locker(&m_Mutex);

    QString encoded_type    = Encode(qstrMethodType);
    QString encoded_address = Encode(address);

    QString str_delete = QString("DELETE FROM `contact_method` "
                                 "WHERE `contact_id`=%1 AND `method_type`='%2' AND `address`='%3'")
            .arg(nContactID).arg(encoded_type).arg(encoded_address);

    return (DBHandler::getInstance()->runQuery(str_delete));
}

// --------------------------------------------

QString MTContactHandler::GetMethodDisplayName(int nMethodID)
{
    return this->GetValueByID(nMethodID, "method_display_name", "msg_method", "method_id");
}

bool MTContactHandler::SetMethodDisplayName(int nMethodID, QString input)
{
    return this->SetValueByID(nMethodID, input, "method_display_name", "msg_method", "method_id");
}

QString MTContactHandler::GetMethodType(int nMethodID)
{
    return this->GetValueByID(nMethodID, "method_type", "msg_method", "method_id");
}

bool MTContactHandler::SetMethodType(int nMethodID, QString input)
{
    return this->SetValueByID(nMethodID, input, "method_type", "msg_method", "method_id");
}

QString MTContactHandler::GetMethodTypeDisplay(int nMethodID)
{
    return this->GetValueByID(nMethodID, "method_type_display", "msg_method", "method_id");
}

bool MTContactHandler::SetMethodTypeDisplay(int nMethodID, QString input)
{
    return this->SetValueByID(nMethodID, input, "method_type_display", "msg_method", "method_id");
}

QString MTContactHandler::GetMethodConnectStr (int nMethodID)
{
    return this->GetEncryptedValueByID(nMethodID, "method_connect", "msg_method", "method_id");
}

bool MTContactHandler::SetMethodConnectStr (int nMethodID, QString input)
{
    return this->SetEncryptedValueByID(nMethodID, input, "method_connect", "msg_method", "method_id");
}


QString MTContactHandler::GetMethodType(QString qstrAddress) // Get the method type based on the address.
{
    QMutexLocker locker(&m_Mutex);

    QString returnVal("");
    QString encoded_address = Encode(qstrAddress);

    QString str_select = QString("SELECT method_type "
                                 "FROM `msg_method` "
                                 "INNER JOIN `nym_method` "
                                 "ON nym_method.method_id=msg_method.method_id "
                                 "WHERE nym_method.address='%1' LIMIT 0,1").arg(encoded_address);

    int nRows = DBHandler::getInstance()->querySize(str_select);

    if (nRows > 0)
    {
        QString qstrEncType = DBHandler::getInstance()->queryString(str_select, 0, 0);
        QString qstrType    = Decode(qstrEncType);

        return qstrType;
    }
    // -------------------------------------------------------
    str_select = QString("SELECT method_type "
                         "FROM `contact_method` "
                         "WHERE contact_method.address='%1' LIMIT 0,1").arg(encoded_address);

    nRows = DBHandler::getInstance()->querySize(str_select);

    if (nRows > 0)
    {
        QString qstrEncType = DBHandler::getInstance()->queryString(str_select, 0, 0);
        QString qstrType    = Decode(qstrEncType);

        return qstrType;
    }
    // -------------------------------------------------------
    return returnVal;
}

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
// Whereas for a Bitmessage address, the ID would be:
// "bitmessage|METHOD_ID"
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

bool MTContactHandler::GetMsgMethods(mapIDName & theMap, bool bAddServers/*=false*/, QString filterByType/*=""*/)
{
    bool bFoundAny = false;

    {   // This block is here so locker will expire at the bottom of the block.
        // (Since GetServers call, below this block, also locks the mutex.)
        //
        QMutexLocker locker(&m_Mutex);

        QString str_select = QString("SELECT * FROM `msg_method`");

        int nRows = DBHandler::getInstance()->querySize(str_select);
        // -----------------------------------
        for (int ii=0; ii < nRows; ii++)
        {
            //Extract data
            int     nMethodID          = DBHandler::getInstance()->queryInt   (str_select, 0, ii);
            QString qstrEncDisplayName = DBHandler::getInstance()->queryString(str_select, 1, ii);
            QString qstrEncType        = DBHandler::getInstance()->queryString(str_select, 2, ii);
            QString qstrEncTypeDisplay = DBHandler::getInstance()->queryString(str_select, 3, ii);
//          QString qstrEncConnect     = DBHandler::getInstance()->queryString(str_select, 4, ii);
            // -----------------------------------------------------
            QString qstrDisplayName    = Decode(qstrEncDisplayName);
            QString qstrType           = Decode(qstrEncType);
            QString qstrTypeDisplay    = Decode(qstrEncTypeDisplay);
//          QString qstrConnect        = Decrypt(qstrEncConnect);
            // -----------------------------------------------------
            // If a type filter is passed in, and it doesn't match the type of this record,
            // then continue.
            if (!filterByType.isEmpty() && (0 != filterByType.compare(qstrType)))
                continue;
            // -----------------------------------------------------
            // Whereas for a Bitmessage address, the ID would be:
            // "bitmessage|METHOD_ID"
            // Display name:  "Bitmessage: Localhost"
            //
//            QString qstrID   = filterByType.isEmpty() ? QString("%1|%2").arg(qstrType).arg(nMethodID) : QString("%1").arg(nMethodID);
//            QString qstrName = filterByType.isEmpty() ? QString("%1: %2").arg(qstrTypeDisplay).arg(qstrDisplayName) : qstrDisplayName;
            QString qstrID   = QString("%1").arg(nMethodID);
            QString qstrName = qstrDisplayName;

            theMap.insert(qstrID, qstrName);

            bFoundAny = true;
        }
    }
    // --------------------------------------------
    bool bGotServers = false;

    if (bAddServers)
        bGotServers = this->GetServers(theMap, true); //bPrependOTType=false by default

    return bFoundAny || bGotServers;
}



/*
class MTComms
{
    static bool      add(const std::string type, const std::string commstring);
    static bool      get(const std::string type, mapOfNetModules & mapOutput);
    static bool      types(std::vector<std::string> & vecTypes);
    static MTComms * it ();
};
*/

bool MTContactHandler::GetMsgMethodTypes(mapIDName & theMap, bool bAddServers/*=false*/)
{
    bool bFoundAny = false;

    mapOfCommTypes mapTypes;

    if (MTComms::types(mapTypes))
    {
        for(mapOfCommTypes::iterator it = mapTypes.begin(); it != mapTypes.end(); it++)
        {
            std::string strType = it->first;
            std::string strName = it->second;

            QString    qstrType = QString::fromStdString(strType);
            QString    qstrName = QString::fromStdString(strName);

            if ((qstrType.size() > 0) && (qstrName.size() > 0))
            {
                theMap.insert(qstrType, qstrName);
                bFoundAny = true;
            }
        }
    }
    // --------------------------------------------
    bool bGotServers = false;

    if (bAddServers)
        bGotServers = this->GetServers(theMap, true); //bPrependOTType=false by default

    return bFoundAny || bGotServers;
}

// --------------------------------------------

bool MTContactHandler::GetMethodsAndAddrByNym(mapIDName & theMap, QString filterByNym)
{
    bool bFoundAny = false;
    // ----------------------
    mapIDName mapMethods;

    bool bGotMethods = this->GetMsgMethodsByNym(mapMethods, filterByNym, false, QString(""));

    if (bGotMethods)
    {
        // Loop through mapMethods and for each methodID, call GetAddressesByNym.
        // Then add those addresses, each iteration, to theMap.
        //
        for (mapIDName::iterator ii = mapMethods.begin(); ii != mapMethods.end(); ++ii)
        {
            QString qstrID        = ii.key();
            int nFilterByMethodID = 0;

            QStringList stringlist = qstrID.split("|");

            if (stringlist.size() >= 2) // Should always be 2...
            {
//              QString qstrType     = stringlist.at(0);
                QString qstrMethodID = stringlist.at(1);
                const int  nMethodID = qstrMethodID.isEmpty() ? 0 : qstrMethodID.toInt();
                // --------------------------------------
                if (nMethodID > 0)
                {
                    mapIDName mapAddresses;

                    if (this->GetMethodsAndAddrByNym(mapAddresses, filterByNym, nMethodID))
                    {
                        for (mapIDName::iterator jj = mapAddresses.begin(); jj != mapAddresses.end(); ++jj)
                        {
                            QString qstrAddress = jj.key();

                            if (!theMap.contains(qstrAddress))
                            {
                                bFoundAny = true;
                                theMap.insertMulti(qstrAddress, jj.value());
                            }
                        } // for
                    } // if GetAddressesByNym
                } // if nMethodID > 0
            }
        } // for
     } // if bGotMethods
    // ----------------------
    return bFoundAny;
}


bool MTContactHandler::GetAddressesByNym(mapIDName & theMap, QString filterByNym, QString filterByType)
{
    bool bFoundAny = false;
    // ----------------------
    mapIDName mapMethods;

    bool bGotMethods = this->GetMsgMethodsByNym(mapMethods, filterByNym, false, filterByType);

    if (bGotMethods)
    {
        // Loop through mapMethods and for each methodID, call GetAddressesByNym.
        // Then add those addresses, each iteration, to theMap.
        //
        for (mapIDName::iterator ii = mapMethods.begin(); ii != mapMethods.end(); ++ii)
        {
            QString qstrMethodID  = ii.key();
            int nFilterByMethodID = qstrMethodID.toInt();

            if (nFilterByMethodID > 0)
            {
                mapIDName mapAddresses;

                if (this->GetAddressesByNym(mapAddresses, filterByNym, nFilterByMethodID))
                {
                    for (mapIDName::iterator jj = mapAddresses.begin(); jj != mapAddresses.end(); ++jj)
                    {
                        QString qstrAddress = jj.key();

                        if (!theMap.contains(qstrAddress))
                        {
                            bFoundAny = true;
                            theMap.insertMulti(qstrAddress, jj.value());
                        }
                    } // for
                } // if GetAddressesByNym
            } // if nFilterByMethodID > 0
        } // for
     } // if bGotMethods
    // ----------------------
    return bFoundAny;
}

// --------------------------------------------

int MTContactHandler::GetMethodIDByNymAndAddress(QString filterByNym, QString qstrAddress)
{
    // = "CREATE TABLE nym_method(nym_id TEXT, method_id INTEGER, address TEXT, PRIMARY KEY(nym_id, method_id, address))";

    int nReturn = 0;

    {   // This block is here so locker will expire at the bottom of the block.
        QMutexLocker locker(&m_Mutex);

        QString str_select = QString("SELECT method_id "
                                     "FROM `nym_method` "
                                     "WHERE nym_id='%1' AND address='%2'").arg(filterByNym).arg(qstrAddress);

        int nRows = DBHandler::getInstance()->querySize(str_select);
        // -----------------------------------
        for (int ii=0; ii < nRows; ii++)
        {
            nReturn = DBHandler::getInstance()->queryInt(str_select, 0, ii);
            break; // Should only be one.
            // (You might have multiple addresses for the same NymID/MethodID,
            // but you won't have multiple MethodIDs for the same address. Thus,
            // should be only one.)
        }
    }
    // --------------------------------------------
    return nReturn;
}


bool MTContactHandler::GetMethodsAndAddrByNym(mapIDName & theMap, QString filterByNym, int filterByMethodID)
{
    // = "CREATE TABLE nym_method(nym_id TEXT, method_id INTEGER, address TEXT, PRIMARY KEY(nym_id, method_id, address))";

    bool bFoundAny = false;

    {   // This block is here so locker will expire at the bottom of the block.
        QMutexLocker locker(&m_Mutex);

        QString str_select = QString("SELECT msg_method.method_id, nym_method.address, msg_method.method_type "
                                     "FROM `nym_method` "
                                     "INNER JOIN `msg_method` "
                                     "ON nym_method.method_id=msg_method.method_id "
                                     "WHERE nym_method.nym_id='%1' AND nym_method.method_id=%2").arg(filterByNym).arg(filterByMethodID);

        int nRows = DBHandler::getInstance()->querySize(str_select);
        // -----------------------------------
        for (int ii=0; ii < nRows; ii++)
        {
            //Extract data
            int     nMethodID          = DBHandler::getInstance()->queryInt   (str_select, 0, ii);
            QString qstrEncAddress     = DBHandler::getInstance()->queryString(str_select, 1, ii);
            QString qstrEncType        = DBHandler::getInstance()->queryString(str_select, 2, ii);
//          QString qstrEncTypeDisplay = DBHandler::getInstance()->queryString(str_select, 3, ii);
            // -----------------------------------------------------
            QString qstrAddress        = Decode(qstrEncAddress);
            QString qstrType           = Decode(qstrEncType);
//          QString qstrTypeDisplay    = Decode(qstrEncTypeDisplay);
            // -----------------------------------------------------
            std::string strTypeDisplay = MTComms::displayName(qstrType.toStdString());
            QString    qstrTypeDisplay = QString::fromStdString(strTypeDisplay);

            // For a Bitmessage address, the ID would be:
            // "BITMESSAGE_ADDRESS|METHOD_ID"
            // Display name:  "Bitmessage: BITMESSAGE_ADDRESS"
            //
            QString qstrID   = QString("%1|%2").arg(qstrAddress).arg(nMethodID);
            QString qstrName = QString("%1: %2").arg(qstrTypeDisplay).arg(qstrAddress);
            // --------------------------------------------
            theMap.insertMulti(qstrID, qstrName);

            bFoundAny = true;
        }
    }
    // --------------------------------------------
    return bFoundAny;
}

bool MTContactHandler::GetAddressesByNym(mapIDName & theMap, QString filterByNym, int filterByMethodID)
{
    // = "CREATE TABLE nym_method(nym_id TEXT, method_id INTEGER, address TEXT, PRIMARY KEY(nym_id, method_id, address))";

    bool bFoundAny = false;

    {   // This block is here so locker will expire at the bottom of the block.
        QMutexLocker locker(&m_Mutex);

//      QString str_select = QString("SELECT nym_method.address, msg_method.method_type, msg_method.method_type_display "
        QString str_select = QString("SELECT nym_method.address, msg_method.method_type "
                                     "FROM `nym_method` "
                                     "INNER JOIN `msg_method` "
                                     "ON nym_method.method_id=msg_method.method_id "
                                     "WHERE nym_method.nym_id='%1' AND nym_method.method_id=%2").arg(filterByNym).arg(filterByMethodID);

        int nRows = DBHandler::getInstance()->querySize(str_select);
        // -----------------------------------
        for (int ii=0; ii < nRows; ii++)
        {
            //Extract data
            QString qstrEncAddress     = DBHandler::getInstance()->queryString(str_select, 0, ii);
            QString qstrEncType        = DBHandler::getInstance()->queryString(str_select, 1, ii);
//          QString qstrEncTypeDisplay = DBHandler::getInstance()->queryString(str_select, 2, ii);
            // -----------------------------------------------------
            QString qstrAddress        = Decode(qstrEncAddress);
            QString qstrType           = Decode(qstrEncType);
//          QString qstrTypeDisplay    = Decode(qstrEncTypeDisplay);
            // -----------------------------------------------------
            std::string strTypeDisplay = MTComms::displayName(qstrType.toStdString());
            QString    qstrTypeDisplay = QString::fromStdString(strTypeDisplay);

            // For a Bitmessage address, the ID would be:
            // "BITMESSAGE_ADDRESS"
            // Display name:  "Bitmessage: BITMESSAGE_ADDRESS"
            //
            QString qstrID   = QString("%1").arg(qstrAddress);
            QString qstrName = QString("%1: %2").arg(qstrTypeDisplay).arg(qstrAddress);
            // --------------------------------------------
            theMap.insertMulti(qstrID, qstrName);

            bFoundAny = true;
        }
    }
    // --------------------------------------------
    return bFoundAny;
}

bool MTContactHandler::GetMsgMethodTypesByNym(mapIDName & theMap, QString filterByNym, bool bAddServers/*=false*/)
{
    bool bFoundAny = false;

    {   // This block is here so locker will expire at the bottom of the block.
        QMutexLocker locker(&m_Mutex);

        QString str_select = QString("SELECT method_type, method_type_display "
                                     "FROM `msg_method` "
                                     "INNER JOIN `nym_method` "
                                     "ON nym_method.method_id=msg_method.method_id "
                                     "WHERE nym_method.nym_id='%1'").arg(filterByNym);

        int nRows = DBHandler::getInstance()->querySize(str_select);
        // -----------------------------------
        for (int ii=0; ii < nRows; ii++)
        {
            QString qstrEncType        = DBHandler::getInstance()->queryString(str_select, 0, ii);
            QString qstrEncTypeDisplay = DBHandler::getInstance()->queryString(str_select, 1, ii);
//          QString qstrEncAddress     = DBHandler::getInstance()->queryString(str_select, 2, ii);
            // -----------------------------------------------------
            QString qstrType           = Decode(qstrEncType);
            QString qstrTypeDisplay    = Decode(qstrEncTypeDisplay);
//          QString qstrAddress        = Decode(qstrEncAddress);
            // -----------------------------------------------------
            // Whereas for a Bitmessage address, the ID would be:
            // "bitmessage|METHOD_ID"
            // Display name:  "Bitmessage: Localhost"
            //
            QString qstrID   = QString("%1").arg(qstrType);
            QString qstrName = QString("%1").arg(qstrTypeDisplay);
            // --------------------------------------------
            mapIDName::iterator it = theMap.find(qstrID);

            if (theMap.end() == it) // If it's not already in there.
            {
                theMap.insertMulti(qstrID, qstrName);
                bFoundAny = true;
            }
        }
    }
    // --------------------------------------------
    bool bGotServers = false;

    if (bAddServers)
        bGotServers = this->GetServers(theMap, filterByNym, true); //bPrependOTType=false by default

    return bFoundAny || bGotServers;
}


bool MTContactHandler::GetMsgMethodsByNym(mapIDName & theMap, QString filterByNym, bool bAddServers/*=false*/, QString filterByType/*=""*/)
{
    bool bFoundAny = false;

    {   // This block is here so locker will expire at the bottom of the block.
        QMutexLocker locker(&m_Mutex);

        QString qstrTypeFilter("");

        if (!filterByType.isEmpty())
        {
            QString qstrEncType = Encode(filterByType);
            qstrTypeFilter = QString(" AND msg_method.method_type='%1'").arg(qstrEncType);
        }

        QString str_select = QString("SELECT msg_method.method_id, msg_method.method_display_name, msg_method.method_type, msg_method.method_type_display "
                                     "FROM `msg_method` "
                                     "INNER JOIN `nym_method` "
                                     "ON nym_method.method_id=msg_method.method_id "
                                     "WHERE nym_method.nym_id='%1'%2").arg(filterByNym).arg(qstrTypeFilter);

        int nRows = DBHandler::getInstance()->querySize(str_select);
        // -----------------------------------
        for (int ii=0; ii < nRows; ii++)
        {
            //Extract data
            int     nMethodID          = DBHandler::getInstance()->queryInt   (str_select, 0, ii);
            QString qstrEncDisplayName = DBHandler::getInstance()->queryString(str_select, 1, ii);
            QString qstrEncType        = DBHandler::getInstance()->queryString(str_select, 2, ii);
            QString qstrEncTypeDisplay = DBHandler::getInstance()->queryString(str_select, 3, ii);
//          QString qstrEncConnect     = DBHandler::getInstance()->queryString(str_select, 4, ii);
            // -----------------------------------------------------
            QString qstrDisplayName    = Decode(qstrEncDisplayName);
            QString qstrType           = Decode(qstrEncType);
            QString qstrTypeDisplay    = Decode(qstrEncTypeDisplay);
//          QString qstrConnect        = Decrypt(qstrEncConnect);
            // -----------------------------------------------------
            // Whereas for a Bitmessage address, the ID would be:
            // "bitmessage|METHOD_ID"
            // Display name:  "Bitmessage: Localhost"
            //
            QString qstrID;
            QString qstrName = QString("%1: %2").arg(qstrTypeDisplay).arg(qstrDisplayName);

            if (!filterByType.isEmpty())
                qstrID   = QString("%1").arg(nMethodID); // Method ID only. No need to put the type if we already filtered based on type.
            else
                qstrID   = QString("%1|%2").arg(qstrType).arg(nMethodID); // Here we put the type and method. Caller can split the string.
            // --------------------------------------------
            theMap.insertMulti(qstrID, qstrName);

            bFoundAny = true;
        }
    }
    // --------------------------------------------
    bool bGotServers = false;

    if (bAddServers && filterByType.isEmpty())
        bGotServers = this->GetServers(theMap, filterByNym, true); //bPrependOTType=false by default

    return bFoundAny || bGotServers;
}


QString MTContactHandler::GetNymByAddress(QString qstrAddress)
{
    QString qstrResult("");

    {   // This block is here so locker will expire at the bottom of the block.
        QMutexLocker locker(&m_Mutex);

        QString encoded_address = Encode(qstrAddress);

        QString str_select = QString("SELECT nym_id "
                                     "FROM `nym_method` "
                                     "WHERE address='%1'").arg(encoded_address);

        int nRows = DBHandler::getInstance()->querySize(str_select);
        // -----------------------------------
        for (int ii=0; ii < nRows; ++ii)
            return DBHandler::getInstance()->queryString(str_select, 0, ii);
    }

    return qstrResult;
}

//QString create_nym_method
// = "CREATE TABLE nym_method(nym_id TEXT, method_id INTEGER, address TEXT, PRIMARY KEY(nym_id, method_id, address))";
//QString create_contact_method
// = "CREATE TABLE contact_method(contact_id INTEGER, method_type TEXT, address TEXT, PRIMARY KEY(contact_id, method_id, address))";

int MTContactHandler::GetContactByAddress(QString qstrAddress)
{
    {   // This block is here so locker will expire at the bottom of the block.
        QMutexLocker locker(&m_Mutex);

        QString encoded_address = Encode(qstrAddress);

        QString str_select = QString("SELECT contact_id "
                                     "FROM `contact_method` "
                                     "WHERE address='%1'").arg(encoded_address);

        int nRows = DBHandler::getInstance()->querySize(str_select);
        // -----------------------------------
        for (int ii=0; ii < nRows; ++ii)
        {
            return DBHandler::getInstance()->queryInt(str_select, 0, ii);
        }
    }

    return 0;
}

bool MTContactHandler::GetMsgMethodTypesByContact(mapIDName & theMap, int nFilterByContact, bool bAddServers/*=false*/, QString filterByType/*=""*/, bool bIncludeTypeInKey/*=true*/)
{
// = "CREATE TABLE contact_method(contact_id INTEGER, method_type TEXT, address TEXT, PRIMARY KEY(contact_id, method_id, address))";

    bool bFoundAny = false;

    {   // This block is here so locker will expire at the bottom of the block.
        QMutexLocker locker(&m_Mutex);

        QString qstrTypeFilter("");

        if (!filterByType.isEmpty())
        {
            QString qstrEncType = Encode(filterByType);
            qstrTypeFilter = QString(" AND method_type='%1'").arg(qstrEncType);
        }

        QString str_select = QString("SELECT contact_method.method_type, contact_method.address "
                                     "FROM `contact_method` "
                                     "WHERE contact_id=%1%2").arg(nFilterByContact).arg(qstrTypeFilter);

        int nRows = DBHandler::getInstance()->querySize(str_select);
        // -----------------------------------
        for (int ii=0; ii < nRows; ii++)
        {
            //Extract data
            QString qstrEncType    = DBHandler::getInstance()->queryString(str_select, 0, ii);
            QString qstrEncAddress = DBHandler::getInstance()->queryString(str_select, 1, ii);
            // -----------------------------------------------------
            QString qstrType       = Decode(qstrEncType);
            QString qstrAddress    = Decode(qstrEncAddress);
            // -----------------------------------------------------
            std::string strTypeDisplay = MTComms::displayName(qstrType.toStdString());
            QString    qstrTypeDisplay = QString::fromStdString(strTypeDisplay);

            // For a Bitmessage address, the ID would be:
            // "bitmessage|BITMESSAGE_ADDRESS"
            // Display name:  "Bitmessage: BITMESSAGE_ADDRESS"
            //
            QString qstrID;
            QString qstrName = QString("%1: %2").arg(qstrTypeDisplay).arg(qstrAddress);

            if (!bIncludeTypeInKey || !filterByType.isEmpty())
                qstrID   = QString("%1").arg(qstrAddress); // Address only. No need to put the type if we already filtered based on type.
            else
                qstrID   = QString("%1|%2").arg(qstrType).arg(qstrAddress); // Here we put the type and address. Caller can split the string.
            // --------------------------------------------
            theMap.insertMulti(qstrID, qstrName);

            bFoundAny = true;
        }
    }
    // --------------------------------------------
    bool bGotServers = false;

    if (bAddServers && filterByType.isEmpty())
        bGotServers = this->GetServers(theMap, nFilterByContact, true); //bPrependOTType=false by default

    return bFoundAny || bGotServers;
}

// --------------------------------------------

bool MTContactHandler::GetAddressesByContact(mapIDName & theMap, int nFilterByContact, QString filterByType, bool bIncludeTypeInKey/*=true*/)
{
    return this->GetMsgMethodTypesByContact(theMap, nFilterByContact, false, filterByType, bIncludeTypeInKey);
}

// --------------------------------------------

/*
QString create_msg_method = "CREATE TABLE msg_method"
        " (method_id INTEGER PRIMARY KEY,"   // 1, 2, etc.
        "  method_display_name TEXT,"        // "Localhost"
        "  method_type TEXT,"                // "bitmessage"
        "  method_type_display TEXT,"        // "Bitmessage"
        "  method_connect TEXT)";            // "http://username:password@http://127.0.0.1:8332/"
*/

int  MTContactHandler::AddMsgMethod(QString display_name, QString type, QString type_display, QString connect)
{
    QMutexLocker locker(&m_Mutex);

    QString encoded_display_name  = Encode(display_name);
    QString encoded_type          = Encode(type);
    QString encoded_type_display  = Encode(type_display);
    QString encrypted_connect_str = Encrypt(connect);

    QString str_insert = QString("INSERT INTO `msg_method` "
                                 "(`method_id`, `method_display_name`, `method_type`, `method_type_display`, `method_connect`) "
                                 "VALUES(NULL, '%1', '%2', '%3', '%4')")
            .arg(encoded_display_name).arg(encoded_type).arg(encoded_type_display).arg(encrypted_connect_str);

    DBHandler::getInstance()->runQuery(str_insert);

    int nMethodID = DBHandler::getInstance()->queryInt("SELECT last_insert_rowid() from `msg_method`", 0, 0);

    return nMethodID;
}

// --------------------------------------------

//QString create_nym_method      = "CREATE TABLE nym_method(nym_id TEXT, method_id INTEGER, address TEXT, PRIMARY KEY(nym_id, method_id, address))";
//QString create_contact_method  = "CREATE TABLE contact_method(contact_id INTEGER, method_type TEXT, address TEXT, PRIMARY KEY(contact_id, method_id, address))";

bool MTContactHandler::DeleteMsgMethod(int nMethodID)
{
    QMutexLocker locker(&m_Mutex);

    QString str_delete_nym     = QString("DELETE FROM `nym_method` "
                                 "WHERE `method_id`=%1").arg(nMethodID);
    QString str_delete_msg     = QString("DELETE FROM `msg_method` "
                                 "WHERE `method_id`=%1").arg(nMethodID);

    return (DBHandler::getInstance()->runQuery(str_delete_nym)     &&
            DBHandler::getInstance()->runQuery(str_delete_msg));
}

// --------------------------------------------

// If we have a specific Nym/Server pair, we check (using this function) to see
// if this server is already in my internal list for a given NymID. Notice this
// happens whether there is a CONTACT for that Nym, or not.
// If the pairing is not found, a record is added.
//
void MTContactHandler::NotifyOfNymServerPair(QString nym_id_string, QString notary_id_string)
{
    QMutexLocker locker(&m_Mutex);

    QString str_select_server = QString("SELECT `notary_id` FROM `nym_server` WHERE `nym_id`='%1' AND `notary_id`='%2' LIMIT 0,1").
            arg(nym_id_string).arg(notary_id_string);
    int nRowsServer = DBHandler::getInstance()->querySize(str_select_server);

    if (0 == nRowsServer) // It wasn't already there. (Add it.)
    {
        QString str_insert_server = QString("INSERT INTO `nym_server` "
                                            "(`nym_id`, `notary_id`) "
                                            "VALUES('%1', '%2')").arg(nym_id_string).arg(notary_id_string);
        DBHandler::getInstance()->runQuery(str_insert_server);
    }
}

void MTContactHandler::NotifyOfNymServerUnpair(QString nym_id_string, QString notary_id_string)
{
    QMutexLocker locker(&m_Mutex);

    QString str_select_server = QString("SELECT `notary_id` FROM `nym_server` WHERE `nym_id`='%1' AND `notary_id`='%2' LIMIT 0,1").
            arg(nym_id_string).arg(notary_id_string);
    const int nRowsServer = DBHandler::getInstance()->querySize(str_select_server);

    if (nRowsServer > 0) // It's already there. (Remove it.)
    {
        QString str_insert_server = QString("DELETE FROM `nym_server` WHERE `nym_id`='%1' AND `notary_id`='%2'").
                arg(nym_id_string).arg(notary_id_string);
        DBHandler::getInstance()->runQuery(str_insert_server);
    }
}

void MTContactHandler::NotifyOfNymNamePair(QString nym_id_string, QString name_string)
{
    QMutexLocker locker(&m_Mutex);

    QString str_select = QString("SELECT `nym_id` FROM `nym` WHERE `nym_id`='%1' LIMIT 0,1").arg(nym_id_string);
    const int nRows = DBHandler::getInstance()->querySize(str_select);

    QString queryStr;

    if (0 == nRows) // It wasn't already there. (Add it.)
    {
        queryStr = QString("INSERT INTO `nym` "
                           "(`nym_id`, `nym_display_name`, `contact_id`) "
                           "VALUES(:nym_idBlah, :nym_display_nameBlah, 0)");
    }
    else
    {
        queryStr = QString("UPDATE `nym` SET `nym_display_name`=:nym_display_nameBlah WHERE `nym_id`=:nym_idBlah");
    }

    try
    {
    #ifdef CXX_11
        std::unique_ptr<DBHandler::PreparedQuery> qu;
    #else /* CXX_11?  */
        std::auto_ptr<DBHandler::PreparedQuery> qu;
    #endif /* CXX_11?  */
        qu.reset (DBHandler::getInstance ()->prepareQuery (queryStr));
        // ---------------------------------------------
        qu->bind (":nym_idBlah", nym_id_string);
        qu->bind (":nym_display_nameBlah", name_string);

        DBHandler::getInstance ()->runQuery (qu.release ());
    }
    catch (const std::exception& exc)
    {
        qDebug () << "Error: " << exc.what ();
    }
}

// NOTE: if an account isn't ALREADY found in my contact list, I am
// very unlikely to find it in my wallet (since it's still most likely
// someone else's account) and so I am not able to look up its associated
// nymId, NotaryID, and assetTypeIds. Therefore if I have those available
// when I call this function, then I NEED to pass them in, so that if the
// account CAN be added to some existing contact, we will have its IDs to
// add.
// Therefore this function will not bother adding an account when the Nym
// is unknown.
//
int MTContactHandler::FindContactIDByAcctID(QString acct_id_string,
                                            QString nym_id_string/*=QString("")*/,
                                            QString notary_id_string/*=QString("")*/,
                                            QString asset_id_string/*=QString("")*/)
{
    QMutexLocker locker(&m_Mutex);

    int nContactID = 0;

    QString str_select = QString("SELECT `contact_id` "
                                 "FROM `nym` "
                                 "INNER JOIN `nym_account` "
                                 "ON nym_account.nym_id=nym.nym_id "
                                 "WHERE nym_account.account_id='%1'").arg(acct_id_string);

    int nRows = DBHandler::getInstance()->querySize(str_select);

    for(int ii=0; ii < nRows; ii++)
    {
        //Extract data
        nContactID = DBHandler::getInstance()->queryInt(str_select, 0, ii);
        break;

        // IN THIS CASE, the account record already existed for the given account ID.
        // We were able to join it to the Nym table and get the contact ID as well.
        // BUT -- what if the account record is incomplete? For example, what if
        // the server ID, or asset type ID fields were blank for that account?
        //
        // If they were in the record already, but blank in this call, we'd
        // want to discard the blank values and keep the existing record. But
        // if they were blank in the record, yet available here in this call,
        // then we'd want to UPDATE the record with the latest available values.
        // We certainly wouldn't want to just return and leave the record blank,
        // when we DID have good values that good have been set.
        //
        // UPDATE: They shouldn't ever be blank, because when the account ID is
        // available, then the server and asset type also are (so they would have
        // been added.) The only ID that could be missing would be a NymID,
        // and that wouldn't put us in this block due to the JOIN on NymID.
        // Therefore BELOW this block, we have to check for cases where the account
        // exists even when the Nym doesn't, and if the Nym ID was passed in,
        // add the new Nym, and update the account record with that Nym ID.
        // Of course in that case, we are adding a Nym who has no corresponding
        // contact record, but if he ever IS added as a contact, we will be able
        // to display his name next to his account-related receipts, even those
        // which have no Nym ID associated, since those that DID will enable us
        // to look up later the ones that DON'T.
        //
    }
    // ---------------------------------
    // The above only works if we already have a record of the account AND Nym.
    // But let's say this account is new -- no record of it, even though we
    // may already know the NYM who owns this account.
    //
    // Therefore we need to check to see if the Nym is on a contact, who owns
    // this account, and if so, we need to add this account under that Nym,
    // and return the contact ID. Although it may be in reverse -- we may
    // know the account and Nym IDs now, even though we previously had a record
    // for one or the other, and up until now had no way of connecting the two.
    //
    // However, we need to add the account one way or the other, so we'll do
    // that first, and then if the following piece applies, it can always
    // assume to do an update instead of an insert.
    // ---------------------------------
    QString final_notary_id = notary_id_string;
    QString final_nym_id    = nym_id_string;
    // ---------------------------------
    QString str_select_acct = QString("SELECT * "
                                      "FROM `nym_account` "
                                      "WHERE account_id='%1'").arg(acct_id_string);

    int nRowsAcct = DBHandler::getInstance()->querySize(str_select_acct);

    if (nRowsAcct > 0) // If the account record already existed.
    {
        // Update it IF we have values worth sticking in there.
        //
        if (!notary_id_string.isEmpty() || !asset_id_string.isEmpty() || !nym_id_string.isEmpty())
        {
      //    nym_account(account_id TEXT PRIMARY KEY, notary_id TEXT, nym_id TEXT, asset_id TEXT,
      //                account_display_name TEXT)";
            QString existing_notary_id = DBHandler::getInstance()->queryString(str_select_acct, 1, 0);
            QString existing_asset_id  = DBHandler::getInstance()->queryString(str_select_acct, 3, 0);
            QString existing_nym_id    = DBHandler::getInstance()->queryString(str_select_acct, 2, 0);

            // Here we're just making sure we don't run an update unless we've
            // actually added some new data.
            //
            if ((existing_notary_id.isEmpty() && !notary_id_string.isEmpty()) ||
                (existing_asset_id.isEmpty()  && !asset_id_string.isEmpty())  ||
                (existing_asset_id.isEmpty()  && !asset_id_string.isEmpty()) )
            {
                        final_notary_id    = !existing_notary_id.isEmpty() ? existing_notary_id : notary_id_string;
                QString final_asset_id     = !existing_asset_id.isEmpty()  ? existing_asset_id  : asset_id_string;
                        final_nym_id       = !existing_nym_id.isEmpty()    ? existing_nym_id    : nym_id_string;
                // -----------------------------------------------------------------
                QString str_update_acct = QString("UPDATE `nym_account` SET `notary_id`='%1',`asset_id`='%2',`nym_id`='%3' WHERE `account_id`='%4'").
                        arg(final_notary_id).arg(final_asset_id).arg(final_nym_id).arg(acct_id_string);

                DBHandler::getInstance()->runQuery(str_update_acct);
            }
        }
    }
    else // the account record didn't already exist.
    {
        // Add it then.
        //
        QString str_insert_acct = QString("INSERT INTO `nym_account` "
                                          "(`account_id`, `notary_id`, `nym_id`, `asset_id`) "
                                          "VALUES('%1', '%2', '%3', '%4')").arg(acct_id_string).arg(notary_id_string).arg(nym_id_string).arg(asset_id_string);
        DBHandler::getInstance()->runQuery(str_insert_acct);
    }
    // By this point, the record of this account definitely exists, though we may not have previously
    // had a record of it. (Thus below, we can assume to update, rather than insert, such a record.)
    // ---------------------------------
    // Here's where we check to see if the Nym exists for this account,
    // for any other existing contact, even though the account itself
    // may not have previously existed.
    //
    if (!final_nym_id.isEmpty())
    {
        QString str_select_nym = QString("SELECT `contact_id` FROM `nym` WHERE `nym_id`='%1' LIMIT 0,1").arg(final_nym_id);

        int nRowsNym = DBHandler::getInstance()->querySize(str_select_nym);

        if (nRowsNym > 0) // the nymId was found!
        {
            for(int ii=0; ii < nRowsNym; ii++)
            {
                // Found it! (If we're in this loop.) This means a Contact was found in the contact db
                // who already contained a Nym with an ID matching the NymID on this account.
                //
                nContactID = DBHandler::getInstance()->queryInt(str_select_nym, 0, ii);
                // ------------------------------------------------------------
                // So we definitely found the right contact and should return it.
                //
                break; // In practice there should only be one row.

            } // For (nym existed, even though account didn't.)
        }
        // -------------------------
        // else... nym record wasn't found, but we DO have the Nym ID as well as the account ID,
        // and we HAVE already added a record for the account. So perhaps we should add a
        // record for the Nym as well...
        //
        else
        {
            // How to "Upsert":
//          INSERT OR IGNORE INTO players (user_name, age) VALUES ("steven", 32);
//          UPDATE players SET user_name="steven", age=32 WHERE user_name="steven";

            QString str_insert_nym = QString("INSERT INTO `nym` "
                                             "(`nym_id`) "
                                             "VALUES('%1')").arg(final_nym_id);
            DBHandler::getInstance()->runQuery(str_insert_nym);
            // -----------------------------------------------
//          QString str_update_nym = QString("UPDATE `nym` "
//                                           "(`nym_id`) "
//                                           "VALUES('%1')").arg(final_nym_id);
//          DBHandler::getInstance()->runQuery(str_update_nym);
        }
        // ---------------------------------------------------------------------
        // Finally, do the same actions found in NotifyOfNymServerPair, only if final_notary_id
        // !isEmpty(), to make sure we record the server as well, when appropriate.
        //
        if (!final_notary_id.isEmpty())
        {
            QString str_select_server = QString("SELECT `notary_id` FROM `nym_server` WHERE `nym_id`='%1' AND `notary_id`='%2' LIMIT 0,1").arg(final_nym_id).arg(final_notary_id);
            int nRowsServer = DBHandler::getInstance()->querySize(str_select_server);

            if (0 == nRowsServer) // It wasn't already there. (Add it.)
            {
                QString str_insert_server = QString("INSERT INTO `nym_server` "
                                                    "(`nym_id`, `notary_id`) "
                                                    "VALUES('%1', '%2')").arg(final_nym_id).arg(final_notary_id);
                DBHandler::getInstance()->runQuery(str_insert_server);
            }
        }
    } // NymID is available (was passed in.)
    // ---------------------------------
    return nContactID;
}



int MTContactHandler::FindContactIDByNymID(QString nym_id_string)
{
    QMutexLocker locker(&m_Mutex);

    QString str_select = QString("SELECT `contact_id` FROM `nym` WHERE `nym_id`='%1'").arg(nym_id_string);

    int nRows = DBHandler::getInstance()->querySize(str_select);

    for(int ii=0; ii < nRows; ii++)
    {
        //Extract data
        int contact_id = DBHandler::getInstance()->queryInt(str_select, 0, ii);

        return contact_id; // In practice there should only be one row.
    }

    return 0; // Didn't find anyone.
}


MTContactHandler::~MTContactHandler()
{
}





