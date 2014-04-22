//
//  MTRecordList.cpp
//  OTClient module
//
//  Created by Chris Odom on 4/28/13.
//  Copyright (c) 2013 Monetas. All rights reserved.
//

#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/MTRecordList.hpp>

#include <opentxs/OpenTransactions.hpp>
#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OTWallet.hpp>
#include <opentxs/OTPayment.hpp>
#include <opentxs/OTAccount.hpp>
#include <opentxs/OTLedger.hpp>
#include <opentxs/OT_ME.hpp>
#include <opentxs/OTMessage.hpp>
#include <opentxs/OTLog.hpp>

#include <algorithm>


#define MC_UI_TEXT_TO "<font color='grey'>To:</font> %s"
#define MC_UI_TEXT_FROM "<font color='grey'>From:</font> %s"

//virtual
std::string MTNameLookup::GetNymName(const std::string & str_id,
                                     const std::string * p_server_id/*=NULL*/) const
{
    if (str_id.empty())
        return "";

    return OTAPI_Wrap::It()->GetNym_Name(str_id);
}

//virtual
std::string MTNameLookup::GetAcctName(const std::string & str_id,
                                      const std::string * p_nym_id/*=NULL*/,
                                      const std::string * p_server_id/*=NULL*/,
                                      const std::string * p_asset_id/*=NULL*/) const
{
    if (str_id.empty())
        return "";

    return OTAPI_Wrap::It()->GetAccountWallet_Name(str_id);
}


// ------------------------------------------------
//static

const std::string  MTRecordList::s_blank("");


const std::string  MTRecordList::s_message_type("message");

// ------------------------------------------------
// SETUP:

// Set the default server here.

void MTRecordList::SetServerID(const std::string str_id)
{
    ClearServers();
    // -----------------------
    AddServerID(str_id);
}

// Unless you have many servers, then use this.

void MTRecordList::AddServerID(const std::string str_id)
{
    m_servers.insert(m_servers.end(), str_id);
}


// Also clears m_contents

void MTRecordList::ClearServers()
{
    ClearContents();
    // -----------------------
    m_servers.clear();
}

// ------------------------------------------------


void MTRecordList::SetAssetID(const std::string str_id)
{
    ClearAssets();
    // -----------------------
    AddAssetID(str_id);
}


void MTRecordList::AddAssetID(const std::string str_id)
{
    // -------------------------
    OTWallet * pWallet = OTAPI_Wrap::OTAPI()->GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    // -------------------------
    OT_ASSERT_MSG(NULL != pWallet, "Wallet was NULL. Should never happen.");
    // ------------------------------------------------
    const OTString     strAssetTypeID(str_id);
    const OTIdentifier theAssetTypeID(strAssetTypeID);
    // ------------------------------------------------
    std::string str_asset_name;
    // ------------------------------------------------
    // Name is dollars, fraction is cents, TLA is USD and
    // Symbol is $ (for example.) Here, we're grabbing the TLA.
    //
    OTAssetContract * pAssetContract = pWallet->GetAssetContract(theAssetTypeID);
    // ------------------------------------------------
    if (NULL != pAssetContract)
    {
        str_asset_name = pAssetContract->GetCurrencyTLA().Get();  // This might be "USD" -- preferable that this works.
        // ------------------------------------------------
        if (str_asset_name.empty())
            str_asset_name = pAssetContract->GetCurrencySymbol().Get();  // This might be "$".
        // ------------------------------------------------
        if (str_asset_name.empty())
            str_asset_name = pAssetContract->GetCurrencyName().Get();  // This might be "dollars".
    }
    // ------------------------------------------------
    if (str_asset_name.empty())
        str_asset_name = OTAPI_Wrap::It()->GetAssetType_Name(str_id); // Otherwise we try to grab the name.
    // ------------------------------------------------
    // (Otherwise we just leave it blank. The ID is too big to cram in here.)
    // ------------------------------------------------
    m_assets.insert(std::pair<std::string, std::string>(str_id, str_asset_name));
}


void MTRecordList::ClearAssets()
{
    ClearContents();
    // -----------------------
    m_assets.clear();
}

// ------------------------------------------------


void MTRecordList::SetNymID(const std::string str_id)
{
    ClearNyms();
    // -----------------------
    AddNymID(str_id);
}


void MTRecordList::AddNymID(const std::string str_id)
{
    m_nyms.insert(m_nyms.end(), str_id);
}


void MTRecordList::ClearNyms()
{
    ClearContents();
    // -----------------------
    m_nyms.clear();
}

// ------------------------------------------------


void MTRecordList::SetAccountID(const std::string str_id)
{
    ClearAccounts();
    // -----------------------
    AddAccountID(str_id);
}


void MTRecordList::AddAccountID(const std::string str_id)
{
    m_accounts.insert(m_accounts.end(), str_id);
}


void MTRecordList::ClearAccounts()
{
    ClearContents();
    // -----------------------
    m_accounts.clear();
}



 void MTRecordList::AcceptChequesAutomatically  (bool bVal/*=true*/) { m_bAutoAcceptCheques   = bVal; }
 void MTRecordList::AcceptReceiptsAutomatically (bool bVal/*=true*/) { m_bAutoAcceptReceipts  = bVal; }
 void MTRecordList::AcceptTransfersAutomatically(bool bVal/*=true*/) { m_bAutoAcceptTransfers = bVal; }
 void MTRecordList::AcceptCashAutomatically     (bool bVal/*=true*/) { m_bAutoAcceptCash      = bVal; }

 bool MTRecordList::DoesAcceptChequesAutomatically  ()  { return m_bAutoAcceptCheques;   }
 bool MTRecordList::DoesAcceptReceiptsAutomatically ()  { return m_bAutoAcceptReceipts;  }
 bool MTRecordList::DoesAcceptTransfersAutomatically()  { return m_bAutoAcceptTransfers; }
 bool MTRecordList::DoesAcceptCashAutomatically     ()  { return m_bAutoAcceptCash;      }

typedef std::map<int32_t, OTPayment *> mapOfPayments;


