//
//  MTRecord.cpp
//  OTClient module
//
//  Created by Chris Odom on 4/28/13.
//  Copyright (c) 2013 Monetas. All rights reserved.
//



#include <OTStorage.h>

#include <OTString.h>
#include <OTIdentifier.h>
#include <OTASCIIArmor.h>
#include <OTAccount.h>
#include <OTLedger.h>
#include <OTTransaction.h>
#include <OTWallet.h>
#include <OTIdentifier.h>
#include <OTPayment.h>

#include <OpenTransactions.h>
#include <OTAPI.h>
#include <OT_ME.h>

#include <OTLog.h>

#include "MTRecord.h"


const std::string Instrument_TypeStrings[] =
{
    // ------------------
    // OTCheque is derived from OTTrackable, which is derived from OTInstrument, which is
    // derived from OTScriptable, which is derived from OTContract.
    // ------------------
    "cheque",         // A cheque drawn on a user's account.
    "voucher",        // A cheque drawn on a server account (cashier's cheque aka banker's cheque)
    "invoice",        // A cheque with a negative amount. (Depositing this causes a payment out, instead of a deposit in.)
    // ------------------
    "payment plan",   // An OTCronItem-derived OTPaymentPlan, related to a recurring payment plan.
    "smart contract", // An OTCronItem-derived OTSmartContract, related to a smart contract.
    // ------------------
    "purse",          // An OTContract-derived OTPurse containing a list of cash OTTokens.
    // ------------------
    "ERROR_STATE"
};
// ---------------------------------------
const std::string & MTRecord_GetTypeString(int theType)
{
	return Instrument_TypeStrings[theType];
}
// ---------------------------------------
bool MTRecord::FormatAmount(std::string & str_output)
{
    if (m_str_amount.empty() || m_str_asset_id.empty()) // Need these to do the formatting.
    {
        OTLog::vOutput(0, "%s: Unable to format amount. Type: %s Amount: %s  Asset: %s",
                       __FUNCTION__, m_str_type.c_str(), m_str_amount.c_str(), m_str_asset_id.c_str());
        return false;
    }
    // -----------------------------------
    str_output = OTAPI_Wrap::It()->FormatAmount(m_str_asset_id,
                                                OTAPI_Wrap::It()->StringToLong(m_str_amount));
    return (!str_output.empty());
}
// ---------------------------------------
bool MTRecord::FormatDescription(std::string & str_output)
{
    OTString strDescription, strStatus, strKind;
    
    if (IsRecord())
        strStatus = "(record)";
    // -----------------------------
    else if (IsPending())
        strStatus = "(pending)";
    // -----------------------------
    if (IsOutgoing())
        strKind.Format("%s",
                       IsPending() ? "outgoing " : "sent ");
    else // incoming.
        strKind.Format("%s",
                       IsPending() ? "incoming " : (IsReceipt() ? "" : "received "));
    // -----------------------------
    if (IsRecord())
    {
        if (MTRecord::Transfer == this->GetRecordType())
            strDescription.Format("%s%s %s", strKind.Get(), "transfer", strStatus.Get());
        else
            strDescription.Format("%s%s %s", strKind.Get(), GetInstrumentType().c_str(), strStatus.Get());
    }
    else
    {
        strDescription.Format("%s %s%s", strStatus.Get(), strKind.Get(), GetInstrumentType().c_str());
    }
    // -----------------------------
    str_output = strDescription.Get();
    // -----------------------------
    return (!str_output.empty());
}
// ---------------------------------------
MTRecord::MTRecordType MTRecord::GetRecordType() const { return m_RecordType; }
// ---------------------------------------
// For completed records (not pending.)
//
bool MTRecord::CanDeleteRecord() const
{
    if (this->IsMail())
        return true;
    
    if (false == this->IsRecord())
        return false;

    if (this->IsPending())  // This may be superfluous given the above 'if' statement.
        return false;

    return true;
}
// ---------------------------------------
// For incoming, pending (not-yet-accepted) instruments.
//
bool MTRecord::CanAcceptIncoming() const
{
    if (this->IsOutgoing())
        return false;

    if (false == this->IsPending())
        return false;
    
    if (this->IsMail())
        return false;

    if (this->IsRecord()) // This may be superfluous given the above 'if' pending.
        return false;

    return true;
}
// ---------------------------------------
// For INcoming, pending (not-yet-accepted) instruments.
//
bool MTRecord::CanDiscardIncoming() const
{
    if (this->IsOutgoing())
        return false;

    if (false == this->IsPending())
        return false;
    
    if (this->IsMail())
        return false;
    
    if (this->IsRecord()) // This may be superfluous given the above 'if' pending.
        return false;
    
    if (this->IsReceipt()) // Receipts must be accepted, not discarded.
        return false;
    
    if (MTRecord::Transfer == this->GetRecordType()) // All incoming, pending instruments EXCEPT transfer can be discarded.
        return false;
    
    return true;
}
// ---------------------------------------
// For OUTgoing, pending (not-yet-accepted) instruments.
//
bool MTRecord::CanCancelOutgoing() const
{
    if (false == this->IsOutgoing())
        return false;
    
    if (false == this->IsPending())
        return false;
    
    if (this->IsMail())
        return false;
    
    if (this->IsRecord()) // This may be superfluous given the above 'if' pending.
        return false;

    if (this->IsReceipt()) // Receipts can't be canceled. (Probably superfluous.)
        return false;

    if (MTRecord::Transfer == this->GetRecordType()) // All outgoing, pending instruments EXCEPT transfer can be canceled.
        return false;

    return true;
}