bool MTRecordList::PerformAutoAccept()
{
    // -------------------------
    OTWallet * pWallet = OTAPI_Wrap::OTAPI()->GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    // -------------------------
    if (NULL == pWallet)
    {
        OTLog::vError("MTRecordList::%s: Error: Wallet is NULL.\n", __FUNCTION__);
        return false;
    }
    // ------------------------------------------------
    // LOOP NYMS
    //
    int nNymIndex = -1;
    if (m_bAutoAcceptCheques || m_bAutoAcceptCash) FOR_EACH_IT(list_of_strings, m_nyms, it_nym)
    {
        ++nNymIndex;
        // ------------------------------------------------
        if (0 == nNymIndex)
            OTLog::vOutput(0, "======================================\n %s: Beginning auto-accept loop through Nyms...\n", __FUNCTION__);
        // ------------------------------------------------
        const std::string  & str_nym_id(*it_nym);
        const OTIdentifier   theNymID  (str_nym_id);
        const OTString       strNymID  (theNymID);
        OTPseudonym * pNym = pWallet->GetNymByID(theNymID);
        if (NULL == pNym) continue;
        // ------------------------------------------------
        // LOOP SERVERS
        //
        // For each nym, for each server, loop through its payments inbox
        //
        int nServerIndex = -1;
        FOR_EACH_IT(list_of_strings, m_servers, it_server)
        {
            ++nServerIndex;
            // --------------------------
            const std::string  & str_server_id(*it_server);
            const OTIdentifier   theServerID(str_server_id);
            OTServerContract *   pServer = pWallet->GetServerContract(theServerID);
            OT_ASSERT(NULL != pServer);
            // ------------------------------------------------
            const OTString strServerID(theServerID);
            OTLog::vOutput(0, "%s: Server %d, ID: %s\n", __FUNCTION__, nServerIndex, strServerID.Get());
            // ------------------------------------------------
            mapOfPayments thePaymentMap;
            // ------------------------------------------------
            // OPTIMIZE FYI:
            // The "NoVerify" version is much faster, but you will lose the ability to get the
            // sender/recipient name from the receipts in the box. The code will, however, work
            // either way.
            //
            OTLedger * pInbox = m_bRunFast ? OTAPI_Wrap::OTAPI()->LoadPaymentInboxNoVerify(theServerID, theNymID) :
                                             OTAPI_Wrap::OTAPI()->LoadPaymentInbox        (theServerID, theNymID);
            OTCleanup<OTLedger> theInboxAngel(pInbox);

            int32_t  nIndex = (-1);
            // It loaded up, so let's loop through it.
            if (NULL != pInbox) FOR_EACH(mapOfTransactions, pInbox->GetTransactionMap())
            {
                OTTransaction * pBoxTrans = (*it).second;
                OT_ASSERT(NULL != pBoxTrans);
                // ------------------------------
                ++nIndex; // 0 on first iteration.
                // ------------------------------------------------
                OTLog::vOutput(0, "%s: Incoming payment: %d\n", __FUNCTION__, nIndex);
                // ------------------------------------------------
                const std::string * p_str_asset_type = &MTRecordList::s_blank; // <========== ASSET TYPE
                const std::string * p_str_asset_name = &MTRecordList::s_blank; // asset type display name.
                // ------------------------------------------------
                std::string str_amount;  // <========== AMOUNT
                std::string str_type;    // Instrument type.
                // ------------------------------------------------
                OTPayment * pPayment = pInbox->GetInstrument(*pNym,
                                                             nIndex);     // ===> Returns financial instrument by index.
                OTCleanup<OTPayment> thePaymentAngel(pPayment);
                // ------------------------------
                if (NULL == pPayment) // then we treat it like it's abbreviated.
                {
                    OTLog::vError("%s: Payment retrieved from payments inbox was NULL. (It's abbreviated?) Skipping.\n", __FUNCTION__);
                }
                // ---------------------------------------------------
                // We have pPayment, the instrument accompanying the receipt in the payments inbox.
                //
                else if (pPayment->IsValid() && pPayment->SetTempValues())
                {
                    OTIdentifier theAssetTypeID;

                    if (pPayment->GetAssetTypeID(theAssetTypeID))
                    {
                        OTString strTemp(theAssetTypeID);
                        const std::string str_inpmt_asset(strTemp.Get()); // The asset type we found on the payment (if we found anything.)
                        // -----------------------------
                        map_of_strings::iterator it_asset = m_assets.find(str_inpmt_asset);
                        // -----------------------------
                        if (it_asset != m_assets.end()) // Found it on the map of asset types we care about.
                        {
                            p_str_asset_type = &(it_asset->first);   // Set the asset type ID.
                            p_str_asset_name = &(it_asset->second);  // The CurrencyTLA. Examples: USD, BTC, etc.
                        }
                        else
                        {
                            // There was definitely an asset type on the instrument, and it definitely
                            // did not match any of the assets that we care about.
                            // Therefore, skip.
                            //
                            OTLog::vError("%s: Skipping: Incoming payment (we don't care about asset %s)\n",
                                          __FUNCTION__, str_inpmt_asset.c_str());
                            continue;
                        }
                    }
                    // By this point, p_str_asset_type and p_str_asset_name are definitely set.
                    OT_ASSERT(NULL != p_str_asset_type); // and it's either blank, or it's one of the asset types we care about.
                    OT_ASSERT(NULL != p_str_asset_name); // and it's either blank, or it's one of the asset types we care about.
                    // ---------------------------------------------------
                    // Instrument type (cheque, voucher, etc)
                    int nType = static_cast<int> (pPayment->GetType());

                    str_type = MTRecord_GetTypeString(nType);
                    // ------------------------------
                    // For now, we only accept cash, cheques and vouchers.
                    //
                    if ( (m_bAutoAcceptCheques && ((0 == str_type.compare("cheque")) || (0 == str_type.compare("voucher")))) ||
                         (m_bAutoAcceptCash    &&  (0 == str_type.compare("cash"))))
                    {
                        OTLog::vOutput(0, "%s: Adding to acceptance list: pending incoming %s.\n",
                                       __FUNCTION__, str_type.c_str());
                        // -------------------------
                        thePaymentMap.insert(std::pair<int32_t, OTPayment *>(nIndex, pPayment));
                        thePaymentAngel.SetCleanupTargetPointer(NULL); // Now we HAVE to cleanup, below... Otherwise pPayment will leak.
                    }
                    else
                        OTLog::vOutput(0, "%s: Unknown instrument type: %s\n", __FUNCTION__, str_type.c_str());
                }
                else
                    OTLog::vOutput(0, "%s: Failed in pPayment->IsValid or pPayment->SetTempValues()\n", __FUNCTION__);
                // ------------------------------
            } // looping through payments inbox.
            else
                OTLog::vOutput(1, "%s: Failed loading payments inbox. (Probably just doesn't exist yet.)\n", __FUNCTION__);
            // --------------------------------------------------------------------------
            // Above we compiled a list of purses, cheques / vouchers to accept.
            // If there are any on that list, then ACCEPT them here.
            //
            if (!thePaymentMap.empty())
            {
                for (mapOfPayments::reverse_iterator it = thePaymentMap.rbegin(); it != thePaymentMap.rend(); ++it) // backwards since we are processing (removing) payments by index.
                {
                    long        lIndex   = static_cast<long>(it->first);
                    OTPayment * pPayment = it->second;
                    // -------------------------
                    if (NULL == pPayment)
                    {
                        OTLog::vError("%s: Error: payment pointer was NULL! (Should never happen.) Skipping.\n", __FUNCTION__);
                        continue;
                    }
                    // -------------------------
                    OTString strPayment;
                    std::string str_payment_contents;

                    if (pPayment->GetPaymentContents(strPayment))
                    {
                        str_payment_contents = strPayment.Get();
                    }
                    else
                    {
                        OTLog::vError("%s: Error: Failed while trying to get payment string contents. (Skipping.)\n", __FUNCTION__);
                        continue;
                    }
                    // ---------------------------------
                    OTIdentifier paymentAssetType;
                    bool bGotAsset = pPayment->GetAssetTypeID(paymentAssetType);

                    std::string str_asset_type_id;

                    if (bGotAsset)
                    {
                        const OTString strAssetTypeID(paymentAssetType);
                        str_asset_type_id = strAssetTypeID.Get();
                    }
                    // -------------------------------------------
                    if (str_asset_type_id.empty())
                    {
                        OTLog::vError("%s: Error: Failed while trying to get asset type ID from payment. (Skipping.)\n", __FUNCTION__);
                        continue;
                    }
                    // -------------------------------------------
                    // pick an account to deposit the cheque into.
                    //
                    FOR_EACH_IT(list_of_strings, m_accounts, it_acct)
                    {
                        const std::string  & str_account_id(*it_acct);
                        const OTIdentifier   theAccountID  (str_account_id);
                        OTAccount * pAccount = pWallet->GetAccount(theAccountID);
                        OT_ASSERT(NULL != pAccount);
                        // ------------------------------------------------
                        const OTIdentifier & theAcctNymID    = pAccount->GetUserID();
                        const OTIdentifier & theAcctServerID = pAccount->GetPurportedServerID();
                        const OTIdentifier & theAcctAssetID  = pAccount->GetAssetTypeID();
                        // -----------------------------------
                        const std::string    str_acct_type   = pAccount->GetTypeString();
                        // -----------------------------------
//                      const OTString       strAcctNymID   (theAcctNymID);
                        const OTString       strAcctServerID(theAcctServerID);
                        const OTString       strAcctAssetID (theAcctAssetID);
                        // ------------------------------------------------
                        // If the current account is owned by the Nym, AND it has the same asset type ID
                        // as the cheque being deposited, then let's deposit the cheque into that account.
                        //
                        // TODO: we should first just see if the default account matches, instead of doing
                        // this loop in the first place.
                        //
                        if ((theNymID == theAcctNymID)                           &&
                            (strAcctServerID.Compare(str_server_id    .c_str())) &&
                            (strAcctAssetID .Compare(str_asset_type_id.c_str())) &&
                            (0 == str_acct_type.compare("simple"))) // No issuer accounts allowed here. Only simple accounts.
                        {
                            // Accept it.
                            //
                            OTString strIndices;
                            strIndices.Format("%ld", lIndex);
                            const std::string str_indices(strIndices.Get());

                            OT_ME madeEasy;
                            int32_t nReturn = madeEasy.accept_from_paymentbox(str_account_id, str_indices, "ANY");

                            switch (nReturn)
                            {
                                case 0:
                                    OTLog::vOutput(0, "%s: This instrument was expired, so it was moved to the record box.\n", __FUNCTION__);
                                case 1: // success
                                    break;
                                    // ----------------------------------
                                default:
                                    OTLog::vError("%s: Error while trying to accept this instrument.\n", __FUNCTION__);
                                    break;
                            } // switch
                            break;
                        }
                    } // loop through accounts to find one to deposit cheque into.
                } // Loop through payments to deposit.
                // ------------------------------------------
                // Empty the list and delete the payments inside.
                //
                FOR_EACH(mapOfPayments, thePaymentMap)
                {
                    OTPayment * pPayment = it->second;
                    if (NULL != pPayment)
                        delete pPayment;
                    pPayment = NULL;
                }
                thePaymentMap.clear();
            } // if (!thePaymentMap.empty())
            // ------------------------------------------------
        } // FOR_EACH_IT(list_of_strings, m_servers, it_server)
    } // FOR_EACH_IT(list_of_strings, m_nyms, it_nym)
    // ------------------------------------------------
    // ASSET ACCOUNT -- INBOX
    //
    // Loop through the Accounts.
    //
    // ------------------------------------------------
    int nAccountIndex = -1;
    if (m_bAutoAcceptReceipts || m_bAutoAcceptTransfers) FOR_EACH_IT(list_of_strings, m_accounts, it_acct)
    {
        ++nAccountIndex; // (0 on first iteration.)
        // ------------------
        if (0 == nAccountIndex)
            OTLog::vOutput(0, "---------------------------------\n %s: Beginning auto-accept loop through the accounts in the wallet...\n",
                           __FUNCTION__);
        // ------------------------------------------------
        // For each account, loop through its inbox, outbox, and record box.
        //
        const std::string  & str_account_id(*it_acct);
        const OTIdentifier   theAccountID  (str_account_id);
        OTAccount * pAccount = pWallet->GetAccount(theAccountID);
        OT_ASSERT(NULL != pAccount);
        // ------------------------------------------------
        const OTIdentifier & theNymID    = pAccount->GetUserID();
        const OTIdentifier & theServerID = pAccount->GetPurportedServerID();
        const OTIdentifier & theAssetID  = pAccount->GetAssetTypeID();
        // ------------------------------------------------
        const OTString       strNymID   (theNymID);
        const OTString       strServerID(theServerID);
        const OTString       strAssetID (theAssetID);
        // ------------------------------------------------
        OTLog::vOutput(0, "------------\n%s: Account: %d, ID: %s\n", __FUNCTION__,
                       nAccountIndex, str_account_id.c_str());
        // ------------------------------------------------
        const std::string    str_nym_id    (strNymID   .Get());
        const std::string    str_server_id (strServerID.Get());
        const std::string    str_asset_id  (strAssetID .Get());
        // ------------------------------------------------
        const std::string  * pstr_nym_id     = &MTRecordList::s_blank;
        const std::string  * pstr_server_id  = &MTRecordList::s_blank;
        const std::string  * pstr_asset_id   = &MTRecordList::s_blank;
        const std::string  * pstr_asset_name = &MTRecordList::s_blank;
        // ------------------------------------------------
        // NOTE: Since this account is already on my "care about" list for accounts,
        // I wouldn't bother double-checking my "care about" lists for servers, nyms,
        // and asset types. But I still look up the appropriate string for each, since
        // I have to pass a reference to it into the constructor for MTRecord. (To a version
        // that won't be deleted, since the MTRecord will reference it. And the "care about"
        // list definitely contains a copy of the string that won't be deleted.)
        //
        list_of_strings::iterator it_nym    = std::find(m_nyms   .begin(),    m_nyms.end(),    str_nym_id);
        list_of_strings::iterator it_server = std::find(m_servers.begin(), m_servers.end(), str_server_id);
         map_of_strings::iterator it_asset  = m_assets.find(str_asset_id);
        // ------------------------------------------------
        if ((m_nyms.end() == it_nym) || (m_servers.end() == it_server) || (m_assets.end() == it_asset))
        {
            OTLog::vOutput(0, "%s: Skipping an account (%s) since its Nym, or Server, "
                           "or Asset Type wasn't on my list.\n", __FUNCTION__,
                           str_account_id.c_str());
            continue;
        }
        // ------------------------------------------------
        // These pointers are what we'll use to construct each MTRecord.
        //
        pstr_nym_id     = &(*it_nym);
        pstr_server_id  = &(*it_server);
        pstr_asset_id   = &(it_asset->first);
        pstr_asset_name = &(it_asset->second);
        // ------------------------------------------------
        // Loop through asset account INBOX.
        //
        // OPTIMIZE FYI:
        // NOTE: LoadInbox is much SLOWER than LoadInboxNoVerify, but it also lets you get
        // the NAME off of the box receipt. So if you are willing to GIVE UP the NAME, in
        // return for FASTER PERFORMANCE, then call SetFastMode() before Populating.
        //
        OTLedger * pInbox = m_bRunFast ? OTAPI_Wrap::OTAPI()->LoadInboxNoVerify(theServerID, theNymID, theAccountID) :
                                         OTAPI_Wrap::OTAPI()->LoadInbox        (theServerID, theNymID, theAccountID);
        OTCleanup<OTLedger> theInboxAngel(pInbox);
        // ------------------------------------------------
        if (NULL == pInbox)
        {
            OTLog::vOutput(0, "%s: Skipping an account (%s) since its inbox failed to load (have you downloaded the latest one?)\n", __FUNCTION__,
                           str_account_id.c_str());
            continue;
        }
        // ------------------------------------------------
        const OTString    strInbox(*pInbox);
        const std::string str_inbox(strInbox.Get());
        // ------------------------------------------------
        bool bFoundAnyToAccept = false;
        std::string strResponseLedger;
        // ------------------------------------------------
        int nInboxIndex = -1;
        // It loaded up, so let's loop through it.
        FOR_EACH(mapOfTransactions, pInbox->GetTransactionMap())
        {
            ++nInboxIndex; // (0 on first iteration.)
            // ------------------------------------------------
            if (0 == nInboxIndex)
                OTLog::vOutput(0, "%s: Beginning loop through asset account INBOX...\n", __FUNCTION__);
            // --------------------------------
            OTTransaction * pBoxTrans = (*it).second;
            OT_ASSERT(NULL != pBoxTrans);
            // ------------------------------------------------
            OTLog::vOutput(0, "%s: Inbox index: %d\n", __FUNCTION__, nInboxIndex);
            // ------------------------------------------------
            const std::string str_type(pBoxTrans->GetTypeString()); // pending, chequeReceipt, etc.
            // ------------------------------------------------
            const bool bIsTransfer = (OTTransaction::pending == pBoxTrans->GetType());
            const bool bIsReceipt  = !bIsTransfer;
            // ------------------------------------------------
            if ((m_bAutoAcceptReceipts  &&  bIsReceipt)   ||
                (m_bAutoAcceptTransfers &&  bIsTransfer))
            {
                OTLog::vOutput(0, "%s: Auto-accepting: incoming %s (str_type: %s)\n",
                               __FUNCTION__, bIsTransfer ? "pending transfer" : "receipt",
                               str_type.c_str());
                // ------------------------------------------------
                // If we haven't found any yet, then this must be the first one!
                //
                if (!bFoundAnyToAccept)
                {
                    bFoundAnyToAccept = true;
                    // ------------------------
                    OT_ME madeEasy;

                    int32_t nNumberNeeded = 20;
                    if (!madeEasy.make_sure_enough_trans_nums(nNumberNeeded, // I'm just hardcoding: "Make sure I have at least 20 transaction numbers."
                                                              str_server_id, str_nym_id))
                    {
                        OTLog::vOutput(0, "\n\nFailure: make_sure_enough_trans_nums: returned false. (Skipping inbox for account %s)\n",
                                       str_account_id.c_str());
                        continue;
                    }
                    // ---------------------------------
                    strResponseLedger = OTAPI_Wrap::It()->Ledger_CreateResponse(str_server_id, str_nym_id, str_account_id, str_inbox);

                    if (strResponseLedger.empty())
                    {
                        OTLog::vOutput(0, "\n\nFailure: OT_API_Ledger_CreateResponse returned NULL. (Skipping inbox for account %s)\n",
                                       str_account_id.c_str());
                        continue;
                    }
                }
                // -------------------------
                const OTString strTrans(*pBoxTrans);
                const std::string str_trans(strTrans.Get());
                std::string strNEW_ResponseLEDGER = OTAPI_Wrap::It()->Transaction_CreateResponse(str_server_id, str_nym_id, str_account_id, strResponseLedger, str_trans, true); // accept = true (versus rejecting a pending transfer, for example.)

                if (strNEW_ResponseLEDGER.empty())
                {
                    OTLog::vOutput(0, "\n\nFailure: OT_API_Transaction_CreateResponse returned NULL. (Skipping inbox for account %s)\n",
                                   str_account_id.c_str());
                    continue;
                }
                strResponseLedger = strNEW_ResponseLEDGER;
            }
        } // if (NULL != pInbox) FOR_EACH(mapOfTransactions, pInbox->GetTransactionMap())
        // -------------------------------------------------
        // Okay now we have the response ledger all ready to go, let's process it!
        //
        if (bFoundAnyToAccept && !strResponseLedger.empty())
        {
            std::string strFinalizedResponse = OTAPI_Wrap::It()->Ledger_FinalizeResponse(str_server_id, str_nym_id, str_account_id, strResponseLedger);

            if (strFinalizedResponse.empty())
            {
                OTLog::vOutput(0, "\n\nFailure: OT_API_Ledger_FinalizeResponse returned NULL. (Skipping inbox for account %s)\n",
                               str_account_id.c_str());
                continue;
            }
            // ***************************************************************
            // Instantiate the "OT Made Easy" object.
            //
            OT_ME madeEasy;

            // Server communications are handled here...
            //
            std::string strResponse = madeEasy.process_inbox(str_server_id, str_nym_id, str_account_id, strFinalizedResponse);
            std::string strAttempt  = "process_inbox";

            // ***************************************************************

            int32_t nInterpretReply = madeEasy.InterpretTransactionMsgReply(str_server_id, str_nym_id, str_account_id, strAttempt, strResponse);

            if (1 == nInterpretReply)
            {
                // Download all the intermediary files (account balance, inbox, outbox, etc)
                // since they have probably changed from this operation.
                //
                bool bRetrieved = madeEasy.retrieve_account(str_server_id, str_nym_id, str_account_id, true); //bForceDownload defaults to false.

                OTLog::vOutput(0, "\n\nServer response (%s): SUCCESS processing/accepting inbox.\n", strAttempt.c_str());
                OTLog::vOutput(0, "%s retrieving intermediary files for account.\n", (bRetrieved ? "Success" : "Failed"));
            }
        }
    } // FOR_EACH_IT(list_of_strings, m_accounts, it_acct)
    // ------------------------------------------------
    return true;
}





bool compare_records (shared_ptr_MTRecord i, shared_ptr_MTRecord j)
{
    return j->operator<(*i);
}


// ***************************************************
// POPULATE:

// Populates m_contents from OT API. Calls ClearContents().

bool MTRecordList::Populate()
{
    OT_ASSERT(NULL != m_pLookup);
    // -----------------------
    ClearContents();
    // -----------------------
    // Loop through all the accounts.
    //
    // From Open-Transactions.h:
    // OTAPI_Wrap::OTAPI()->GetServerCount()
    //
    // From OTAPI.h:
    // OTAPI_Wrap::It()->GetServerCount()  // wraps the above call.
    //
    // -------------------------
    OTWallet * pWallet = OTAPI_Wrap::OTAPI()->GetWallet(__FUNCTION__); // This logs and ASSERTs already.
    // -------------------------
    if (NULL == pWallet)
    {
        OTLog::vError("MTRecordList::%s: Error: Wallet is NULL.\n", __FUNCTION__);
        return false;
    }
    // ------------------------------------------------
    // Before populating, process out any items we're supposed to accept automatically.
    //
    PerformAutoAccept();
    // ------------------------------------------------
    // OUTPAYMENTS, OUTMAIL, MAIL, PAYMENTS INBOX, and RECORD BOX (2 kinds.)
    // Loop through the Nyms.
    //
    int nNymIndex = -1;
    FOR_EACH_IT(list_of_strings, m_nyms, it_nym)
    {
        ++nNymIndex;
        // ------------------------------------------------
        if (0 == nNymIndex)
            OTLog::vOutput(0, "=============== %s: Beginning loop through Nyms...\n", __FUNCTION__);
        // ------------------------------------------------
        const std::string  & str_nym_id(*it_nym);
        const OTIdentifier   theNymID  (str_nym_id);
        const OTString       strNymID  (theNymID);
        OTPseudonym * pNym = pWallet->GetNymByID(theNymID);
        if (NULL == pNym) continue;
        // ------------------------------------------------
        // For each Nym, loop through his OUTPAYMENTS box.
        //
        const int32_t nOutpaymentsCount  = OTAPI_Wrap::It()->GetNym_OutpaymentsCount(str_nym_id);

        OTLog::vOutput(0, "--------\n%s: Nym %d, nOutpaymentsCount: %d, ID: %s\n",
                       __FUNCTION__, nNymIndex, nOutpaymentsCount, strNymID.Get());
        // ------------------------------------------------
        for ( int32_t nCurrentOutpayment = 0; nCurrentOutpayment < nOutpaymentsCount; ++nCurrentOutpayment)
        {
            // ------------------------------------------------
            OTLog::vOutput(0, "%s: Outpayment instrument: %d\n", __FUNCTION__, nCurrentOutpayment);
            // ------------------------------------------------
            const OTString strOutpayment(
                OTAPI_Wrap::It()->GetNym_OutpaymentsContentsByIndex(str_nym_id, nCurrentOutpayment));
            // ----------------------------------
            std::string str_memo;
            OTPayment   theOutPayment(strOutpayment);

            if (!theOutPayment.IsValid() || !theOutPayment.SetTempValues())
            {
                OTLog::vError("%s: Skipping: Unable to load outpayments instrument from string:\n%s\n",
                              __FUNCTION__, strOutpayment.Get());
                continue;
            }
            // ----------------------------------
            int64_t lAmount = 0;
            std::string str_amount;  // <========== AMOUNT

            if (theOutPayment.GetAmount(lAmount))
            {
                if (((OTPayment::CHEQUE  == theOutPayment.GetType())  ||
                     (OTPayment::PURSE   == theOutPayment.GetType())  ||
                     (OTPayment::VOUCHER == theOutPayment.GetType())) && (lAmount > 0))
                    lAmount *= (-1);

                if ((OTPayment::INVOICE  == theOutPayment.GetType())  && (lAmount < 0))
                    lAmount *= (-1);

                OTString strTemp;
                strTemp.Format("%ld", lAmount);
                str_amount = strTemp.Get();
            }
            // ----------------------------------
            OTIdentifier        theAssetTypeID;
            const std::string * p_str_asset_type = &MTRecordList::s_blank; // <========== ASSET TYPE
            const std::string * p_str_asset_name = &MTRecordList::s_blank; // asset type display name.
            std::string str_outpmt_asset; // The asset type we found on the payment (if we found anything.)

            if (theOutPayment.GetAssetTypeID(theAssetTypeID))
            {
                OTString strTemp(theAssetTypeID);
                str_outpmt_asset = strTemp.Get();
                // -----------------------------
                map_of_strings::iterator it_asset = m_assets.find(str_outpmt_asset);
                // -----------------------------
                if (it_asset != m_assets.end()) // Found it on the map of asset types we care about.
                {
                    p_str_asset_type = &(it_asset->first);   // Set the asset type ID.
                    p_str_asset_name = &(it_asset->second);  // The CurrencyTLA. Examples: USD, BTC, etc.
                }
                else
                {
                    // There was definitely an asset type on the instrument, and it definitely
                    // did not match any of the assets that we care about.
                    // Therefore, skip.
                    //
                    OTLog::vOutput(0, "%s: Skipping outpayment (we don't care about asset type %s)\n",
                                   __FUNCTION__, str_outpmt_asset.c_str());
                    continue;
                }
            }
            // By this point, p_str_asset_type and p_str_asset_name are definitely set.
            OT_ASSERT(NULL != p_str_asset_type); // and it's either blank, or it's one of the asset types we care about.
            OT_ASSERT(NULL != p_str_asset_name); // and it's either blank, or it's one of the asset types we care about.
            // ----------------------------------
            OTIdentifier theAccountID;
            const std::string * p_str_account = &MTRecordList::s_blank; // <========== ACCOUNT
            std::string str_outpmt_account; // The accountID we found on the payment (if we found anything.)

            if (theOutPayment.GetSenderAcctIDForDisplay(theAccountID)) // Since Nym is ME, the Account must be MY acct.
            {                                             // (In Outpayments, the SENDER's account is MY acct.)
                OTString strTemp(theAccountID);
                str_outpmt_account = strTemp.Get();
                // -----------------------------
                list_of_strings::iterator it_acct = std::find(m_accounts.begin(), m_accounts.end(), str_outpmt_account);
                // -----------------------------
                if (it_acct != m_accounts.end()) // Found it on the list of accounts we care about.
                {
                    p_str_account = &(*it_acct);
                }
                // We don't skip vouchers since the sender account (e.g. the server's account)
                // is definitely not one of my accounts -- so the voucher would end up getting
                // skipped every single time.
                //
//              else if (OTPayment::VOUCHER != theOutPayment.GetType())
                else
                {
                    // There was definitely an account on the instrument, and it definitely
                    // did not match any of the accounts that we care about.
                    // Therefore, skip.
                    //
                    OTLog::vOutput(0, "%s: Skipping outpayment (we don't care about account %s)\n",
                                   __FUNCTION__, str_outpmt_account.c_str());
                    continue;
                }
            }
            // By this point, p_str_account is definitely set.
            OT_ASSERT(NULL != p_str_account); // and it's either blank, or it's one of the accounts we care about.
            // ----------------------------------
            // strOutpayment contains the actual outgoing payment instrument.
            //
            const std::string str_outpmt_server =
                OTAPI_Wrap::It()->GetNym_OutpaymentsServerIDByIndex(str_nym_id, nCurrentOutpayment);
            // ----------------------------------
            const std::string str_outpmt_recipientID =
                OTAPI_Wrap::It()->GetNym_OutpaymentsRecipientIDByIndex(str_nym_id, nCurrentOutpayment);
            // ----------------------------------
            // str_outpmt_server is the server for this outpayment.
            // But is that server on our list of servers that we care about?
            // Let's see if that server is on m_servers (otherwise we can skip it.)
            // Also, let's do the same for asset types.
            //
            list_of_strings::iterator it_server = std::find(m_servers.begin(), m_servers.end(), str_outpmt_server);

            if (it_server != m_servers.end()) // Found the serverID on the list of servers we care about.
            {
                // ---------------------------------------------------
                // TODO OPTIMIZE: instead of looking up the Nym's name every time, look it
                // up ONCE when first adding the NymID. Add it to a map, instead of a list,
                // and add the Nym's name as the second item in the map's pair.
                // (Just like I already did with the asset type.)
                //
                OTString strName(m_pLookup->GetNymName(str_outpmt_recipientID, &(*it_server))), strNameTemp;
                std::string  str_name;

                if (strName.Exists())
                    strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                else
                    strNameTemp.Format(MC_UI_TEXT_TO, str_outpmt_recipientID.c_str());

                str_name = strNameTemp.Get();
                // ---------------------------------------------------
                OTString strMemo;
                if (theOutPayment.GetMemo(strMemo))
                {
                    str_memo = strMemo.Get();
                }
                // ----------------------------------
                // For the "date" on this record we're using the "valid from" date on the instrument.
                std::string str_date = "0";
                time_t      tFrom    =  0 ;
                time_t      tTo      =  0 ;

                if (theOutPayment.GetValidFrom(tFrom))
                {
                    const long lFrom = static_cast<long>(tFrom);
                    OTString strFrom;
                    strFrom.Format("%ld", lFrom);
                    str_date = strFrom.Get();
                }
                theOutPayment.GetValidTo(tTo);
                // ---------------------------------------------------
                // Instrument type (cheque, voucher, etc)
                //
                int nType = static_cast<int> (theOutPayment.GetType());

                const std::string & str_type = MTRecord_GetTypeString(nType);
                // ---------------------------------------------------
                // CREATE A MTRecord AND POPULATE IT...
                //
                OTLog::vOutput(0, "%s: ADDED: pending outgoing instrument (str_type: %s)\n",
                               __FUNCTION__, str_type.c_str());

                shared_ptr_MTRecord sp_Record(new MTRecord(*it_server,
                                                           *p_str_asset_type,
                                                           *p_str_asset_name,
                                                            str_nym_id,    // This is the Nym WHOSE BOX IT IS.
                                                           *p_str_account, // This is the Nym's account according to the payment instrument, IF that account was found on our list of accounts we care about. Or it's blank if no account was found on the payment instrument.
                                                           // Everything above this line, it stores a reference to an external string.
                                                           // -----------------------------
                                                           // Everything below this line, it makes its own internal copy of the string.
                                                           str_name, // name of recipient (since its in outpayments box.)
                                                           str_date, // the "valid from" date on the instrument.
                                                           str_amount,
                                                           str_type, // cheque, voucher, smart contract, etc
                                                           true,   //bIsPending=true since its in the outpayments box.
                                                           true,   //bIsOutgoing=true. Otherwise it'd be in record box already.
                                                           false,//IsRecord
                                                           false,//IsReceipt
                                                           MTRecord::Instrument
                                                           ));
                // -------------------------------------------------
                sp_Record->SetContents(strOutpayment.Get());
                // -------------------------------------------------
                sp_Record->SetOtherNymID(str_outpmt_recipientID);
                // -------------------------------------------------
                if (!str_memo.empty())
                    sp_Record->SetMemo(str_memo);
                // -------------------------------------------------
                sp_Record->SetDateRange(tFrom, tTo);
                sp_Record->SetBoxIndex(static_cast<int>(nCurrentOutpayment));
                // -------------------------------------------------
                int64_t lTransNum = 0;
                theOutPayment.GetOpeningNum(lTransNum, theNymID);
                // -------------------------------------------------
                sp_Record->SetTransactionNum(lTransNum);
                m_contents.push_back(sp_Record);
            }
            else // the server for this outpayment is not on the list of servers we care about. Skip this outpayment.
            {
                OTLog::vOutput(0, "%s: Skipping outgoing instrument (we don't care about server %s)\n",
                               __FUNCTION__, str_outpmt_server.c_str());
                continue;
            }
        } // for outpayments.
        // ------------------------------------------------
        // For each Nym, loop through his MAIL box.
        //
        const int32_t nMailCount  = OTAPI_Wrap::It()->GetNym_MailCount(str_nym_id);
        for ( int32_t nCurrentMail = 0; nCurrentMail < nMailCount; ++nCurrentMail)
        {
            // ------------------------------------------------
            OTLog::vOutput(0, "%s: Mail index: %d\n", __FUNCTION__, nCurrentMail);
            // ------------------------------------------------
            OTMessage *	pMsg = pNym->GetMailByIndex(static_cast<int>(nCurrentMail));
            OT_ASSERT(NULL != pMsg);
            // ------------------------------------------------
            const std::string str_mail_server =
                OTAPI_Wrap::It()->GetNym_MailServerIDByIndex(str_nym_id, nCurrentMail);
            // ----------------------------------
            const std::string str_mail_senderID =
                OTAPI_Wrap::It()->GetNym_MailSenderIDByIndex(str_nym_id, nCurrentMail);
            // ----------------------------------
            // str_mail_server is the server for this mail.
            // But is that server on our list of servers that we care about?
            // Let's see if that server is on m_servers (otherwise we can skip it.)
            //
            list_of_strings::iterator it_server = std::find(m_servers.begin(), m_servers.end(), str_mail_server);

            if (it_server != m_servers.end()) // Found the serverID on the list of servers we care about.
            {
                // ---------------------------------------------------
                // TODO OPTIMIZE: instead of looking up the Nym's name every time, look it
                // up ONCE when first adding the NymID. Add it to a map, instead of a list,
                // and add the Nym's name as the second item in the map's pair.
                // (Just like I already did with the asset type.)
                //
                OTString strName(m_pLookup->GetNymName(str_mail_senderID, &(*it_server))), strNameTemp;
                std::string  str_name;

                if (strName.Exists())
                    strNameTemp.Format(MC_UI_TEXT_FROM, strName.Get());
                else
                    strNameTemp.Format(MC_UI_TEXT_FROM, str_mail_senderID.c_str());

                str_name = strNameTemp.Get(); // Todo: lookup the name in address book also.
                // ---------------------------------------------------
                const std::string * p_str_asset_type = &MTRecordList::s_blank; // <========== ASSET TYPE
                const std::string * p_str_asset_name = &MTRecordList::s_blank; // asset type display name.
                const std::string * p_str_account    = &MTRecordList::s_blank; // <========== ACCOUNT

                std::string str_amount; // There IS NO amount, on mail. (So we leave this empty.)

                long lDate = pMsg->m_lTime;
                OTString strDate;
                strDate.Format("%ld", lDate);
                const std::string str_date(strDate.Get());
                // ---------------------------------------------------
                // CREATE A MTRecord AND POPULATE IT...
                //
                OTLog::vOutput(0, "%s: ADDED: incoming mail.\n", __FUNCTION__);

                shared_ptr_MTRecord sp_Record(new MTRecord(*it_server,
                                                           *p_str_asset_type,
                                                           *p_str_asset_name,
                                                           str_nym_id,     // This is the Nym WHOSE BOX IT IS.
                                                           *p_str_account, // This is the Nym's account according to the payment instrument, IF that account was found on our list of accounts we care about. Or it's blank if no account was found on the payment instrument.
                                                           // Everything above this line, it stores a reference to an external string.
                                                           // -----------------------------
                                                           // Everything below this line, it makes its own internal copy of the string.
                                                           str_name, // name of sender (since its in incoming mail box.)
                                                           str_date, // How do we get the date from a mail?
                                                           str_amount,
                                                           MTRecordList::s_message_type, // "message"
                                                           false, //bIsPending=false since its already received.
                                                           false, //bIsOutgoing=false. It's incoming mail, not outgoing mail.
                                                           false, //IsRecord
                                                           false, //IsReceipt
                                                           MTRecord::Mail
                                                           ));
                const OTString strMail(OTAPI_Wrap::It()->GetNym_MailContentsByIndex(str_nym_id, nCurrentMail));
                sp_Record->SetContents(strMail.Get());
                // -------------------------------------------------
                sp_Record->SetOtherNymID(str_mail_senderID);
                // -------------------------------------------------
                sp_Record->SetBoxIndex(static_cast<int>(nCurrentMail));
                // -------------------------------------------------
                sp_Record->SetDateRange(static_cast<time_t>(pMsg->m_lTime), static_cast<time_t>(pMsg->m_lTime));
                // -------------------------------------------------
                m_contents.push_back(sp_Record);
            }
        } // loop through incoming Mail.
        // ------------------------------------------------
        // Outmail
        //
        const int32_t nOutmailCount   = OTAPI_Wrap::It()->GetNym_OutmailCount(str_nym_id);
        for ( int32_t nCurrentOutmail = 0; nCurrentOutmail < nOutmailCount; ++nCurrentOutmail)
        {
            // ------------------------------------------------
            OTLog::vOutput(0, "%s: Outmail index: %d\n", __FUNCTION__, nCurrentOutmail);
            // ------------------------------------------------
            OTMessage *	pMsg = pNym->GetOutmailByIndex(static_cast<int>(nCurrentOutmail));
            OT_ASSERT(NULL != pMsg);
            // ------------------------------------------------
            const std::string str_mail_server =
                OTAPI_Wrap::It()->GetNym_OutmailServerIDByIndex(str_nym_id, nCurrentOutmail);
            // ----------------------------------
            const std::string str_mail_recipientID =
                OTAPI_Wrap::It()->GetNym_OutmailRecipientIDByIndex(str_nym_id, nCurrentOutmail);
            // ----------------------------------
            // str_mail_server is the server for this mail.
            // But is that server on our list of servers that we care about?
            // Let's see if that server is on m_servers (otherwise we can skip it.)
            //
            list_of_strings::iterator it_server = std::find(m_servers.begin(), m_servers.end(), str_mail_server);

            if (it_server != m_servers.end()) // Found the serverID on the list of servers we care about.
            {
                // ---------------------------------------------------
                // TODO OPTIMIZE: instead of looking up the Nym's name every time, look it
                // up ONCE when first adding the NymID. Add it to a map, instead of a list,
                // and add the Nym's name as the second item in the map's pair.
                // (Just like I already did with the asset type.)
                //
                OTString strName(m_pLookup->GetNymName(str_mail_recipientID, &(*it_server))), strNameTemp;
                std::string  str_name;

                if (strName.Exists())
                    strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                else
                    strNameTemp.Format(MC_UI_TEXT_TO, str_mail_recipientID.c_str());

                str_name = strNameTemp.Get();
                // ---------------------------------------------------
                const std::string * p_str_asset_type = &MTRecordList::s_blank; // <========== ASSET TYPE
                const std::string * p_str_asset_name = &MTRecordList::s_blank; // asset type display name.
                const std::string * p_str_account    = &MTRecordList::s_blank; // <========== ACCOUNT

                std::string str_amount; // There IS NO amount, on mail. (So we leave this empty.)

                long lDate = pMsg->m_lTime;
                OTString strDate;
                strDate.Format("%ld", lDate);
                const std::string str_date(strDate.Get());
                // ---------------------------------------------------
                // CREATE A MTRecord AND POPULATE IT...
                //
                OTLog::vOutput(0, "%s: ADDED: sent mail.\n", __FUNCTION__);

                shared_ptr_MTRecord sp_Record(new MTRecord(*it_server,
                                                           *p_str_asset_type,
                                                           *p_str_asset_name,
                                                           str_nym_id,     // This is the Nym WHOSE BOX IT IS.
                                                           *p_str_account, // This is the Nym's account according to the payment instrument, IF that account was found on our list of accounts we care about. Or it's blank if no account was found on the payment instrument.
                                                           // Everything above this line, it stores a reference to an external string.
                                                           // -----------------------------
                                                           // Everything below this line, it makes its own internal copy of the string.
                                                           str_name, // name of recipient (since its in outgoing mail box.)
                                                           str_date, // How do we get the date from a mail?
                                                           str_amount,
                                                           MTRecordList::s_message_type, // "message"
                                                           false, //bIsPending=false since its already sent.
                                                           true,  //bIsOutgoing=true. It's OUTGOING mail.
                                                           false, //IsRecord (it's not in the record box.)
                                                           false, //IsReceipt
                                                           MTRecord::Mail
                                                           ));
                const OTString strOutmail(OTAPI_Wrap::It()->GetNym_OutmailContentsByIndex(str_nym_id, nCurrentOutmail));
                sp_Record->SetContents(strOutmail.Get());
                // -------------------------------------------------
                sp_Record->SetBoxIndex(static_cast<int>(nCurrentOutmail));
                // -------------------------------------------------
                sp_Record->SetOtherNymID(str_mail_recipientID);
                // -------------------------------------------------
                sp_Record->SetDateRange(static_cast<time_t>(pMsg->m_lTime), static_cast<time_t>(pMsg->m_lTime));
                // -------------------------------------------------
                m_contents.push_back(sp_Record);
            }
        } // loop through outgoing Mail.
        // ------------------------------------------------
        // For each nym, for each server, loop through its payments inbox and record box.
        //
        int nServerIndex = -1;
        FOR_EACH_IT(list_of_strings, m_servers, it_server)
        {
            ++nServerIndex;
            // --------------------------
            const OTIdentifier theServerID(*it_server);
            OTServerContract * pServer = pWallet->GetServerContract(theServerID);
            OT_ASSERT(NULL != pServer);
            // ------------------------------------------------
            const OTString strServerID(theServerID);
            OTLog::vOutput(0, "%s: Server %d, ID: %s\n", __FUNCTION__, nServerIndex, strServerID.Get());
            // ------------------------------------------------
            // OPTIMIZE FYI:
            // The "NoVerify" version is much faster, but you will lose the ability to get the
            // sender/recipient name from the receipts in the box. The code will, however, work
            // either way.
            //
            OTLedger * pInbox = m_bRunFast ? OTAPI_Wrap::OTAPI()->LoadPaymentInboxNoVerify(theServerID, theNymID) :
                                             OTAPI_Wrap::OTAPI()->LoadPaymentInbox        (theServerID, theNymID);
            OTCleanup<OTLedger> theInboxAngel(pInbox);

            int32_t  nIndex = (-1);
            // It loaded up, so let's loop through it.
            if (NULL != pInbox) FOR_EACH(mapOfTransactions, pInbox->GetTransactionMap())
            {
                OTTransaction * pBoxTrans = (*it).second;
                OT_ASSERT(NULL != pBoxTrans);
                // ------------------------------
                ++nIndex; // 0 on first iteration.
                // ------------------------------------------------
                OTLog::vOutput(0, "%s: Incoming payment: %d\n", __FUNCTION__, nIndex);
                // ------------------------------------------------
                std::string  str_name; // name of sender (since its in the payments inbox.)
                std::string  str_sender_nym_id;
                std::string  str_sender_acct_id;

                if (false == pBoxTrans->IsAbbreviated())
                {
                    OTIdentifier theSenderID;

                    if (pBoxTrans->GetSenderUserIDForDisplay(theSenderID))
                    {
                        const OTString strSenderID(theSenderID);
                        str_sender_nym_id = strSenderID.Get();

                        OTString strName(m_pLookup->GetNymName(str_sender_nym_id, &(*it_server))), strNameTemp;

                        if (strName.Exists())
                            strNameTemp.Format(MC_UI_TEXT_FROM, strName.Get());
                        else
                            strNameTemp.Format(MC_UI_TEXT_FROM, str_sender_nym_id.c_str());

                        str_name = strNameTemp.Get(); // Todo: lookup the name in address book also.
                    }

                    theSenderID.Release();

                    if (pBoxTrans->GetSenderAcctIDForDisplay(theSenderID))
                    {
                        const OTString strSenderID(theSenderID);
                        str_sender_acct_id = strSenderID.Get();
                    }
                }
                // ------------------------------
                time_t  tValidFrom = 0, tValidTo = 0;
                // ----------------------------------
                std::string str_date    = "0"; // the "date signed" on the transaction receipt.
                time_t      tDateSigned = pBoxTrans->GetDateSigned();

                if (tDateSigned > 0)
                {
                    tValidFrom = tDateSigned;
                    const long lDateSigned = static_cast<long>(tDateSigned);
                    OTString strDateSigned;
                    strDateSigned.Format("%ld", lDateSigned);
                    str_date = strDateSigned.Get();
                }
                // ----------------------------------
                const std::string * p_str_asset_type = &MTRecordList::s_blank; // <========== ASSET TYPE
                const std::string * p_str_asset_name = &MTRecordList::s_blank; // asset type display name.
                // ----------------------------------
                std::string str_amount;   // <========== AMOUNT
                std::string str_type;     // Instrument type.
                std::string str_memo;
                OTString    strContents; // Instrument contents.

                if (pBoxTrans->IsAbbreviated())
                {
                    str_type = pBoxTrans->GetTypeString(); // instrumentNotice, etc.
                    // --------------------------------------------------
                    long lAmount = pBoxTrans->GetAbbrevDisplayAmount();

                    if (0 != lAmount)
                    {
                        OTString strTemp;
                        strTemp.Format("%ld", lAmount);
                        str_amount = strTemp.Get();
                    }
                }
                else // NOT abbreviated. (Full box receipt is already loaded.)
                {
                    OTPayment * pPayment = pInbox->GetInstrument(*pNym,
                                                                 nIndex);     // ===> Returns financial instrument by index.
                    OTCleanup<OTPayment> thePaymentAngel(pPayment);
                    // ------------------------------
                    if (NULL == pPayment) // then we treat it like it's abbreviated.
                    {
                        str_type = pBoxTrans->GetTypeString(); // instrumentNotice, etc.
                        // --------------------------------------------------
                        long lAmount = pBoxTrans->GetAbbrevDisplayAmount();

                        if (0 == lAmount)
                            lAmount = pBoxTrans->GetReceiptAmount();
                        // -----------------------------------------
                        if (0 != lAmount)
                        {
                            OTString strTemp;
                            strTemp.Format("%ld", lAmount);
                            str_amount = strTemp.Get();
                        }
                    }
                    // ---------------------------------------------------
                    // We have pPayment, the instrument accompanying the receipt in the payments inbox.
                    //
                    else if (pPayment->SetTempValues())
                    {
                        // ----------------------------------
                        pPayment->GetValidFrom(tValidFrom);
                        pPayment->GetValidTo  (tValidTo);

                        if (tValidFrom > 0)
                        {
                            const long lFrom = static_cast<long>(tValidFrom);
                            OTString strFrom;
                            strFrom.Format("%ld", lFrom);
                            str_date = strFrom.Get();
                        }
                        // ----------------------------------
                        OTString strMemo;
                        if (pPayment->GetMemo(strMemo))
                        {
                            str_memo = strMemo.Get();
                        }
                        // ----------------------------------
                        pPayment->GetPaymentContents(strContents);
                        // -----------------------------
                        OTIdentifier theAssetTypeID, theSenderAcctID;

                        if (pPayment->GetAssetTypeID(theAssetTypeID))
                        {
                            OTString strTemp(theAssetTypeID);
                            const std::string str_inpmt_asset(strTemp.Get()); // The asset type we found on the payment (if we found anything.)
                            // -----------------------------
                            map_of_strings::iterator it_asset = m_assets.find(str_inpmt_asset);
                            // -----------------------------
                            if (it_asset != m_assets.end()) // Found it on the map of asset types we care about.
                            {
                                p_str_asset_type = &(it_asset->first);   // Set the asset type ID.
                                p_str_asset_name = &(it_asset->second);  // The CurrencyTLA. Examples: USD, BTC, etc.
                            }
                            else
                            {
                                // There was definitely an asset type on the instrument, and it definitely
                                // did not match any of the assets that we care about.
                                // Therefore, skip.
                                //
                                OTLog::vError("%s: Skipping: Incoming payment (we don't care about asset %s)\n",
                                              __FUNCTION__, str_inpmt_asset.c_str());
                                continue;
                            }
                        }
                        // --------------------------------------------------
                        if (str_sender_acct_id.empty() && pPayment->GetSenderAcctIDForDisplay(theSenderAcctID))
                        {
                            OTString strTemp(theSenderAcctID);
                            str_sender_acct_id = strTemp.Get();
                        }
                        // --------------------------------------------------
                        // By this point, p_str_asset_type and p_str_asset_name are definitely set.
                        OT_ASSERT(NULL != p_str_asset_type); // and it's either blank, or it's one of the asset types we care about.
                        OT_ASSERT(NULL != p_str_asset_name); // and it's either blank, or it's one of the asset types we care about.
                        // ---------------------------------------------------
                        // Instrument type (cheque, voucher, etc)
                        int nType = static_cast<int> (pPayment->GetType());

                        str_type = MTRecord_GetTypeString(nType);
                        // ---------------------------------------------------
                        int64_t lAmount = 0;

                        if (pPayment->GetAmount(lAmount))
                        {
                            OTString strTemp;
                            strTemp.Format("%ld", lAmount);
                            str_amount = strTemp.Get();
                        }
                    }
                }
                // ------------------------------
                OTLog::vOutput(0, "%s: ADDED: pending incoming payment (str_type: %s)\n",
                               __FUNCTION__, str_type.c_str());

                shared_ptr_MTRecord sp_Record(new MTRecord(*it_server,
                                                           *p_str_asset_type,
                                                           *p_str_asset_name,
                                                           str_nym_id,     // This is the Nym WHOSE BOX IT IS.
                                                           MTRecordList::s_blank, // This is the Nym's account for this box. (Blank for payments inbox.)
                                                           // Everything above this line, it stores a reference to an external string.
                                                           // -----------------------------
                                                           // Everything below this line, it makes its own internal copy of the string.
                                                           str_name, // name of sender (since its in the inbox.)
                                                           str_date, // the "valid from" date on the instrument.
                                                           str_amount,
                                                           str_type, // pending, chequeReceipt, etc.
                                                           true,     // I believe all incoming "payment inbox" items are pending. (Cheques waiting to be cashed, smart contracts waiting to be signed, etc.)
                                                           false,    // bIsOutgoing=false. (Since this is the payment INbox, nothing is outgoing...)
                                                           false, //bIsRecord
                                                           false, //bIsReceipt
                                                           MTRecord::Instrument));
                // -------------------------------------------------
                if (strContents.Exists())
                    sp_Record->SetContents(strContents.Get());
                // -------------------------------------------------
                sp_Record->SetDateRange(tValidFrom, tValidTo);
                // -------------------------------------------------
                sp_Record->SetBoxIndex(static_cast<int>(nIndex));
                // -------------------------------------------------
                if (!str_memo.empty())
                    sp_Record->SetMemo(str_memo);
                // -------------------------------------------------
                if (!str_sender_nym_id.empty())
                    sp_Record->SetOtherNymID(str_sender_nym_id);
                // -------------------------------------------------
                if (!str_sender_acct_id.empty())
                    sp_Record->SetOtherAccountID(str_sender_acct_id);
                // -------------------------------------------------
                sp_Record->SetTransNumForDisplay(pBoxTrans->GetReferenceNumForDisplay());
                sp_Record->SetTransactionNum(pBoxTrans->GetTransactionNum());

                m_contents.push_back(sp_Record);
                // ------------------------------

            } // looping through inbox.
            else
                OTLog::vOutput(1, "%s: Failed loading payments inbox. (Probably just doesn't exist yet.)\n", __FUNCTION__);
            // ------------------------------------------------
            nIndex = (-1);

            // Also loop through its record box. For this record box, pass the USER_ID twice,
            // since it's the recordbox for the Nym.
            // OPTIMIZE FYI: m_bRunFast impacts run speed here.
            OTLedger * pRecordbox = m_bRunFast ? OTAPI_Wrap::OTAPI()->LoadRecordBoxNoVerify(theServerID, theNymID, theNymID) : // twice.
                                                 OTAPI_Wrap::OTAPI()->LoadRecordBox        (theServerID, theNymID, theNymID);
            OTCleanup<OTLedger> theRecordBoxAngel(pRecordbox);

            // It loaded up, so let's loop through it.
            if (NULL != pRecordbox) FOR_EACH(mapOfTransactions, pRecordbox->GetTransactionMap())
            {
                OTTransaction * pBoxTrans = (*it).second;
                OT_ASSERT(NULL != pBoxTrans);
                // ------------------------------
                bool bOutgoing = false;
                // ----------------------------------
                ++nIndex; // 0 on first iteration.
                // ------------------------------------------------
                OTLog::vOutput(0, "%s: Payment RECORD index: %d\n", __FUNCTION__, nIndex);
                // ------------------------------------------------
                std::string  str_name; // name of sender OR recipient (depending on whether it was originally incoming or outgoing.)
                std::string  str_other_nym_id;
                std::string  str_other_acct_id;

                if (false == pBoxTrans->IsAbbreviated())
                {
                    OTIdentifier theSenderID,    theSenderAcctID;
                    OTIdentifier theRecipientID, theRecipientAcctID;

                    if (pBoxTrans->GetSenderUserIDForDisplay(theSenderID))
                    {
                        const OTString    strSenderID  (theSenderID);
                        const std::string str_sender_id(strSenderID.Get());

                        // Usually, Nym is the RECIPIENT. Sometimes he's the sender.
                        // Either way, we want the OTHER ID (the other Nym) for display.
                        // So here, if Nym's CLEARLY the sender, then we want the RECIPIENT.
                        // Whereas if Nym were the recipient, then we'd want the SENDER. (For display.)
                        //
                        if (0 == str_nym_id.compare(str_sender_id)) // str_nym_id IS str_sender_id. (Therefore we want recipient.)
                        {
                            bOutgoing = true; // if Nym is the sender, then it must have been outgoing.

                            if (pBoxTrans->GetRecipientUserIDForDisplay(theRecipientID))
                            {
                                const OTString strRecipientID(theRecipientID);
                                const std::string str_recipient_id(strRecipientID.Get());

                                OTString strName(m_pLookup->GetNymName(str_recipient_id, &(*it_server))), strNameTemp;

                                if (strName.Exists())
                                    strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                                else
                                    strNameTemp.Format(MC_UI_TEXT_TO, str_recipient_id.c_str());

                                str_name         = strNameTemp.Get(); // Todo: lookup the name in address book also.
                                str_other_nym_id = str_recipient_id;
                                // -------------------------------------------
                                if (pBoxTrans->GetRecipientAcctIDForDisplay(theRecipientAcctID))
                                {
                                    const OTString strRecipientAcctID(theRecipientAcctID);
                                    str_other_acct_id = strRecipientAcctID.Get();
                                }
                            }
                        }
                        else // str_nym_id IS NOT str_sender_id. (Therefore we want sender.)
                        {    // In this case, some OTHER Nym is the sender, so it must have been incoming. (And bOutgoing is already false.)

                            OTString strName(m_pLookup->GetNymName(str_sender_id, &(*it_server))), strNameTemp;

                            if (strName.Exists())
                                strNameTemp.Format(MC_UI_TEXT_FROM, strName.Get());
                            else
                                strNameTemp.Format(MC_UI_TEXT_FROM, str_sender_id.c_str());

                            str_name         = strNameTemp.Get(); // Todo: lookup the name in address book also.
                            str_other_nym_id = str_sender_id;
                            // -------------------------------------------
                            if (pBoxTrans->GetSenderAcctIDForDisplay(theSenderAcctID))
                            {
                                const OTString strSenderAcctID(theSenderAcctID);
                                str_other_acct_id = strSenderAcctID.Get();
                            }
                        }
                    }
                    // In this block below, we already KNOW GetSenderUserIDForDisplay is EMPTY.
                    // (So it's "recipient or bust.")
                    else if (pBoxTrans->GetRecipientUserIDForDisplay(theRecipientID))
                    {
                        const OTString strRecipientID(theRecipientID);
                        const std::string str_recipient_id(strRecipientID.Get());

                        if (0 != str_nym_id.compare(str_recipient_id)) // str_nym_id is NOT str_recipient_id. (Therefore we want str_recipient_id.)
                        {
                            // If Nym is not the recipient, then he must be the sender.
                            // (Therefore it must be outgoing.)
                            bOutgoing = true;

                            OTString strName(m_pLookup->GetNymName(str_recipient_id, &(*it_server))), strNameTemp;

                            if (strName.Exists())
                                strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                            else
                                strNameTemp.Format(MC_UI_TEXT_TO, str_recipient_id.c_str());

                            str_name         = strNameTemp.Get(); // Todo: lookup the name in address book also.
                            str_other_nym_id = str_recipient_id;
                            // -------------------------------------------
                            if (pBoxTrans->GetRecipientAcctIDForDisplay(theRecipientAcctID))
                            {
                                const OTString strRecipientAcctID(theRecipientAcctID);
                                str_other_acct_id = strRecipientAcctID.Get();
                            }
                        }
                    }
                } // if not abbreviated.
                // ------------------------------
                time_t tValidFrom = 0, tValidTo = 0;
                // ------------------------------
                std::string str_date    = "0"; // the "date signed" on the transaction receipt.
                time_t      tDateSigned = pBoxTrans->GetDateSigned();

                if (tDateSigned > 0)
                {
                    tValidFrom = tDateSigned;
                    const long lDateSigned = static_cast<long>(tDateSigned);
                    OTString strDateSigned;
                    strDateSigned.Format("%ld", lDateSigned);
                    str_date = strDateSigned.Get();
                }
                // ----------------------------------
                const std::string * p_str_asset_type = &MTRecordList::s_blank; // <========== ASSET TYPE
                const std::string * p_str_asset_name = &MTRecordList::s_blank; // asset type display name.
                const std::string * p_str_account    = &MTRecordList::s_blank; // <========== ACCOUNT
                // ----------------------------------
                std::string str_amount;  // <========== AMOUNT
                std::string str_type;    // Instrument type.
                std::string str_memo;    // Instrument memo (if applicable.)
                OTString    strContents; // Instrument contents.

                if (pBoxTrans->IsAbbreviated())
                {
                    str_type = pBoxTrans->GetTypeString(); // instrumentNotice, etc.
                    // --------------------------------------------------
                    long lAmount = pBoxTrans->GetAbbrevDisplayAmount();

                    if (0 != lAmount)
                    {
                        OTString strTemp;
                        strTemp.Format("%ld", lAmount);
                        str_amount = strTemp.Get();
                    }
                }
                else // NOT abbreviated. (Full box receipt is already loaded.)
                {
                    OTPayment * pPayment = pRecordbox->GetInstrument(*pNym,
                                                                     nIndex);     // ===> Returns financial instrument by index.
                    OTCleanup<OTPayment> thePaymentAngel(pPayment);
                    // ------------------------------
                    if (NULL == pPayment) // then we treat it like it's abbreviated.
                    {
                        str_type = pBoxTrans->GetTypeString(); // instrumentNotice, etc.
                        // --------------------------------------------------
                        long lAmount = pBoxTrans->GetAbbrevDisplayAmount();

                        if (0 != lAmount)
                        {
                            OTString strTemp;
                            strTemp.Format("%ld", lAmount);
                            str_amount = strTemp.Get();
                        }
                    }
                    // ---------------------------------------------------
                    // We have pPayment, the instrument accompanying the receipt in the payments recordbox.
                    else if (pPayment->SetTempValues())
                    {
                        // ----------------------------------
                        pPayment->GetValidFrom(tValidFrom);
                        pPayment->GetValidTo  (tValidTo);

                        if (tValidFrom > 0)
                        {
                            const long lFrom = static_cast<long>(tValidFrom);
                            OTString strFrom;
                            strFrom.Format("%ld", lFrom);
                            str_date = strFrom.Get();
                        }
                        // ----------------------------------
                        pPayment->GetPaymentContents(strContents);
                        // ------------------------------------
                        OTIdentifier theAccountID;

                        if (bOutgoing) // Nym is sender.
                        {
                            if (pPayment->GetSenderAcctIDForDisplay(theAccountID)) // Since Nym is ME, the Account must be MY acct.
                            {                                            // (If this record was originally OUTgoing, then the SENDER's account is MY acct.)
                                OTString strTemp(theAccountID);
                                std::string str_outpmt_account = strTemp.Get(); // The accountID we found on the payment (only applies to outgoing payments.)
                                // -----------------------------
                                list_of_strings::iterator it_acct = std::find(m_accounts.begin(), m_accounts.end(), str_outpmt_account);
                                // -----------------------------
                                if (it_acct != m_accounts.end()) // Found it on the list of accounts we care about.
                                {
                                    p_str_account = &(*it_acct);
                                }
                                else
                                {
                                    // There was definitely an account on the instrument, and it definitely
                                    // did not match any of the accounts that we care about.
                                    // Therefore, skip.
                                    //
                                    OTLog::vOutput(0, "%s: Skipping 'sent payment' record. (We don't care about account %s)\n",
                                                   __FUNCTION__, str_outpmt_account.c_str());
                                    continue;
                                }
                                // -------------------------------------------
                            }
                        }
                        else // Nym is recipient.
                        {
                            // Why is this here? Because if Nym is recipient, let's say he received an instrumentNotice containing
                            // a sendUserInstrument message containing an incoming cheque. Well... that incoming cheque (the payload
                            // on sendUserInstrument message) is ENCRYPTED. Meaning the above calls to pBoxReceipt->GetSenderAcctID
                            // on the instrumentNotice transaction will FAIL. One option is to pass pNym into GetSenderAcctID so it
                            // can decrypt the payload and return the value. But since we already have the payload decrypted here
                            // (we already have the cheque loaded up here) we can just grab the senderAcctID directly from the cheque.
                            // That's why this is here -- because this is where we KNOW we have the account ID -- so we grab it.
                            //
                            if (str_other_acct_id.empty() && (pPayment->GetSenderAcctIDForDisplay(theAccountID)))
                            {
                                OTString strTemp(theAccountID);
                                str_other_acct_id = strTemp.Get();
                            }
                        }
                        // ------------------------------------------
                        // By this point, p_str_account is definitely set.
                        OT_ASSERT(NULL != p_str_account); // and it's either blank, or it's one of the accounts we care about.
                        // ---------------------------------------------------
                        OTIdentifier theAssetTypeID;

                        if (pPayment->GetAssetTypeID(theAssetTypeID))
                        {
                            OTString strTemp(theAssetTypeID);
                            const std::string str_inpmt_asset(strTemp.Get()); // The asset type we found on the payment (if we found anything.)
                            // -----------------------------
                            map_of_strings::iterator it_asset = m_assets.find(str_inpmt_asset);
                            // -----------------------------
                            if (it_asset != m_assets.end()) // Found it on the map of asset types we care about.
                            {
                                p_str_asset_type = &(it_asset->first);   // Set the asset type ID.
                                p_str_asset_name = &(it_asset->second);  // The CurrencyTLA. Examples: USD, BTC, etc.
                            }
                            else
                            {
                                // There was definitely an asset type on the instrument, and it definitely
                                // did not match any of the assets that we care about.
                                // Therefore, skip.
                                //
                                OTLog::vError("%s: Skipping: Payment record (we don't care about asset type %s)\n",
                                              __FUNCTION__, str_inpmt_asset.c_str());
                                continue;
                            }
                        }
                        // By this point, p_str_asset_type and p_str_asset_name are definitely set.
                        OT_ASSERT(NULL != p_str_asset_type); // and it's either blank, or it's one of the asset types we care about.
                        OT_ASSERT(NULL != p_str_asset_name); // and it's either blank, or it's one of the asset types we care about.
                        // ----------------------------------
                        OTString strMemo;
                        if (pPayment->GetMemo(strMemo))
                        {
                            str_memo = strMemo.Get();
                        }
                        // ----------------------------------
                        // Instrument type (cheque, voucher, etc)
                        int nType = static_cast<int> (pPayment->GetType());

                        str_type = MTRecord_GetTypeString(nType);
                        // ---------------------------------------------------
                        int64_t lAmount = 0;

                        if (pPayment->GetAmount(lAmount))
                        {
                            OTString strTemp;
                            strTemp.Format("%ld", lAmount);
                            str_amount = strTemp.Get();
                        }
                    }
                }
                // ------------------------------
                OTLog::vOutput(0, "%s: ADDED: Payment record %s (str_type: %s)\n",
                               __FUNCTION__, bOutgoing ? "(sent)" : "(received)", str_type.c_str());

                shared_ptr_MTRecord sp_Record(new MTRecord(*it_server,
                                                           *p_str_asset_type,
                                                           *p_str_asset_name,
                                                           str_nym_id,     // This is the Nym WHOSE BOX IT IS.
                                                           *p_str_account, // This is the Nym's account for this box. (Blank for incoming, set for outgoing.)
                                                           // Everything above this line, it stores a reference to an external string.
                                                           // -----------------------------
                                                           // Everything below this line, it makes its own internal copy of the string.
                                                           str_name,    // name of sender or recipient (since its in the recordbox.)
                                                           str_date,    // the "date signed" on the receipt.
                                                           str_amount,
                                                           str_type,    // pending, chequeReceipt, etc.
                                                           false,       // Everything in the recordbox is finished. (NOT pending.)
                                                           bOutgoing, // Since it's the recordbox, it contains both incoming and outgoing receipts.
                                                           true, //IsRecord
                                                           false,//IsReceipt,
                                                           MTRecord::Instrument));
                // -------------------------------------------------
                if (strContents.Exists())
                    sp_Record->SetContents(strContents.Get());
                // -------------------------------------------------
                sp_Record->SetDateRange(tValidFrom, tValidTo);
                // -------------------------------------------------
                sp_Record->SetBoxIndex(static_cast<int>(nIndex));
                // -------------------------------------------------
                if (!str_memo.empty())
                    sp_Record->SetMemo(str_memo);
                // -------------------------------------------------
                if (!str_other_nym_id.empty())
                    sp_Record->SetOtherNymID(str_other_nym_id);
                // -------------------------------------------------
                if (!str_other_acct_id.empty())
                    sp_Record->SetOtherAccountID(str_other_acct_id);
                // -------------------------------------------------
                sp_Record->SetTransNumForDisplay(pBoxTrans->GetReferenceNumForDisplay());
                sp_Record->SetTransactionNum(pBoxTrans->GetTransactionNum());

                m_contents.push_back(sp_Record);
                // ------------------------------

            } // Loop through Recordbox
            else
                OTLog::vOutput(1, "%s: Failed loading payments record box. (Probably just doesn't exist yet.)\n", __FUNCTION__);
            // ------------------------------------------------

            // ------------------------------------------------
            // EXPIRED RECORDS:
            nIndex = (-1);

            // Also loop through its expired record box.
            // OPTIMIZE FYI: m_bRunFast impacts run speed here.
            OTLedger * pExpiredbox = m_bRunFast ? OTAPI_Wrap::OTAPI()->LoadExpiredBoxNoVerify(theServerID, theNymID) :
                                                  OTAPI_Wrap::OTAPI()->LoadExpiredBox        (theServerID, theNymID);
            OTCleanup<OTLedger> theExpiredBoxAngel(pExpiredbox);

            // It loaded up, so let's loop through it.
            if (NULL != pExpiredbox) FOR_EACH(mapOfTransactions, pExpiredbox->GetTransactionMap())
            {
                OTTransaction * pBoxTrans = (*it).second;
                OT_ASSERT(NULL != pBoxTrans);
                // ------------------------------
                bool bOutgoing = false;
                // ----------------------------------
                ++nIndex; // 0 on first iteration.
                // ------------------------------------------------
                OTLog::vOutput(0, "%s: Expired payment RECORD index: %d\n", __FUNCTION__, nIndex);
                // ------------------------------------------------
                std::string  str_name; // name of sender OR recipient (depending on whether it was originally incoming or outgoing.)
                std::string  str_other_nym_id;
                std::string  str_other_acct_id;

                if (false == pBoxTrans->IsAbbreviated())
                {
                    OTIdentifier theSenderID,    theSenderAcctID;
                    OTIdentifier theRecipientID, theRecipientAcctID;

                    if (pBoxTrans->GetSenderUserIDForDisplay(theSenderID))
                    {
                        const OTString    strSenderID  (theSenderID);
                        const std::string str_sender_id(strSenderID.Get());

                        // Usually, Nym is the RECIPIENT. Sometimes he's the sender.
                        // Either way, we want the OTHER ID (the other Nym) for display.
                        // So here, if Nym's CLEARLY the sender, then we want the RECIPIENT.
                        // Whereas if Nym were the recipient, then we'd want the SENDER. (For display.)
                        //
                        if (0 == str_nym_id.compare(str_sender_id)) // str_nym_id IS str_sender_id. (Therefore we want recipient.)
                        {
                            bOutgoing = true; // if Nym is the sender, then it must have been outgoing.

                            if (pBoxTrans->GetRecipientUserIDForDisplay(theRecipientID))
                            {
                                const OTString strRecipientID(theRecipientID);
                                const std::string str_recipient_id(strRecipientID.Get());

                                OTString strName(m_pLookup->GetNymName(str_recipient_id, &(*it_server))), strNameTemp;

                                if (strName.Exists())
                                    strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                                else
                                    strNameTemp.Format(MC_UI_TEXT_TO, str_recipient_id.c_str());

                                str_name         = strNameTemp.Get(); // Todo: lookup the name in address book also.
                                str_other_nym_id = str_recipient_id;
                                // -------------------------------------------
                                if (pBoxTrans->GetRecipientAcctIDForDisplay(theRecipientAcctID))
                                {
                                    const OTString strRecipientAcctID(theRecipientAcctID);
                                    str_other_acct_id = strRecipientAcctID.Get();
                                }
                            }
                        }
                        else // str_nym_id IS NOT str_sender_id. (Therefore we want sender.)
                        {    // In this case, some OTHER Nym is the sender, so it must have been incoming. (And bOutgoing is already false.)

                            OTString strName(m_pLookup->GetNymName(str_sender_id, &(*it_server))), strNameTemp;

                            if (strName.Exists())
                                strNameTemp.Format(MC_UI_TEXT_FROM, strName.Get());
                            else
                                strNameTemp.Format(MC_UI_TEXT_FROM, str_sender_id.c_str());

                            str_name         = strNameTemp.Get(); // Todo: lookup the name in address book also.
                            str_other_nym_id = str_sender_id;
                            // -------------------------------------------
                            if (pBoxTrans->GetSenderAcctIDForDisplay(theSenderAcctID))
                            {
                                const OTString strSenderAcctID(theSenderAcctID);
                                str_other_acct_id = strSenderAcctID.Get();
                            }
                        }
                    }
                    // In this block below, we already KNOW GetSenderUserIDForDisplay is EMPTY.
                    // (So it's "recipient or bust.")
                    else if (pBoxTrans->GetRecipientUserIDForDisplay(theRecipientID))
                    {
                        const OTString strRecipientID(theRecipientID);
                        const std::string str_recipient_id(strRecipientID.Get());

                        if (0 != str_nym_id.compare(str_recipient_id)) // str_nym_id is NOT str_recipient_id. (Therefore we want str_recipient_id.)
                        {
                            // If Nym is not the recipient, then he must be the sender.
                            // (Therefore it must be outgoing.)
                            bOutgoing = true;

                            OTString strName(m_pLookup->GetNymName(str_recipient_id, &(*it_server))), strNameTemp;

                            if (strName.Exists())
                                strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                            else
                                strNameTemp.Format(MC_UI_TEXT_TO, str_recipient_id.c_str());

                            str_name         = strNameTemp.Get(); // Todo: lookup the name in address book also.
                            str_other_nym_id = str_recipient_id;
                            // -------------------------------------------
                            if (pBoxTrans->GetRecipientAcctIDForDisplay(theRecipientAcctID))
                            {
                                const OTString strRecipientAcctID(theRecipientAcctID);
                                str_other_acct_id = strRecipientAcctID.Get();
                            }
                        }
                    }
                } // if not abbreviated.
                // ------------------------------
                time_t tValidFrom = 0, tValidTo = 0;
                // ------------------------------
                std::string str_date    = "0"; // the "date signed" on the transaction receipt.
                time_t      tDateSigned = pBoxTrans->GetDateSigned();

                if (tDateSigned > 0)
                {
                    tValidFrom = tDateSigned;
                    const long lDateSigned = static_cast<long>(tDateSigned);
                    OTString strDateSigned;
                    strDateSigned.Format("%ld", lDateSigned);
                    str_date = strDateSigned.Get();
                }
                // ----------------------------------
                const std::string * p_str_asset_type = &MTRecordList::s_blank; // <========== ASSET TYPE
                const std::string * p_str_asset_name = &MTRecordList::s_blank; // asset type display name.
                const std::string * p_str_account    = &MTRecordList::s_blank; // <========== ACCOUNT
                // ----------------------------------
                std::string str_amount;  // <========== AMOUNT
                std::string str_type;    // Instrument type.
                std::string str_memo;    // Instrument memo (if applicable.)
                OTString    strContents; // Instrument contents.

                if (pBoxTrans->IsAbbreviated())
                {
                    str_type = pBoxTrans->GetTypeString(); // instrumentNotice, etc.
                    // --------------------------------------------------
                    long lAmount = pBoxTrans->GetAbbrevDisplayAmount();

                    if (0 != lAmount)
                    {
                        OTString strTemp;
                        strTemp.Format("%ld", lAmount);
                        str_amount = strTemp.Get();
                    }
                }
                else // NOT abbreviated. (Full box receipt is already loaded.)
                {
                    OTPayment * pPayment = pExpiredbox->GetInstrument(*pNym,
                                                                      nIndex);     // ===> Returns financial instrument by index.
                    OTCleanup<OTPayment> thePaymentAngel(pPayment);
                    // ------------------------------
                    if (NULL == pPayment) // then we treat it like it's abbreviated.
                    {
                        str_type = pBoxTrans->GetTypeString(); // instrumentNotice, etc.
                        // --------------------------------------------------
                        long lAmount = pBoxTrans->GetAbbrevDisplayAmount();

                        if (0 != lAmount)
                        {
                            OTString strTemp;
                            strTemp.Format("%ld", lAmount);
                            str_amount = strTemp.Get();
                        }
                    }
                    // ---------------------------------------------------
                    // We have pPayment, the instrument accompanying the receipt in the payments recordbox.
                    else if (pPayment->SetTempValues())
                    {
                        // ----------------------------------
                        pPayment->GetValidFrom(tValidFrom);
                        pPayment->GetValidTo  (tValidTo);

                        if (tValidFrom > 0)
                        {
                            const long lFrom = static_cast<long>(tValidFrom);
                            OTString strFrom;
                            strFrom.Format("%ld", lFrom);
                            str_date = strFrom.Get();
                        }
                        // ----------------------------------
                        pPayment->GetPaymentContents(strContents);
                        // ------------------------------------
                        OTIdentifier theAccountID;

                        if (bOutgoing) // Nym is sender.
                        {
                            if (pPayment->GetSenderAcctIDForDisplay(theAccountID)) // Since Nym is ME, the Account must be MY acct.
                            {                                            // (If this record was originally OUTgoing, then the SENDER's account is MY acct.)
                                OTString strTemp(theAccountID);
                                std::string str_outpmt_account = strTemp.Get(); // The accountID we found on the payment (only applies to outgoing payments.)
                                // -----------------------------
                                list_of_strings::iterator it_acct = std::find(m_accounts.begin(), m_accounts.end(), str_outpmt_account);
                                // -----------------------------
                                if (it_acct != m_accounts.end()) // Found it on the list of accounts we care about.
                                {
                                    p_str_account = &(*it_acct);
                                }
                                else
                                {
                                    // There was definitely an account on the instrument, and it definitely
                                    // did not match any of the accounts that we care about.
                                    // Therefore, skip.
                                    //
                                    OTLog::vOutput(0, "%s: Skipping 'sent payment' expired record. (We don't care about account %s)\n",
                                                   __FUNCTION__, str_outpmt_account.c_str());
                                    continue;
                                }
                                // -------------------------------------------
                            }
                        }
                        else // Nym is recipient.
                        {
                            // Why is this here? Because if Nym is recipient, let's say he received an instrumentNotice containing
                            // a sendUserInstrument message containing an incoming cheque. Well... that incoming cheque (the payload
                            // on sendUserInstrument message) is ENCRYPTED. Meaning the above calls to pBoxReceipt->GetSenderAcctID
                            // on the instrumentNotice transaction will FAIL. One option is to pass pNym into GetSenderAcctID so it
                            // can decrypt the payload and return the value. But since we already have the payload decrypted here
                            // (we already have the cheque loaded up here) we can just grab the senderAcctID directly from the cheque.
                            // That's why this is here -- because this is where we KNOW we have the account ID -- so we grab it.
                            //
                            if (str_other_acct_id.empty() && (pPayment->GetSenderAcctIDForDisplay(theAccountID)))
                            {
                                OTString strTemp(theAccountID);
                                str_other_acct_id = strTemp.Get();
                            }
                        }
                        // ------------------------------------------
                        // By this point, p_str_account is definitely set.
                        OT_ASSERT(NULL != p_str_account); // and it's either blank, or it's one of the accounts we care about.
                        // ---------------------------------------------------
                        OTIdentifier theAssetTypeID;

                        if (pPayment->GetAssetTypeID(theAssetTypeID))
                        {
                            OTString strTemp(theAssetTypeID);
                            const std::string str_inpmt_asset(strTemp.Get()); // The asset type we found on the payment (if we found anything.)
                            // -----------------------------
                            map_of_strings::iterator it_asset = m_assets.find(str_inpmt_asset);
                            // -----------------------------
                            if (it_asset != m_assets.end()) // Found it on the map of asset types we care about.
                            {
                                p_str_asset_type = &(it_asset->first);   // Set the asset type ID.
                                p_str_asset_name = &(it_asset->second);  // The CurrencyTLA. Examples: USD, BTC, etc.
                            }
                            else
                            {
                                // There was definitely an asset type on the instrument, and it definitely
                                // did not match any of the assets that we care about.
                                // Therefore, skip.
                                //
                                OTLog::vError("%s: Skipping: Expired payment record (we don't care about asset type %s)\n",
                                              __FUNCTION__, str_inpmt_asset.c_str());
                                continue;
                            }
                        }
                        // By this point, p_str_asset_type and p_str_asset_name are definitely set.
                        OT_ASSERT(NULL != p_str_asset_type); // and it's either blank, or it's one of the asset types we care about.
                        OT_ASSERT(NULL != p_str_asset_name); // and it's either blank, or it's one of the asset types we care about.
                        // ----------------------------------
                        OTString strMemo;
                        if (pPayment->GetMemo(strMemo))
                        {
                            str_memo = strMemo.Get();
                        }
                        // ----------------------------------
                        // Instrument type (cheque, voucher, etc)
                        int nType = static_cast<int> (pPayment->GetType());

                        str_type = MTRecord_GetTypeString(nType);
                        // ---------------------------------------------------
                        int64_t lAmount = 0;

                        if (pPayment->GetAmount(lAmount))
                        {
                            OTString strTemp;
                            strTemp.Format("%ld", lAmount);
                            str_amount = strTemp.Get();
                        }
                    }
                }
                // ------------------------------
                OTLog::vOutput(0, "%s: ADDED: Expired payment record %s (str_type: %s)\n",
                               __FUNCTION__, bOutgoing ? "(sent)" : "(received)", str_type.c_str());

                shared_ptr_MTRecord sp_Record(new MTRecord(*it_server,
                                                           *p_str_asset_type,
                                                           *p_str_asset_name,
                                                           str_nym_id,     // This is the Nym WHOSE BOX IT IS.
                                                           *p_str_account, // This is the Nym's account for this box. (Blank for incoming, set for outgoing.)
                                                           // Everything above this line, it stores a reference to an external string.
                                                           // -----------------------------
                                                           // Everything below this line, it makes its own internal copy of the string.
                                                           str_name,    // name of sender or recipient (since its in the recordbox.)
                                                           str_date,    // the "date signed" on the receipt.
                                                           str_amount,
                                                           str_type,    // pending, chequeReceipt, etc.
                                                           false,       // Everything in the recordbox is finished. (NOT pending.)
                                                           bOutgoing, // Since it's the recordbox, it contains both incoming and outgoing receipts.
                                                           true, //IsRecord
                                                           false,//IsReceipt,
                                                           MTRecord::Instrument));
                // -------------------------------------------------
                if (strContents.Exists())
                    sp_Record->SetContents(strContents.Get());
                // -------------------------------------------------
                sp_Record->SetDateRange(tValidFrom, tValidTo);
                // -------------------------------------------------
                sp_Record->SetExpired();
                // -------------------------------------------------
                sp_Record->SetBoxIndex(static_cast<int>(nIndex));
                // -------------------------------------------------
                if (!str_memo.empty())
                    sp_Record->SetMemo(str_memo);
                // -------------------------------------------------
                if (!str_other_nym_id.empty())
                    sp_Record->SetOtherNymID(str_other_nym_id);
                // -------------------------------------------------
                if (!str_other_acct_id.empty())
                    sp_Record->SetOtherAccountID(str_other_acct_id);
                // -------------------------------------------------
                sp_Record->SetTransNumForDisplay(pBoxTrans->GetReferenceNumForDisplay());
                sp_Record->SetTransactionNum(pBoxTrans->GetTransactionNum());

                m_contents.push_back(sp_Record);
                // ------------------------------

            } // Loop through ExpiredBox
            else
                OTLog::vOutput(1, "%s: Failed loading expired payments box. (Probably just doesn't exist yet.)\n", __FUNCTION__);
            // ------------------------------------------------

        } // Loop through servers for each Nym.

    } // Loop through Nyms.
    // ------------------------------------------------
    // ASSET ACCOUNT -- INBOX/OUTBOX + RECORD BOX
    // Loop through the Accounts.
    //
    // ------------------------------------------------
    OTLog::vOutput(0, "================ %s: Looping through the accounts in the wallet...\n", __FUNCTION__);
    // ------------------------------------------------
    int nAccountIndex = -1;
    FOR_EACH_IT(list_of_strings, m_accounts, it_acct)
    {
        ++nAccountIndex; // (0 on first iteration.)
        // ------------------
        // For each account, loop through its inbox, outbox, and record box.
        //
        const std::string  & str_account_id(*it_acct);
        const OTIdentifier   theAccountID  (str_account_id);
        OTAccount * pAccount = pWallet->GetAccount(theAccountID);
        OT_ASSERT(NULL != pAccount);
        // ------------------------------------------------
        const OTIdentifier & theNymID    = pAccount->GetUserID();
        const OTIdentifier & theServerID = pAccount->GetPurportedServerID();
        const OTIdentifier & theAssetID  = pAccount->GetAssetTypeID();
        // ------------------------------------------------
        const OTString       strNymID   (theNymID);
        const OTString       strServerID(theServerID);
        const OTString       strAssetID (theAssetID);
        // ------------------------------------------------
        OTLog::vOutput(0, "------------\n%s: Account: %d, ID: %s\n", __FUNCTION__,
                       nAccountIndex, str_account_id.c_str());
        // ------------------------------------------------
        const std::string    str_nym_id   (strNymID.Get());
        const std::string    str_server_id(strServerID.Get());
        const std::string    str_asset_id (strAssetID.Get());
        // ------------------------------------------------
        const std::string  * pstr_nym_id     = &MTRecordList::s_blank;
        const std::string  * pstr_server_id  = &MTRecordList::s_blank;
        const std::string  * pstr_asset_id   = &MTRecordList::s_blank;
        const std::string  * pstr_asset_name = &MTRecordList::s_blank;
        // ------------------------------------------------
        // NOTE: Since this account is already on my "care about" list for accounts,
        // I wouldn't bother double-checking my "care about" lists for servers, nyms,
        // and asset types. But I still look up the appropriate string for each, since
        // I have to pass a reference to it into the constructor for MTRecord. (To a version
        // that won't be deleted, since the MTRecord will reference it. And the "care about"
        // list definitely contains a copy of the string that won't be deleted.)
        //
        list_of_strings::iterator it_nym    = std::find(m_nyms.begin(),    m_nyms.end(),    str_nym_id);
        list_of_strings::iterator it_server = std::find(m_servers.begin(), m_servers.end(), str_server_id);
         map_of_strings::iterator it_asset  = m_assets.find(str_asset_id);
        // ------------------------------------------------
        if ((m_nyms.end() == it_nym) || (m_servers.end() == it_server) || (m_assets.end() == it_asset))
        {
            OTLog::vOutput(0, "%s: Skipping an account (%s) since its Nym, or Server, "
                           "or Asset Type wasn't on my list.\n", __FUNCTION__,
                           str_account_id.c_str());
            continue;
        }
        // ------------------------------------------------
        // These pointers are what we'll use to construct each MTRecord.
        //
        pstr_nym_id     = &(*it_nym);
        pstr_server_id  = &(*it_server);
        pstr_asset_id   = &(it_asset->first);
        pstr_asset_name = &(it_asset->second);
        // ------------------------------------------------
        // Loop through asset account INBOX.
        //
        // OPTIMIZE FYI:
        // NOTE: LoadInbox is much SLOWER than LoadInboxNoVerify, but it also lets you get
        // the NAME off of the box receipt. So if you are willing to GIVE UP the NAME, in
        // return for FASTER PERFORMANCE, then call SetFastMode() before Populating.
        //
        OTLedger * pInbox = m_bRunFast ? OTAPI_Wrap::OTAPI()->LoadInboxNoVerify(theServerID, theNymID, theAccountID) :
                                         OTAPI_Wrap::OTAPI()->LoadInbox        (theServerID, theNymID, theAccountID);
        OTCleanup<OTLedger> theInboxAngel(pInbox);

        // ------------------------------------------------
        int nInboxIndex = -1;
        // It loaded up, so let's loop through it.
        if (NULL != pInbox) FOR_EACH(mapOfTransactions, pInbox->GetTransactionMap())
        {
            ++nInboxIndex; // (0 on first iteration.)
            // ------------------------------------------------
            if (0 == nInboxIndex)
                OTLog::vOutput(0, "%s: Beginning loop through asset account INBOX...\n", __FUNCTION__);
            // --------------------------------
            OTTransaction * pBoxTrans = (*it).second;
            OT_ASSERT(NULL != pBoxTrans);
            // ------------------------------------------------
            OTLog::vOutput(0, "%s: Inbox index: %d\n", __FUNCTION__, nInboxIndex);
            // ------------------------------------------------
            bool bCanceled = false;
            // ------------------------------------------------
            std::string  str_name; // name of sender (since its in the inbox.)
            std::string  str_other_nym_id;
            std::string  str_other_acct_id;
            std::string  str_memo;

            if (false == pBoxTrans->IsAbbreviated())
            {
                OTString strMemo;

                if (pBoxTrans->GetMemo(strMemo))
                    str_memo = strMemo.Get();
                // ------------------------------------------------
                if (OTTransaction::pending == pBoxTrans->GetType())
                {
                    // NOTE: REMOVE THE BELOW CODE. (Found a better way, above this block.)
//                    const OTString strBoxTrans(*pBoxTrans);
//
//                    if (strBoxTrans.Exists())
//                        str_memo = OTAPI_Wrap::It()->Pending_GetNote(*pstr_server_id, *pstr_nym_id, str_account_id, strBoxTrans.Get());
                    // ------------------------------
                    OTIdentifier theSenderID, theSenderAcctID;

                    if (pBoxTrans->GetSenderAcctIDForDisplay(theSenderAcctID)) // ACCOUNT name.
                    {
                        if (pBoxTrans->GetSenderUserIDForDisplay(theSenderID))
                        {
                            const OTString strSenderID(theSenderID);
                            str_other_nym_id = strSenderID.Get();
                        }
                        // ------------------------------------
                        const OTString    strSenderAcctID(theSenderAcctID);
                        const std::string str_sender_acct_id(strSenderAcctID.Get());

                        str_other_acct_id = str_sender_acct_id;

                        OTString strName(m_pLookup->GetAcctName(str_other_acct_id,
                                                                str_other_nym_id.empty() ? NULL :
                                                                    &str_other_nym_id, // nym ID if known
                                                                pstr_server_id, // server ID if known.
                                                                pstr_asset_id)), // asset ID if known.
                                 strNameTemp;

                        if (strName.Exists())
                        {
                            strNameTemp.Format(MC_UI_TEXT_FROM, strName.Get());
                            str_name = strNameTemp.Get();
                        }
                        else if (!str_other_nym_id.empty())
                        {
                            OTString strNymName(m_pLookup->GetNymName(str_other_nym_id, &(*it_server))), strNymNameTemp;

                            if (strNymName.Exists())
                            {
                                strNymNameTemp.Format(MC_UI_TEXT_FROM, strNymName.Get());
                                str_name = strNameTemp.Get();
                            }
                        }
                        // ---------------------------------------
                        if (str_name.empty())
                        {
                            strNameTemp.Format(MC_UI_TEXT_FROM, str_sender_acct_id.c_str());
                            str_name = strNameTemp.Get();
                        }
                    }
                    else if (pBoxTrans->GetSenderUserIDForDisplay(theSenderID)) // NYM name.
                    {
                        const OTString    strSenderID  (theSenderID);
                        const std::string str_sender_id(strSenderID.Get());

                        OTString strName(m_pLookup->GetNymName(str_sender_id, &(*it_server))), strNameTemp;

                        if (strName.Exists())
                            strNameTemp.Format(MC_UI_TEXT_FROM, strName.Get());
                        else
                            strNameTemp.Format(MC_UI_TEXT_FROM, str_sender_id.c_str());

                        str_name         = strNameTemp.Get();
                        str_other_nym_id = str_sender_id;
                    }
                    else
                    {
                        OTString strName(OTAPI_Wrap::It()->GetAccountWallet_Name(str_account_id)), strNameTemp;

                        if (strName.Exists())
                            strNameTemp = strName;
                        else
                            strNameTemp = str_account_id;

                        str_name = strNameTemp.Get(); // Todo: lookup the name in address book also.
                    }
                } // end: (if pending)

                else // else it's a receipt.
                {
                    OTIdentifier theRecipientID, theRecipientAcctID;

                    if (pBoxTrans->GetRecipientUserIDForDisplay(theRecipientID))
                    {
                        const OTString    strRecipientID       (theRecipientID);
                        const std::string str_recipient_user_id(strRecipientID.Get());

                        OTString strName(m_pLookup->GetNymName(str_recipient_user_id, &(*it_server))), strNameTemp;

                        if (strName.Exists())
                            strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                        else
                            strNameTemp.Format(MC_UI_TEXT_TO, str_recipient_user_id.c_str());

                        str_name         = strNameTemp.Get(); // Todo: lookup the name in address book also.
                        str_other_nym_id = str_recipient_user_id;
                        // ------------------------------------
                        if (pBoxTrans->GetRecipientAcctIDForDisplay(theRecipientAcctID))
                        {
                            const OTString strRecipientAcctID(theRecipientAcctID);
                            str_other_acct_id = strRecipientAcctID.Get();
                        }
                    }
                    else if (pBoxTrans->GetRecipientAcctIDForDisplay(theRecipientAcctID))
                    {
                        const OTString    strRecipientAcctID   (theRecipientAcctID);
                        const std::string str_recipient_acct_id(strRecipientAcctID.Get());

                        OTString strName(m_pLookup->GetAcctName(str_recipient_acct_id,
                                                                NULL, // nym ID if known
                                                                pstr_server_id, // server ID if known.
                                                                pstr_asset_id)), // asset ID if known.
                                strNameTemp;

                        if (strName.Exists())
                            strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                        else
                            strNameTemp.Format(MC_UI_TEXT_TO, str_recipient_acct_id.c_str());

                        str_name          = strNameTemp.Get(); // Todo: lookup the name in address book also.
                        str_other_acct_id = str_recipient_acct_id;
                    }
                } //end: (else it's a receipt.)
            }
            // ------------------------------
            bCanceled = pBoxTrans->IsCancelled();
            // ------------------------------
            time_t tValidFrom = 0, tValidTo = 0;
            // ------------------------------
            std::string str_date    = "0"; // the "date signed" on the transaction receipt.
            time_t      tDateSigned = pBoxTrans->GetDateSigned();

            if (tDateSigned > 0)
            {
                tValidFrom = tDateSigned;
                const long lDateSigned = static_cast<long>(tDateSigned);
                OTString strDateSigned;
                strDateSigned.Format("%ld", lDateSigned);
                str_date = strDateSigned.Get();
            }
            // ------------------------------
            std::string str_amount;  // <========== AMOUNT
            long        lAmount = pBoxTrans->GetAbbrevDisplayAmount();

            if (0 == lAmount)
                lAmount = pBoxTrans->GetReceiptAmount();
            // ------------------------------------------
            if (0 != lAmount)
            {
                OTString strTemp;
                strTemp.Format("%ld", lAmount);
                str_amount = strTemp.Get();
            }
            // ------------------------------
            const std::string str_type(pBoxTrans->GetTypeString()); // pending, chequeReceipt, etc.
            // ------------------------------
            OTLog::vOutput(0, "%s: ADDED: incoming %s (str_type: %s)\n",
                           __FUNCTION__,
                           (OTTransaction::pending == pBoxTrans->GetType()) ? "pending transfer" : "receipt",
                           str_type.c_str());

            shared_ptr_MTRecord sp_Record(new MTRecord(*pstr_server_id,
                                                       *pstr_asset_id,
                                                       *pstr_asset_name,
                                                       *pstr_nym_id,   // This is the Nym WHOSE BOX IT IS.
                                                       str_account_id, // This is the Nym's account for this box.
                                                       // Everything above this line, it stores a reference to an external string.
                                                       // -----------------------------
                                                       // Everything below this line, it makes its own internal copy of the string.
                                                       str_name, // name of sender (since its in the inbox.)
                                                       str_date, // the "valid from" date on the instrument.
                                                       str_amount,
                                                       str_type, // pending, chequeReceipt, etc.
                                                       (OTTransaction::pending == pBoxTrans->GetType()), // Sometimes true, often false.
                                                       (lAmount < 0) ? true : false, //bIsOutgoing (this is the inbox, but a transferReceipt in the inbox represents outgoing funds. Whereas a "pending" in the inbox represents incoming funds. For now I'm just going to go based on whether the amount is negative or not, to determine incoming / outgoing. We'll see how that works.)
                                                       false, //IsRecord
                                                       (OTTransaction::pending != pBoxTrans->GetType()), //IsReceipt,
                                                       (OTTransaction::pending == pBoxTrans->GetType()) ?
                                                            MTRecord::Transfer : MTRecord::Receipt ));
            // -------------------------------------------------
            const OTString strContents(*pBoxTrans);
            sp_Record->SetContents(strContents.Get());
            // -------------------------------------------------
            sp_Record->SetDateRange(tValidFrom, tValidTo);
            // -------------------------------------------------
            sp_Record->SetBoxIndex(static_cast<int>(nInboxIndex));
            // -------------------------------------------------
            if (bCanceled)
                sp_Record->SetCanceled();
            // -------------------------------------------------
            if (!str_memo.empty())
                sp_Record->SetMemo(str_memo);
            // -------------------------------------------------
            if (!str_other_nym_id.empty())
                sp_Record->SetOtherNymID(str_other_nym_id);
            // -------------------------------------------------
            if (!str_other_acct_id.empty())
                sp_Record->SetOtherAccountID(str_other_acct_id);
            // -------------------------------------------------
            sp_Record->SetTransNumForDisplay(pBoxTrans->GetReferenceNumForDisplay());
            sp_Record->SetTransactionNum(pBoxTrans->GetTransactionNum());
            // -------------------------------------------------
            m_contents.push_back(sp_Record);
        }
        // ------------------------------------------------
        // OPTIMIZE FYI:
        // NOTE: LoadOutbox is much SLOWER than LoadOutboxNoVerify, but it also lets you get
        // the NAME off of the box receipt. So if you are willing to GIVE UP the NAME, in
        // return for FASTER PERFORMANCE, then call SetFastMode() before running Populate.
        //
        OTLedger * pOutbox = m_bRunFast ? OTAPI_Wrap::OTAPI()->LoadOutboxNoVerify(theServerID, theNymID, theAccountID) :
                                          OTAPI_Wrap::OTAPI()->LoadOutbox        (theServerID, theNymID, theAccountID);
        OTCleanup<OTLedger> theOutboxAngel(pOutbox);

        // It loaded up, so let's loop through it.
        int nOutboxIndex = -1;
        if (NULL != pOutbox) FOR_EACH(mapOfTransactions, pOutbox->GetTransactionMap())
        {
            ++nOutboxIndex; // (0 on first iteration.)
            // ------------------------------------------------
            if (0 == nOutboxIndex)
                OTLog::vOutput(0, "%s: Beginning loop through asset account OUTBOX...\n", __FUNCTION__);
            // --------------------------------
            OTTransaction * pBoxTrans = (*it).second;
            OT_ASSERT(NULL != pBoxTrans);
            // ------------------------------------------------
            OTLog::vOutput(0, "%s: Outbox index: %d\n", __FUNCTION__, nOutboxIndex);
            // ------------------------------------------------
            std::string  str_name; // name of recipient (since its in the outbox.)
            std::string  str_other_nym_id;
            std::string  str_other_acct_id;
            std::string  str_memo;

            if (false == pBoxTrans->IsAbbreviated())
            {
                OTIdentifier theRecipientID, theRecipientAcctID;

                if (pBoxTrans->GetRecipientUserIDForDisplay(theRecipientID))
                {
                    const OTString    strRecipientID  (theRecipientID);
                    const std::string str_recipient_id(strRecipientID.Get());

                    OTString strName(m_pLookup->GetNymName(str_recipient_id, &(*it_server))), strNameTemp;

                    if (strName.Exists())
                        strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                    else
                        strNameTemp.Format(MC_UI_TEXT_TO, str_recipient_id.c_str());

                    str_name         = strNameTemp.Get(); // Todo: lookup the name in address book also.
                    str_other_nym_id = str_recipient_id;
                    // ------------------------------------
                    if (pBoxTrans->GetRecipientAcctIDForDisplay(theRecipientAcctID))
                    {
                        const OTString strRecipientAcctID(theRecipientAcctID);
                        str_other_acct_id = strRecipientAcctID.Get();
                    }
                }
                else if (pBoxTrans->GetRecipientAcctIDForDisplay(theRecipientAcctID))
                {
                    const OTString    strRecipientAcctID(theRecipientAcctID);
                    const std::string str_recipient_acct_id(strRecipientAcctID.Get());

                    OTString strName(m_pLookup->GetAcctName(str_recipient_acct_id,
                                                            NULL, // nym ID if known
                                                            pstr_server_id, // server ID if known.
                                                            pstr_asset_id)), // asset ID if known.
                            strNameTemp;

                    if (strName.Exists())
                        strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                    else
                        strNameTemp.Format(MC_UI_TEXT_TO, str_recipient_acct_id.c_str());

                    str_name          = strNameTemp.Get(); // Todo: lookup the name in address book also.
                    str_other_acct_id = str_recipient_acct_id;
                }
                // --------------------------
                if (OTTransaction::pending == pBoxTrans->GetType())
                {
                    OTString strMemo;

                    if (pBoxTrans->GetMemo(strMemo))
                        str_memo = strMemo.Get();

                    // DELETE THE BELOW CODE (replaced by above code.)
//                    const OTString strBoxTrans(*pBoxTrans);
//
//                    if (strBoxTrans.Exists())
//                        str_memo = OTAPI_Wrap::It()->Pending_GetNote(*pstr_server_id, *pstr_nym_id, str_account_id, strBoxTrans.Get());
                }
            }
            // ------------------------------
            time_t tValidFrom = 0, tValidTo = 0;
            // ------------------------------
            std::string str_date    = "0"; // the "date signed" on the transaction receipt.
            time_t      tDateSigned = pBoxTrans->GetDateSigned();

            if (tDateSigned > 0)
            {
                tValidFrom = tDateSigned;
                const long lDateSigned = static_cast<long>(tDateSigned);
                OTString strDateSigned;
                strDateSigned.Format("%ld", lDateSigned);
                str_date = strDateSigned.Get();
            }
            // ------------------------------
            std::string str_amount;  // <========== AMOUNT
            long        lAmount = pBoxTrans->GetAbbrevDisplayAmount();

            if (0 == lAmount)
                lAmount = pBoxTrans->GetReceiptAmount();
            // -------------------------------------------
            if (lAmount > 0) // Outgoing transfer should display with negative amount
                lAmount *= (-1);
            // -------------------------------------------
            if (0 != lAmount)
            {
                OTString strTemp;
                strTemp.Format("%ld", lAmount);
                str_amount = strTemp.Get();
            }
            // ------------------------------
            std::string str_type(pBoxTrans->GetTypeString()); // pending, chequeReceipt, etc.
            if (0 == str_type.compare("pending"))
                str_type = "transfer";
            // ------------------------------
            OTLog::vOutput(0, "%s: ADDED: %s outgoing transfer (str_type: %s).\n", __FUNCTION__,
                           (OTTransaction::pending == pBoxTrans->GetType()) ? "pending" : "ERROR", str_type.c_str());

            shared_ptr_MTRecord sp_Record(new MTRecord(*pstr_server_id,
                                                       *pstr_asset_id,
                                                       *pstr_asset_name,
                                                       *pstr_nym_id,   // This is the Nym WHOSE BOX IT IS.
                                                       str_account_id, // This is the Nym's account for this box.
                                                       // Everything above this line, it stores a reference to an external string.
                                                       // -----------------------------
                                                       // Everything below this line, it makes its own internal copy of the string.
                                                       str_name, // name of recipient (since its in the outbox.)
                                                       str_date, // the "valid from" date on the instrument.
                                                       str_amount,
                                                       str_type, // pending, chequeReceipt, etc.
                                                       (OTTransaction::pending == pBoxTrans->GetType()), // Basically always true, in this case.
                                                       true,  //bIsOutgoing=true. (Since this is the outbox...)
                                                       false, //IsRecord
                                                       false, //IsReceipt
                                                       MTRecord::Transfer
                                                       ));
            // -------------------------------------------------
            const OTString strContents(*pBoxTrans);
            sp_Record->SetContents(strContents.Get());
            // -------------------------------------------------
            sp_Record->SetDateRange(tValidFrom, tValidTo);
            // -------------------------------------------------
            sp_Record->SetBoxIndex(static_cast<int>(nOutboxIndex));
            // -------------------------------------------------
            if (!str_memo.empty())
                sp_Record->SetMemo(str_memo);
            // -------------------------------------------------
            if (!str_other_nym_id.empty())
                sp_Record->SetOtherNymID(str_other_nym_id);
            // -------------------------------------------------
            if (!str_other_acct_id.empty())
                sp_Record->SetOtherAccountID(str_other_acct_id);
            // -------------------------------------------------
            sp_Record->SetTransNumForDisplay(pBoxTrans->GetReferenceNumForDisplay());
            sp_Record->SetTransactionNum(pBoxTrans->GetTransactionNum());
            // -------------------------------------------------
            m_contents.push_back(sp_Record);
        }
        // ------------------------------------------------
        // For this record box, pass a NymID AND an AcctID,
        // since it's the recordbox for a specific account.
        //
        // OPTIMIZE FYI:
        // NOTE: LoadRecordBox is much SLOWER than LoadRecordBoxNoVerify, but it also lets you get
        // the NAME off of the box receipt. So if you are willing to GIVE UP the NAME, in
        // return for FASTER PERFORMANCE, then call SetFastMode() before Populating.
        //
        OTLedger * pRecordbox = m_bRunFast ? OTAPI_Wrap::OTAPI()->LoadRecordBoxNoVerify(theServerID, theNymID, theAccountID) :
                                             OTAPI_Wrap::OTAPI()->LoadRecordBox        (theServerID, theNymID, theAccountID);
        OTCleanup<OTLedger> theRecordBoxAngel(pRecordbox);

        // It loaded up, so let's loop through it.
        int nRecordIndex = -1;
        if (NULL != pRecordbox) FOR_EACH(mapOfTransactions, pRecordbox->GetTransactionMap())
        {
            ++nRecordIndex;
            // -----------------------------
            OTTransaction * pBoxTrans = (*it).second;
            OT_ASSERT(NULL != pBoxTrans);
            // ------------------------------------------------
            OTLog::vOutput(0, "%s: Account RECORD index: %d\n", __FUNCTION__, nRecordIndex);
            // ------------------------------------------------
            bool bOutgoing = false;
            bool bCanceled = false;
            // ------------------------------
            std::string  str_name; // name of sender OR recipient (depending on whether it was originally incoming or outgoing.)
            std::string  str_other_nym_id;
            std::string  str_other_acct_id;
            std::string  str_memo;

            if (false == pBoxTrans->IsAbbreviated())
            {
                OTIdentifier theSenderID,    theSenderAcctID;
                OTIdentifier theRecipientID, theRecipientAcctID;

                if (pBoxTrans->GetSenderAcctIDForDisplay(theSenderAcctID))
                {
                    const OTString    strSenderAcctID(theSenderAcctID);
                    const std::string str_sender_acct_id(strSenderAcctID.Get());

                    // Usually, Nym is the RECIPIENT. Sometimes he's the sender.
                    // Either way, we want the OTHER ID (the other Nym) for display.
                    // So here, if Nym's CLEARLY the sender, then we want the RECIPIENT.
                    // Whereas if Nym were the recipient, then we'd want the SENDER. (For display.)
                    //
                    if (0 == str_account_id.compare(str_sender_acct_id)) // str_account_id IS str_sender_acct_id. (Therefore we want recipient.)
                    {
                        bOutgoing = true; // if Nym is the sender, then it must have been outgoing.

                        const bool bGotRecipientUserIDForDisplay =
                                pBoxTrans->GetRecipientUserIDForDisplay(theRecipientID);

                        if (pBoxTrans->GetRecipientAcctIDForDisplay(theRecipientAcctID))
                        {
                            const OTString    strRecipientAcctID(theRecipientAcctID);
                            const std::string str_recip_acct_id(strRecipientAcctID.Get());

                                  OTString    strRecipientUserID("");
                                  std::string str_recip_user_id("");

                            if (bGotRecipientUserIDForDisplay)
                            {
                                theRecipientID.GetString(strRecipientUserID);
                                str_recip_user_id = strRecipientUserID.Get();
                            }
                            // ---------------------------------------
                            // NOTE: We check for cancelled here so we don't accidentally
                            // cause the address book to falsely believe that str_recip_user_id
                            // is the owner of str_recip_acct_id. (If the cheque/invoice is cancelled,
                            // the recipient account will be the sender account, which is NOT owned
                            // by the recipient, obviously...)
                            //
                            if (!pBoxTrans->IsCancelled())
                            {
                                OTString strName(m_pLookup->GetAcctName(str_recip_acct_id,
                                                                        // NOTE: we CANNOT pass str_recip_user_id here with str_recip_acct_id
                                                                        // if it's a cancelled instrument, since in that case, the SENDER ACCT
                                                                        // is ALSO the RECIPIENT ACCT. So this logic is ONLY correct since we
                                                                        // are inside the block of if (!pBoxTrans->IsCancelled())
                                                                        // (Otherwise we'd be training the address book to falsely believe that
                                                                        // the recipient Nym is the owner of the sender acct.)
                                                                        bGotRecipientUserIDForDisplay ?
                                                                           &str_recip_user_id : NULL, // nym ID if known
                                                                        pstr_server_id, // server ID if known.
                                                                        pstr_asset_id)), // asset ID if known.
                                        strNameTemp;

                                if (strName.Exists())
                                    strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                                else
                                    strNameTemp.Format(MC_UI_TEXT_TO, str_recip_acct_id.c_str());

                                str_name = strNameTemp.Get(); // We don't want to see our own name on cancelled cheques.
                            }
                            str_other_acct_id = str_recip_acct_id;
                        }
                        // -----------------------------------------
                        if (bGotRecipientUserIDForDisplay)
                        {
                            const OTString strRecipientID(theRecipientID);
                            const std::string str_recipient_id(strRecipientID.Get());

                            str_other_nym_id = str_recipient_id;

                            if (str_name.empty())
                            {
                                OTString strName(m_pLookup->GetNymName(str_recipient_id, &(*it_server))), strNameTemp;

                                if (strName.Exists())
                                    strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                                else
                                    strNameTemp.Format(MC_UI_TEXT_TO, str_recipient_id.c_str());

                                str_name = strNameTemp.Get();
                            }
                        }
                    }
                    else // str_account_id IS NOT str_sender_acct_id. (Therefore we want sender.)
                    {    // In this case, some OTHER Nym is the sender, so it must have been incoming. (And bOutgoing is already false.)

                        if (pBoxTrans->GetSenderUserIDForDisplay(theSenderID))
                        {
                            const OTString strSenderUserID(theSenderID);
                            str_other_nym_id = strSenderUserID.Get();
                        }
                        // ------------------------------------
                        OTString strName(m_pLookup->GetAcctName(str_sender_acct_id,
                                                                str_other_nym_id.empty() ? NULL :
                                                                    &str_other_nym_id, // nym ID if known
                                                                pstr_server_id, // server ID if known.
                                                                pstr_asset_id)), // asset ID if known.
                                strNameTemp;

                        if (strName.Exists())
                            strNameTemp.Format(MC_UI_TEXT_FROM, strName.Get());
                        else
                            strNameTemp.Format(MC_UI_TEXT_FROM, str_sender_acct_id.c_str());

                        str_name          = strNameTemp.Get();
                        str_other_acct_id = str_sender_acct_id;
                    }
                }
                // In this block below, we already KNOW GetSenderAcctIDForDisplay is EMPTY.
                // (So it's "recipient or bust.")
                else if (pBoxTrans->GetRecipientAcctIDForDisplay(theRecipientAcctID))
                {
                    if (pBoxTrans->GetRecipientUserIDForDisplay(theRecipientID))
                    {
                        const OTString strRecipientID(theRecipientID);
                        const std::string str_recipient_user_id(strRecipientID.Get());

                        str_other_nym_id = str_recipient_user_id;
                    }
                    // --------------------------------------------------------
                    const OTString strRecipientAcctID(theRecipientAcctID);
                    const std::string str_recipient_acct_id(strRecipientAcctID.Get());

                    if (0 != str_account_id.compare(str_recipient_acct_id)) // str_account_id is NOT str_recipient_acct_id. (Therefore we want str_recipient_acct_id.)
                    {
                        // If Nym is not the recipient, then he must be the sender.
                        // (Therefore it must be outgoing.)
                        bOutgoing = true;

                        OTString strName(m_pLookup->GetAcctName(str_recipient_acct_id,
                                                                str_other_nym_id.empty() ? NULL :
                                                                    &str_other_nym_id, // nym ID if known
                                                                pstr_server_id, // server ID if known.
                                                                pstr_asset_id)), // asset ID if known.
                                strNameTemp;

                        if (strName.Exists())
                            strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                        else
                            strNameTemp.Format(MC_UI_TEXT_TO, str_recipient_acct_id.c_str());

                        str_name          = strNameTemp.Get(); // Todo: lookup the name in address book also.
                        str_other_acct_id = str_recipient_acct_id;
                    }
                }
                else if (pBoxTrans->GetSenderUserIDForDisplay(theSenderID))
                {
                    const OTString    strSenderID  (theSenderID);
                    const std::string str_sender_id(strSenderID.Get());

                    // Usually, Nym is the RECIPIENT. Sometimes he's the sender.
                    // Either way, we want the OTHER ID (the other Nym) for display.
                    // So here, if Nym's CLEARLY the sender, then we want the RECIPIENT.
                    // Whereas if Nym were the recipient, then we'd want the SENDER. (For display.)
                    //
                    if (0 == str_nym_id.compare(str_sender_id)) // str_nym_id IS str_sender_id. (Therefore we want recipient.)
                    {
                        bOutgoing = true; // if Nym is the sender, then it must have been outgoing.

                        if (pBoxTrans->GetRecipientUserIDForDisplay(theRecipientID))
                        {
                            const OTString strRecipientID(theRecipientID);
                            const std::string str_recipient_id(strRecipientID.Get());

                            OTString strName(m_pLookup->GetNymName(str_recipient_id, &(*it_server))), strNameTemp;

                            if (strName.Exists())
                                strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                            else
                                strNameTemp.Format(MC_UI_TEXT_TO, str_recipient_id.c_str());

                            str_name         = strNameTemp.Get();
                            str_other_nym_id = str_recipient_id;
                            // ------------------------------------
                            if (pBoxTrans->GetRecipientAcctIDForDisplay(theRecipientAcctID))
                            {
                                const OTString strRecipientAcctID(theRecipientAcctID);
                                str_other_acct_id = strRecipientAcctID.Get();
                            }
                        }
                    }
                    else // str_nym_id IS NOT str_sender_id. (Therefore we want sender.)
                    {    // In this case, some OTHER Nym is the sender, so it must have been incoming. (And bOutgoing is already false.)

                        OTString strName(m_pLookup->GetNymName(str_sender_id, &(*it_server))), strNameTemp;

                        if (strName.Exists())
                            strNameTemp.Format(MC_UI_TEXT_FROM, strName.Get());
                        else
                            strNameTemp.Format(MC_UI_TEXT_FROM, str_sender_id.c_str());

                        str_name         = strNameTemp.Get(); // Todo: lookup the name in address book also.
                        str_other_nym_id = str_sender_id;
                        // ------------------------------------
                        if (pBoxTrans->GetSenderAcctIDForDisplay(theSenderAcctID))
                        {
                            const OTString strSenderAcctID(theSenderAcctID);
                            str_other_acct_id = strSenderAcctID.Get();
                        }
                    }
                }
                // In this block below, we already KNOW GetSenderUserIDForDisplay is EMPTY.
                // (So it's "recipient or bust.")
                else if (pBoxTrans->GetRecipientUserIDForDisplay(theRecipientID))
                {
                    const OTString strRecipientID(theRecipientID);
                    const std::string str_recipient_id(strRecipientID.Get());

                    if (0 != str_nym_id.compare(str_recipient_id)) // str_nym_id is NOT str_recipient_id. (Therefore we want str_recipient_id.)
                    {
                        // If Nym is not the recipient, then he must be the sender.
                        // (Therefore it must be outgoing.)
                        bOutgoing = true;

                        OTString strName(m_pLookup->GetNymName(str_recipient_id, &(*it_server))), strNameTemp;

                        if (strName.Exists())
                            strNameTemp.Format(MC_UI_TEXT_TO, strName.Get());
                        else
                            strNameTemp.Format(MC_UI_TEXT_TO, str_recipient_id.c_str());

                        str_name         = strNameTemp.Get(); // Todo: lookup the name in address book also.
                        str_other_nym_id = str_recipient_id;
                        // ------------------------------------
                        if (pBoxTrans->GetRecipientAcctIDForDisplay(theRecipientAcctID))
                        {
                            const OTString strRecipientAcctID(theRecipientAcctID);
                            str_other_acct_id = strRecipientAcctID.Get();
                        }
                    }
                }
                // ---------------------
                // Get the Memo field for a transferReceipt and also for other receipts.
                //
                OTString strMemo;

                if (pBoxTrans->GetMemo(strMemo))
                    str_memo = strMemo.Get();

            } // if not abbreviated.
            // ------------------------------
            bCanceled = pBoxTrans->IsCancelled();
            // ------------------------------
            time_t tValidFrom = 0, tValidTo = 0;
            // ------------------------------
            std::string str_date    = "0"; // the "date signed" on the transaction receipt.
            time_t      tDateSigned = pBoxTrans->GetDateSigned();

            if (tDateSigned > 0)
            {
                tValidFrom = tDateSigned;
                const long lDateSigned = static_cast<long>(tDateSigned);
                OTString strDateSigned;
                strDateSigned.Format("%ld", lDateSigned);
                str_date = strDateSigned.Get();
            }
            // ------------------------------
            std::string str_amount;  // <========== AMOUNT
            long        lAmount = pBoxTrans->GetAbbrevDisplayAmount();

            if (0 == lAmount)
                lAmount = pBoxTrans->GetReceiptAmount();
            // ------------------------------------------
            const std::string str_type(pBoxTrans->GetTypeString()); // pending, chequeReceipt, etc.
            // ------------------------------
            if (0 == str_type.compare("transferReceipt"))
                bOutgoing = true; // only the sender of a transfer will have a transferReceipt.
            else if (0 == str_type.compare("pending"))
                bOutgoing = false; // only the recipient of a transfer will have a pending in his recordbox.
            // ------------------------------
            if (0 != lAmount)
            {

//                if (lAmount < 0)
//                    bOutgoing = true;
//                else
//                    bOutgoing = false;

                // A transfer receipt ALWAYS represents an outgoing transfer.
                // If the amount is over 0, we want to display it as a negative
                // since it represents money LEAVING my account.
//                if ((0 == str_type.compare("transferReceipt")) && (lAmount > 0))
//                    lAmount *= (-1);

                OTString strTemp;
                strTemp.Format("%ld", lAmount);
                str_amount = strTemp.Get();

            }
            // ------------------------------
            OTLog::vOutput(0, "%s: ADDED: %s (asset account) record (str_type: %s)\n",
                           __FUNCTION__,
                           // This line means: If it's a receipt, use a blank string. Otherwise if
                           // it's a transfer, then show sent/received. (This is the record box, so
                           // if it's a transfer, it's a completed one.)
                           //
                           // FYI, for Receipts we don't say "sent transferReceipt",
                           // we just say "transferReceipt."
                           //
                           (pBoxTrans->GetType() != OTTransaction::pending) ? "" : (bOutgoing ? "sent" : "received"),
                           str_type.c_str());

            shared_ptr_MTRecord sp_Record(new MTRecord(*pstr_server_id,
                                                       *pstr_asset_id,
                                                       *pstr_asset_name,
                                                       *pstr_nym_id,   // This is the Nym WHOSE BOX IT IS.
                                                       str_account_id, // This is the Nym's account for this box.
                                                       // Everything above this line, it stores a reference to an external string.
                                                       // -----------------------------
                                                       // Everything below this line, it makes its own internal copy of the string.
                                                       str_name, // name of sender or recipient (whichever is NOT the current Nym.)
                                                       str_date, // the "valid from" date on the instrument.
                                                       str_amount,
                                                       str_type, // pending, chequeReceipt, etc.
                                                       false,    // bPending=false. If it's in the record box, then it's finished (not pending.)
                                                       bOutgoing, // Record box stores both old incoming, AND old outgoing, receipts.
                                                       true, //IsRecord
                                                       pBoxTrans->GetType() != OTTransaction::pending, //IsReceipt
                                                       pBoxTrans->GetType() == OTTransaction::pending ?
                                                            MTRecord::Transfer : MTRecord::Receipt));
            // -------------------------------------------------
            const OTString strContents(*pBoxTrans);
            sp_Record->SetContents(strContents.Get());
            // -------------------------------------------------
            if (bCanceled)
                sp_Record->SetCanceled();
            // -------------------------------------------------
            sp_Record->SetDateRange(tValidFrom, tValidTo);
            // -------------------------------------------------
            sp_Record->SetBoxIndex(static_cast<int>(nRecordIndex));
            // -------------------------------------------------
            if (!str_memo.empty())
                sp_Record->SetMemo(str_memo);
            // -------------------------------------------------
            if (!str_other_nym_id.empty())
                sp_Record->SetOtherNymID(str_other_nym_id);
            // -------------------------------------------------
            if (!str_other_acct_id.empty())
                sp_Record->SetOtherAccountID(str_other_acct_id);
            // -------------------------------------------------
            sp_Record->SetTransNumForDisplay(pBoxTrans->GetReferenceNumForDisplay());
            sp_Record->SetTransactionNum(pBoxTrans->GetTransactionNum());
            // -------------------------------------------------
            m_contents.push_back(sp_Record);
        }
        // ------------------------------------------------

    } // loop through the accounts.
    // ------------------------------------------------
    // SORT the vector.
    //
    // TODO OPTIMIZE: We might load everything up into a multimap, and THEN copy it
    // directly over to the vector. (Since the multimap sorts automatically on insert.)
    // The question is, would doing that be any faster than just sorting it here?
    // (Possibly not, but I'm not sure. Re-visit later.)
    //
    std::sort (m_contents.begin(), m_contents.end(), compare_records); // Todo optimize: any faster sorting algorithms?
    // ------------------------------------------------
    return true;
}