// ***********************************************************

// For completed records (not pending.)
//
bool MTRecord::DeleteRecord()
{
    if (!this->CanDeleteRecord())
        return false;
    // -----------------------------
    if (m_str_server_id.empty() || m_str_nym_id.empty())
    {
        OTLog::vError("%s: Error: missing server id (%s) or nym id (%s)\n",
                      __FUNCTION__, m_str_server_id.c_str(), m_str_nym_id.c_str());
        return false;
    }
    // ------------------------------
    std::string str_using_account;
    
    if ((MTRecord::Transfer == this->GetRecordType()) ||
        (MTRecord::Receipt  == this->GetRecordType()))
    {
        if (m_str_account_id.empty())
        {
            OTLog::vError("%s: Error: missing account id for transfer or receipt.\n", __FUNCTION__);
            return false;
        }
        
        str_using_account = m_str_account_id;
    }
    else
        str_using_account = m_str_nym_id; // For instruments.
    // ------------------------------
    switch (this->GetRecordType())
    {
        // Delete from in-mail or out-mail.
        //
        case MTRecord::Mail:
        {
            if (m_bIsOutgoing) // outgoing mail
            {
                int32_t	nCount = OTAPI_Wrap::GetNym_OutmailCount(m_str_nym_id);
                for (int32_t nIndex = 0; nIndex < nCount; ++nIndex)
                {
                    const std::string str_contents(OTAPI_Wrap::GetNym_OutmailContentsByIndex(m_str_nym_id, nIndex));
                    
                    if (str_contents == m_str_contents) // found it.
                    {
                        return OTAPI_Wrap::Nym_RemoveOutmailByIndex(m_str_nym_id, nIndex);
                    }
                }
            }
            else // incoming mail
            {
                int32_t	nCount = OTAPI_Wrap::GetNym_MailCount(m_str_nym_id);
                for (int32_t nIndex = 0; nIndex < nCount; ++nIndex)
                {
                    const std::string str_contents(OTAPI_Wrap::GetNym_MailContentsByIndex(m_str_nym_id, nIndex));
                    
                    if (str_contents == m_str_contents) // found it.
                    {
                        return OTAPI_Wrap::Nym_RemoveMailByIndex(m_str_nym_id, nIndex);
                    }
                }
            }
            return false;
        }
            break;
        // --------------------------------------------
        case MTRecord::Transfer:   // Delete from asset account recordbox.
        case MTRecord::Receipt:    // Delete from asset account recordbox.
        case MTRecord::Instrument: // Delete from Nym's recordbox.
            break;
        // --------------------------------------------            
        default:
            OTLog::vError("%s: Unexpected type: %s\n",
                          __FUNCTION__, this->GetInstrumentType().c_str());
            return false;
    }
    // --------------------------------------------
    // The below section handles both the Nym's recordbox AND the Asset Account recordbox.
    //
    // ------------------------------
    if (0 == m_lTransactionNum)
    {
        OTLog::vError("%s: Error: Transaction number is 0, in recordbox for server id (%s), nym id (%s), acct id ()\n",
                      __FUNCTION__, m_str_server_id.c_str(), m_str_nym_id.c_str(), str_using_account.c_str());
        return false;
    }
    // ------------------------------
    const OTIdentifier  theServerID(m_str_server_id),
                        theNymID(m_str_nym_id),
                        theAcctID(str_using_account); // this last one sometimes contains NymID (see above.)
    
    OTLedger * pRecordbox = OTAPI_Wrap::OTAPI()->LoadRecordBox(theServerID, theNymID, theAcctID);
    OTCleanup<OTLedger> theRecordBoxAngel(pRecordbox);
    // --------------------------------------------
    if (NULL == pRecordbox)
    {
        OTLog::vError("%s: Failed loading record box for server ID (%s) nymID (%s) accountID (%s)\n",
                      __FUNCTION__, m_str_server_id.c_str(), m_str_nym_id.c_str(), str_using_account.c_str());
        return false; 
    }
    // ------------------------------------------
    // Find the receipt in the recordbox that correlates to this MTRecord.
    //
    int nIndex = pRecordbox->GetTransactionIndex(m_lTransactionNum);
    
    if ((-1) == nIndex)
    {
        OTLog::vError("%s: Error: Unable to find transaction %ld in recordbox for server id (%s), nym id (%s), acct id (%s)\n",
                      __FUNCTION__, m_lTransactionNum, m_str_server_id.c_str(), m_str_nym_id.c_str(), str_using_account.c_str());
        return false;
    }
    // ------------------------------------------
    // Accept it.
    //
    return OTAPI_Wrap::ClearRecord(m_str_server_id, m_str_nym_id, str_using_account, nIndex, false); //clear all = false. We're only clearing one record.
}
// ---------------------------------------
bool MTRecord::AcceptIncomingTransfer() { return this->AcceptIncomingTransferOrReceipt(); } // For incoming, pending (not-yet-accepted) transfers.
bool MTRecord::AcceptIncomingReceipt()  { return this->AcceptIncomingTransferOrReceipt(); } // For incoming, (not-yet-accepted) receipts.
// ---------------------------------------
bool MTRecord::AcceptIncomingTransferOrReceipt()
{
    if (!this->CanAcceptIncoming())
        return false;

    switch (this->GetRecordType())
    {
        // --------------------------------------------            
        // Accept transfer or receipt from asset account inbox.
        //
        case MTRecord::Transfer:
        case MTRecord::Receipt:
        {
            if (m_str_server_id.empty() || m_str_nym_id.empty() || m_str_account_id.empty())
            {
                OTLog::vError("%s: Error: missing server id (%s) or nym id (%s) or acct id (%s)\n",
                              __FUNCTION__, m_str_server_id.c_str(), m_str_nym_id.c_str(), m_str_account_id.c_str());
                return false;
            }
            // ------------------------------
            if (0 == m_lTransactionNum)
            {
                OTLog::vError("%s: Error: Transaction number is 0, in asset account inbox for server id (%s), nym id (%s)\n",
                              __FUNCTION__, m_str_server_id.c_str(), m_str_nym_id.c_str());
                return false;
            }
            // ------------------------------
            const OTIdentifier theServerID(m_str_server_id), theNymID(m_str_nym_id), theAcctID(m_str_account_id);
            
            // Open the Nym's asset account inbox.
            OTLedger * pInbox = OTAPI_Wrap::OTAPI()->LoadInbox(theServerID, theNymID, theAcctID);
            OTCleanup<OTLedger> theInboxAngel(pInbox);
            // ------------------------------------------
            if (NULL == pInbox)
            {
                OTLog::vError("%s: Error: Unable to load asset account inbox for server id (%s), nym id (%s)\n",
                              __FUNCTION__, m_str_server_id.c_str(), m_str_nym_id.c_str());
                return false;
            }
            // ------------------------------------------
            // Find the transfer/receipt therein that correlates to this MTRecord.
            //
            int nIndex = pInbox->GetTransactionIndex(m_lTransactionNum);
            
            if ((-1) == nIndex)
            {
                OTLog::vError("%s: Error: Unable to find transaction %ld in payment inbox for server id (%s), nym id (%s), acct id (%s)\n",
                              __FUNCTION__, m_lTransactionNum, m_str_server_id.c_str(), m_str_nym_id.c_str(), m_str_account_id.c_str());
                return false;
            }
            // ------------------------------------------
            // Accept it.
            //
            OTString strIndices;
            strIndices.Format("%d", nIndex);
            const std::string str_indices(strIndices.Get());
            
            OT_ME madeEasy;

            return madeEasy.accept_inbox_items(m_str_account_id, 0, str_indices);
        }
            break;            
        // --------------------------------------------            
        default:
            OTLog::vError("%s: Unexpected type: %s\n",
                          __FUNCTION__, this->GetInstrumentType().c_str());
            return false;
    }

    return true;
}
// ---------------------------------------
// For incoming, pending (not-yet-accepted) instruments.
//
bool MTRecord::AcceptIncomingInstrument(const std::string & str_into_acct)
{
    if (!this->CanAcceptIncoming())
        return false;

    switch (this->GetRecordType())
    {
        // --------------------------------------------
        // Accept from Nym's payments inbox.
        //
        case MTRecord::Instrument:
        {
            if (m_str_server_id.empty() || m_str_nym_id.empty())
            {
                OTLog::vError("%s: Error: missing server id (%s) or nym id (%s)\n",
                              __FUNCTION__, m_str_server_id.c_str(), m_str_nym_id.c_str());
                return false;
            }
            // ------------------------------
            if (0 == m_lTransactionNum)
            {
                OTLog::vError("%s: Error: Transaction number is 0, in payment inbox for server id (%s), nym id (%s)\n",
                              __FUNCTION__, m_str_server_id.c_str(), m_str_nym_id.c_str());
                return false;
            }
            // ------------------------------
            const OTIdentifier theServerID(m_str_server_id), theNymID(m_str_nym_id);
            
            // Open the Nym's payments inbox.
            OTLedger * pInbox = OTAPI_Wrap::OTAPI()->LoadPaymentInbox(theServerID, theNymID);
            OTCleanup<OTLedger> theInboxAngel(pInbox);
            // ------------------------------------------
            if (NULL == pInbox)
            {
                OTLog::vError("%s: Error: Unable to load payment inbox for server id (%s), nym id (%s)\n",
                              __FUNCTION__, m_str_server_id.c_str(), m_str_nym_id.c_str());
                return false;                
            }
            // ------------------------------------------
            // Find the payment therein that correlates to this MTRecord.
            //
            int nIndex = pInbox->GetTransactionIndex(m_lTransactionNum);
            
            if ((-1) == nIndex)
            {
                OTLog::vError("%s: Error: Unable to find transaction %ld in payment inbox for server id (%s), nym id (%s)\n",
                              __FUNCTION__, m_lTransactionNum, m_str_server_id.c_str(), m_str_nym_id.c_str());
                return false;
            }
            // ------------------------------------------
            // Accept it.
            //
            OTString strIndices;
            strIndices.Format("%d", nIndex);
            const std::string str_indices(strIndices.Get());
            
            OT_ME madeEasy;
            int32_t nReturn = madeEasy.accept_from_paymentbox(str_into_acct, str_indices, "ANY");
            
            switch (nReturn)
            {
                case 0:
                    OTLog::vOutput(0, "%s: This instrument was expired, so it was moved to the record box.\n", __FUNCTION__);
                    return true;
                
                case 1: // success
                    break;
                // ----------------------------------
                default:
                    OTLog::vError("%s: Error while trying to accept this instrument.\n", __FUNCTION__);
                    return false;
            } // switch
        } // case: instrument
            break;
        // --------------------------------------------            
        default:
            OTLog::vError("%s: Unexpected type: %s\n",
                          __FUNCTION__, this->GetInstrumentType().c_str());
            return false;
    }

    return true;
}
// ---------------------------------------
// For incoming, pending (not-yet-accepted) instruments.
//
bool MTRecord::DiscardIncoming()
{
    if (!this->CanDiscardIncoming())
        return false;
    
    switch (this->GetRecordType())
    {
        case MTRecord::Instrument:
        {
            if (m_str_server_id.empty() || m_str_nym_id.empty())
            {
                OTLog::vError("%s: Error: missing server id (%s) or nym id (%s)\n",
                              __FUNCTION__, m_str_server_id.c_str(), m_str_nym_id.c_str());
                return false;
            }
            // ------------------------------
            if (0 == m_lTransactionNum)
            {
                OTLog::vError("%s: Error: Transaction number is 0, in payment inbox for server id (%s), nym id (%s)\n",
                              __FUNCTION__, m_str_server_id.c_str(), m_str_nym_id.c_str());
                return false;
            }
            // ------------------------------
            const OTIdentifier theServerID(m_str_server_id), theNymID(m_str_nym_id);
            
            // Open the Nym's payments inbox.
            OTLedger * pInbox = OTAPI_Wrap::OTAPI()->LoadPaymentInbox(theServerID, theNymID);
            OTCleanup<OTLedger> theInboxAngel(pInbox);
            // ------------------------------------------
            if (NULL == pInbox)
            {
                OTLog::vError("%s: Error: Unable to load payment inbox for server id (%s), nym id (%s)\n",
                              __FUNCTION__, m_str_server_id.c_str(), m_str_nym_id.c_str());
                return false;
            }
            // ------------------------------------------
            // Find the payment therein that correlates to this MTRecord.
            //
            int nIndex = pInbox->GetTransactionIndex(m_lTransactionNum);
            
            if ((-1) == nIndex)
            {
                OTLog::vError("%s: Error: Unable to find transaction %ld in payment inbox for server id (%s), nym id (%s)\n",
                              __FUNCTION__, m_lTransactionNum, m_str_server_id.c_str(), m_str_nym_id.c_str());
                return false;
            }
            // ------------------------------------------
            // Accept it.
            //
            OTString strIndices;
            strIndices.Format("%d", nIndex);
            const std::string str_indices(strIndices.Get());
            
            OT_ME madeEasy;

            return madeEasy.discard_incoming_payments(m_str_server_id, m_str_nym_id, str_indices);

        } // case: instrument
            break;
        // --------------------------------------------
        default:
            OTLog::vError("%s: Unexpected type: %s\n",
                          __FUNCTION__, this->GetInstrumentType().c_str());
            return false;
    }
    
    return true;
}
// --------------------------------------------
bool MTRecord::IsTransfer() const { return (MTRecord::Transfer == m_RecordType); }
// --------------------------------------------
bool MTRecord::IsCheque() const
{
    if (!m_str_contents.empty())
    {
        OTString  strPayment(m_str_contents);
        OTPayment thePayment(strPayment);
        
        if (thePayment.IsValid() && thePayment.SetTempValues())
        {
            if (OTPayment::CHEQUE == thePayment.GetType())
                return true;
        }
    }
    return false;
}
// --------------------------------------------
bool MTRecord::IsVoucher() const
{
    if (!m_str_contents.empty())
    {
        OTString  strPayment(m_str_contents);
        OTPayment thePayment(strPayment);
        
        if (thePayment.IsValid() && thePayment.SetTempValues())
        {
            if (OTPayment::VOUCHER == thePayment.GetType())
                return true;
        }
    }
    return false;
}
// --------------------------------------------
bool MTRecord::IsContract() const
{
    if (!m_str_contents.empty())
    {
        OTString  strPayment(m_str_contents);
        OTPayment thePayment(strPayment);
        
        if (thePayment.IsValid() && thePayment.SetTempValues())
        {
            if (OTPayment::SMART_CONTRACT == thePayment.GetType())
                return true;
        }
    }
    return false;    
}
// --------------------------------------------
bool MTRecord::IsPaymentPlan() const
{
    if (!m_str_contents.empty())
    {
        OTString  strPayment(m_str_contents);
        OTPayment thePayment(strPayment);
        
        if (thePayment.IsValid() && thePayment.SetTempValues())
        {
            if (OTPayment::PAYMENT_PLAN == thePayment.GetType())
                return true;
        }
    }
    return false;    
}
// ---------------------------------------
// For outgoing, pending (not-yet-accepted) instruments.
//
bool MTRecord::CancelOutgoing(const std::string str_via_acct) // This can be blank if it's a cheque.
{
    if (!this->CanCancelOutgoing())
        return false;
    
    switch (this->GetRecordType())
    {
        case MTRecord::Instrument:
        {
            if (m_str_nym_id.empty())
            {
                OTLog::vError("%s: Error: missing nym id (%s)\n",
                              __FUNCTION__, m_str_nym_id.c_str());
                return false;
            }
            
            const OTIdentifier theNymID(m_str_nym_id);
            // ------------------------------
            if (0 == m_lTransactionNum)
            {
                OTLog::vError("%s: Error: Transaction number is 0, for outgoing payment for nym id (%s)\n",
                              __FUNCTION__, m_str_nym_id.c_str());
                return false;
            }
            // ------------------------------
            std::string str_using_acct;

            if (this->IsCheque())
            {
                str_using_acct = m_str_account_id;
            }
            else
            {
                str_using_acct = str_via_acct;
            }
            // ---------------------------------------
            if (str_using_acct.empty())
            {
                OTLog::vError("%s: Error: Missing account ID (FAILURE)\n", __FUNCTION__);
                return false;                
            }
            // ---------------------------------------
            // Find the payment in the Nym's outpayments box that correlates to this MTRecord.
            //
            int32_t nCount = OTAPI_Wrap::GetNym_OutpaymentsCount(m_str_nym_id);
            
            for (int32_t nIndex = 0; nIndex < nCount; ++nIndex)
            {
                std::string strOutpayment(OTAPI_Wrap::GetNym_OutpaymentsContentsByIndex(m_str_nym_id, nIndex));
                
                if (strOutpayment.empty())
                {
                    long lIndex = nIndex;
                    OTLog::vError("%s: Error: Blank outpayment at index %ld\n", __FUNCTION__, lIndex);
                    return false;
                }
                // ------------------------
                OTString  strPayment(strOutpayment);
                OTPayment thePayment(strPayment);
                
                if (!thePayment.IsValid() || !thePayment.SetTempValues())
                {
                    long lIndex = nIndex;
                    OTLog::vError("%s: Error: Invalid outpayment at index %ld\n", __FUNCTION__, lIndex);
                    return false;
                }
                // ------------------------
                long lTransNum = 0;
                thePayment.GetOpeningNum(lTransNum, theNymID);
                // ------------------------
                if (lTransNum == m_lTransactionNum) // Found it.
                {
                    long lIndex = nIndex;
                    OTString strIndices;
                    strIndices.Format("%ld", lIndex);
                    const std::string str_indices(strIndices.Get());
                    // ---------------------------------
                    OT_ME madeEasy;
                    
                    return madeEasy.cancel_outgoing_payments(m_str_nym_id, str_using_acct, str_indices);
                }
            }
        }
            break;
        // -----------------------------            
        default:
            OTLog::vError("%s: Unexpected type: %s\n",
                          __FUNCTION__, this->GetInstrumentType().c_str());
            return false;
    }
    
    return true;
}
// ---------------------------------------
long  MTRecord::GetTransactionNum() const         { return m_lTransactionNum;      } // Trans Num of receipt in the box. (Unless outpayment, contains number for instrument.)
// ---------------------------------------
void  MTRecord::SetTransactionNum(long lTransNum) { m_lTransactionNum = lTransNum; }
// ---------------------------------------
long  MTRecord::GetTransNumForDisplay() const
{
    if (m_lTransNumForDisplay > 0)
        return m_lTransNumForDisplay;
    
    return m_lTransactionNum;
}
void  MTRecord::SetTransNumForDisplay(long lTransNum) { m_lTransNumForDisplay = lTransNum; }
// ---------------------------------------
bool  MTRecord::IsMail()                          const { return MTRecord::Mail == this->GetRecordType(); }
// ---------------------------------------
bool  MTRecord::IsPending()                       const { return m_bIsPending;            }
bool  MTRecord::IsOutgoing()                      const { return m_bIsOutgoing;           }
bool  MTRecord::IsRecord()                        const { return m_bIsRecord;             }
bool  MTRecord::IsReceipt()                       const { return m_bIsReceipt;            }
bool  MTRecord::HasContents()                     const { return !m_str_contents.empty(); }
// ---------------------------------------
const std::string & MTRecord::GetServerID()       const { return m_str_server_id;         }
const std::string & MTRecord::GetAssetID()        const { return m_str_asset_id;          }
const std::string & MTRecord::GetCurrencyTLA()    const { return m_str_currency_tla;      }
const std::string & MTRecord::GetNymID()          const { return m_str_nym_id;            }
const std::string & MTRecord::GetAccountID()      const { return m_str_account_id;        }
const std::string & MTRecord::GetOtherNymID()     const { return m_str_other_nym_id;      }
const std::string & MTRecord::GetOtherAccountID() const { return m_str_other_account_id;  }
// ---------------------------------------
const std::string & MTRecord::GetName()           const { return m_str_name;              }
const std::string & MTRecord::GetDate()           const { return m_str_date;              }
const std::string & MTRecord::GetAmount()         const { return m_str_amount;            }
const std::string & MTRecord::GetInstrumentType() const { return m_str_type;              }
const std::string & MTRecord::GetContents()       const { return m_str_contents;          }
// ---------------------------------------
void  MTRecord::SetContents      (const std::string & str_contents) { m_str_contents         = str_contents; }
void  MTRecord::SetOtherNymID    (const std::string & str_ID)       { m_str_other_nym_id     = str_ID;       }
void  MTRecord::SetOtherAccountID(const std::string & str_ID)       { m_str_other_account_id = str_ID;       }
// ---------------------------------------
bool MTRecord::operator<(const MTRecord& rhs)     { return m_str_date < rhs.m_str_date; }
// ---------------------------------------
MTRecord::MTRecord(const std::string & str_server_id,
                   const std::string & str_asset_id,
                   const std::string & str_currency_tla,
                   const std::string & str_nym_id,
                   const std::string & str_account_id,
                   const std::string & str_name,
                   const std::string & str_date,
                   const std::string & str_amount,
                   const std::string & str_type,
                   bool  bIsPending,
                   bool  bIsOutgoing,
                   bool  bIsRecord,
                   bool  bIsReceipt,
                   MTRecordType eRecordType) :
m_str_server_id(str_server_id),
m_str_asset_id(str_asset_id),
m_str_currency_tla(str_currency_tla),
m_str_nym_id(str_nym_id),
m_str_account_id(str_account_id),
// ------------------------------
m_str_name(str_name),
m_str_date(str_date),
m_str_amount(str_amount),
m_str_type(str_type),
m_lTransactionNum(0),
m_lTransNumForDisplay(0),
m_bIsPending(bIsPending),
m_bIsOutgoing(bIsOutgoing),
m_bIsRecord(bIsRecord),
m_bIsReceipt(bIsReceipt),
m_RecordType(eRecordType) { }
// ---------------------------------------