// ------------------------------------------------

MTRecordList::MTRecordList(MTNameLookup & theLookup) :
    m_pLookup(&theLookup),
    m_bRunFast(false),
    m_bAutoAcceptCheques  (false),
    m_bAutoAcceptReceipts (false),
    m_bAutoAcceptTransfers(false),
    m_bAutoAcceptCash     (false)
{

}


MTRecordList::~MTRecordList()
{
    if (NULL != m_pLookup)
        delete m_pLookup;

    m_pLookup = NULL;
}

// Clears m_contents (NOT nyms, accounts, servers, or asset types.)

void MTRecordList::ClearContents()
{
    m_contents.clear();
}

// ------------------------------------------------
// RETRIEVE:
//


int MTRecordList::size()
{
    return m_contents.size();
}


bool MTRecordList::RemoveRecord(int nIndex)
{
    OT_ASSERT((nIndex >= 0) && (nIndex < static_cast<long>(m_contents.size())));
    m_contents.erase(m_contents.begin()+nIndex);
    return true;
}

weak_ptr_MTRecord MTRecordList::GetRecord(int nIndex)
{
    OT_ASSERT((nIndex >= 0) && (nIndex < static_cast<long>(m_contents.size())));
    weak_ptr_MTRecord wp_record(m_contents[nIndex]);
    return wp_record;
}

// ------------------------------------------------

















