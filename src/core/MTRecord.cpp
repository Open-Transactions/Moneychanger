//
//  MTRecord.cpp
//  OTClient module
//
//  Created by Chris Odom on 4/28/13.
//  Copyright (c) 2013 Monetas. All rights reserved.
//

#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/MTRecord.hpp>

#include <opentxs/OpenTransactions.hpp>
#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OTString.hpp>
#include <opentxs/OTPaymentPlan.hpp>
#include <opentxs/OTPayment.hpp>
#include <opentxs/OTLedger.hpp>
#include <opentxs/OT_ME.hpp>
#include <opentxs/OTLog.hpp>

#include <algorithm>



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
    "cash",           // An OTContract-derived OTPurse containing a list of cash OTTokens.
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
//        OTLog::vOutput(0, "%s: Unable to format amount. Type: %s Amount: %s  Asset: %s",
//                       __FUNCTION__, m_str_type.c_str(), m_str_amount.c_str(), m_str_asset_id.c_str());
        return false;
    }
    // -----------------------------------
    str_output = OTAPI_Wrap::It()->FormatAmount(m_str_asset_id,
                                                OTAPI_Wrap::It()->StringToLong(m_str_amount));
    return (!str_output.empty());
}
// ---------------------------------------
bool MTRecord::FormatMailSubject(std::string & str_output)
{
    str_output.clear();
    // ------------------------
    if (IsMail())
    {
        if (!HasContents())
            str_output = "(empty subject)";
        else
        {
            std::string str_temp_contents = GetContents();

            if (str_temp_contents.compare(0,8,"Subject:") == 0)
            {
                // Make the replacement.
                str_temp_contents.replace(0, 8, "");
            }
            // -----------------------------------
            // Trim it, since there is probably a space after "Subject:"
            // (Plus we want it trimmed anyway.)
            std::string str_contents = OTString::trim(str_temp_contents);
            // -----------------------------------
            // Cut the string at the first newline.
            //
            std::string::size_type pos_start = 0;
            std::string::size_type pos       = str_contents.find("\r\n", pos_start);

            if (std::string::npos == pos) // Didn't find anything.
                pos = str_contents.find ("\n", pos_start);
            // -----------------------------------
            if (std::string::npos != pos) // We found a newline.
                str_contents.erase(pos, std::string::npos);
            // -----------------------------------
            // Trim it again, just for good measure.
            str_output = OTString::trim(str_contents);
        }
    }
    // -----------------------------
    return (!str_output.empty());
}
// ---------------------------------------
bool MTRecord::FormatShortMailDescription(std::string & str_output)
{
    OTString strDescription;

    if (IsMail())
    {
        if (!HasContents())
            strDescription.Set("(empty message)");
        else
        {
            std::string str_temp_contents = GetContents();
            std::string str_contents      = OTString::trim(str_temp_contents);

            if (str_contents.compare(0,8,"Subject:") == 0)
            {
                // Make the replacement.
                str_contents.replace(0, 8, "");
            }
            // -----------------------------------
            bool bTruncated = false;

            if (str_contents.size() > 30)
            {
                str_contents.erase(30, std::string::npos);
                bTruncated = true;
            }
            // -----------------------------------
            // Replace any newlines with spaces...
            //
            std::replace( str_contents.begin(), str_contents.end(), '\r', ' ');
            std::replace( str_contents.begin(), str_contents.end(), '\n', ' ');

//          str_contents.erase(std::remove(str_contents.begin(), str_contents.end(), '\n'), str_contents.end());
//          str_contents.erase(std::remove(str_contents.begin(), str_contents.end(), '\r'), str_contents.end());
            // -----------------------------------
            strDescription.Format("%s%s", OTString::trim(str_contents).c_str(),
                                  bTruncated ? "..." : "");
        }
    }
    // -----------------------------
    str_output = strDescription.Get();
    // -----------------------------
    return (!str_output.empty());
}
// ---------------------------------------
bool MTRecord::FormatDescription(std::string & str_output)
{
    OTString strDescription, strStatus, strKind;

    if (IsRecord())
    {
        if (IsExpired())
            strStatus = "(EXPIRED)"; // How to tell difference between instrument that processed successfully and THEN expired, versus one that expired before processing successfully? (See next comment.)
        else if (IsInvoice())
            strStatus = "(payment sent)";  // TODO: need an "invalid records" box for expired and canceled. Otherwise we may falsely assume "payment sent" when actually it expired. (We may also falsely assume payment expired when actually it was sent.) Solution is a new box.
        else
            strStatus = "";
//          strStatus = "(record)";
    }
    // -----------------------------
    else if (IsPending())
    {
        if (IsExpired())
            strStatus = "(EXPIRED)";
        else if (IsInvoice())
            strStatus = "(unpaid)";
        else if (!IsCash())
            strStatus = "(pending)";
    }
    // -----------------------------
    if (IsCanceled())
    {
        strStatus = "(CANCELED)";

        if (IsOutgoing() || IsReceipt())
            strKind.Format("%s", "sent ");
    }
    // -----------------------------
    else
    {
        if (IsOutgoing())
            strKind.Format("%s",
                           ((IsPending() && !IsCash()) ? "outgoing " : "sent "));
        else // incoming.
            strKind.Format("%s",
                           IsPending() ? "incoming " : (IsReceipt() ? "" : "received "));
    }
    // -----------------------------
    OTString strTransNumForDisplay;

    if (!IsCash())
        strTransNumForDisplay.Format(" #%ld", GetTransNumForDisplay());
    // -----------------------------
    if (IsRecord())
    {
        if (this->IsTransfer())
            strDescription.Format("%s%s%s %s", strKind.Get(), "transfer", strTransNumForDisplay.Get(), strStatus.Get());
        else if (this->IsVoucher())
            strDescription.Format("%s%s%s %s", strKind.Get(), "payment", strTransNumForDisplay.Get(), strStatus.Get());
        else if (this->IsReceipt())
        {
            std::string str_instrument_type;

            if (0 == GetInstrumentType().compare("transferReceipt"))
                str_instrument_type = "transfer";
            else if (0 == GetInstrumentType().compare("finalReceipt"))
                str_instrument_type = "final receipt (closed)";
            else if (0 == GetInstrumentType().compare("marketReceipt"))
            {
                const long lAmount = OTAPI_Wrap::It()->StringToLong(m_str_amount);

                // I *think* successful trades have a negative amount -- we'll find out!
                //
                if (lAmount == 0)
                    str_instrument_type = "failed trade";
                else
                {
                    str_instrument_type = "market trade (receipt)";
                }
            }
            else if (0 == GetInstrumentType().compare("chequeReceipt"))
            {
                const long lAmount = OTAPI_Wrap::It()->StringToLong(m_str_amount);

                // I paid OUT when this chequeReceipt came through. It must be a normal cheque that I wrote.
                if (lAmount <= 0) // Notice less than OR EQUAL TO 0 -- that's because a canceled cheque has a 0 value.
                    str_instrument_type = "cheque";
                else // I GOT PAID when this chequeReceipt came through. It must be an invoice I wrote, that someone paid.
                {
                    if (IsCanceled())
                        str_instrument_type = "invoice";
                    else
                        str_instrument_type = "invoice (payment received)";
                }
            }
            else if (0 == GetInstrumentType().compare("voucherReceipt"))
            {
                str_instrument_type = "payment";
            }
            else if (0 == GetInstrumentType().compare("paymentReceipt"))
            {
                const long lAmount = OTAPI_Wrap::It()->StringToLong(m_str_amount);

                if (!IsCanceled() && (lAmount > 0))
                    strKind.Set("received ");

                str_instrument_type = "recurring payment";
            }

            strDescription.Format("%s%s%s %s", strKind.Get(), str_instrument_type.c_str(), strTransNumForDisplay.Get(), strStatus.Get());
        }
        else
            strDescription.Format("%s%s%s %s", strKind.Get(), GetInstrumentType().c_str(), strTransNumForDisplay.Get(), strStatus.Get());
    }
    else
    {
        if (this->IsTransfer())
            strDescription.Format("%s %s%s%s", strStatus.Get(), strKind.Get(), "transfer", strTransNumForDisplay.Get());
        else if (this->IsVoucher())
            strDescription.Format("%s %s%s%s", strStatus.Get(), strKind.Get(), "payment", strTransNumForDisplay.Get());

        else if (this->IsReceipt())
        {
            std::string str_instrument_type;

            if (0 == GetInstrumentType().compare("transferReceipt"))
            {
                if (IsCanceled())
                    str_instrument_type = "transfer";
                else
                    str_instrument_type = "transfer (receipt)";
            }
            else if (0 == GetInstrumentType().compare("finalReceipt"))
                str_instrument_type = "final receipt";
            else if (0 == GetInstrumentType().compare("marketReceipt"))
            {
                const long lAmount = OTAPI_Wrap::It()->StringToLong(m_str_amount);

                // I *think* marketReceipts have negative value. We'll just test for non-zero.
                if (lAmount == 0)
                    str_instrument_type = "failed trade";
                else
                    str_instrument_type = "market trade (receipt)";
            }
            else if (0 == GetInstrumentType().compare("chequeReceipt"))
            {
                const long lAmount = OTAPI_Wrap::It()->StringToLong(m_str_amount);

                // I paid OUT when this chequeReceipt came through. It must be a normal cheque that I wrote.
                if (lAmount <= 0) // Notice less than OR EQUAL TO 0 -- that's because a canceled cheque has a 0 value.
                {
                    if (IsCanceled())
                        str_instrument_type = "cheque";
                    else
                        str_instrument_type = "cheque (receipt)";
                }
                else // I GOT PAID when this chequeReceipt came through. It must be an invoice I wrote, that someone paid.
                {
                    if (IsCanceled())
                        str_instrument_type = "invoice";
                    else
                        str_instrument_type = "sent invoice (receipt: payment received)";
                }
            }
            else if (0 == GetInstrumentType().compare("voucherReceipt"))
            {
                if (IsCanceled())
                    str_instrument_type = "payment";
                else
                    str_instrument_type = "payment (receipt)";
            }
            else if (0 == GetInstrumentType().compare("paymentReceipt"))
            {
                const long lAmount = OTAPI_Wrap::It()->StringToLong(m_str_amount);

                if (!IsCanceled() && (lAmount > 0))
                    strKind.Set("received ");

                str_instrument_type = "recurring payment (receipt)";
            }

            strDescription.Format("%s %s%s%s", strStatus.Get(), strKind.Get(), str_instrument_type.c_str(), strTransNumForDisplay.Get());
        }
        else
            strDescription.Format("%s %s%s%s", strStatus.Get(), strKind.Get(), GetInstrumentType().c_str(), strTransNumForDisplay.Get());
    }
    // -----------------------------
    str_output = strDescription.Get();
    // -----------------------------
    return (!str_output.empty());
}
// ---------------------------------------
bool MTRecord::HasInitialPayment()
{
    if (!IsPaymentPlan())
        return false;

    OTPaymentPlan  thePlan;
    const OTString strPlan(GetContents().c_str());

    if (thePlan.LoadContractFromString(strPlan) &&
        thePlan.HasInitialPayment())
        return true;
    return false;
}

bool MTRecord::HasPaymentPlan()
{
    if (!IsPaymentPlan())
        return false;

    OTPaymentPlan  thePlan;
    const OTString strPlan(GetContents().c_str());

    if (thePlan.LoadContractFromString(strPlan) &&
        thePlan.HasPaymentPlan())
        return true;
    return false;
}

time_t MTRecord::GetInitialPaymentDate()
{
    if (!IsPaymentPlan())
        return 0;

    OTPaymentPlan  thePlan;
    const OTString strPlan(GetContents().c_str());

    if (thePlan.LoadContractFromString(strPlan) &&
        thePlan.HasInitialPayment())
        return thePlan.GetInitialPaymentDate();
    return 0;
}

long MTRecord::GetInitialPaymentAmount()
{
    if (!IsPaymentPlan())
        return 0;

    OTPaymentPlan  thePlan;
    const OTString strPlan(GetContents().c_str());

    if (thePlan.LoadContractFromString(strPlan) &&
        thePlan.HasInitialPayment())
        return thePlan.GetInitialPaymentAmount();
    return 0;
}


time_t MTRecord::GetPaymentPlanStartDate()
{
    if (!IsPaymentPlan())
        return 0;

    OTPaymentPlan  thePlan;
    const OTString strPlan(GetContents().c_str());

    if (thePlan.LoadContractFromString(strPlan) &&
        thePlan.HasPaymentPlan())
        return thePlan.GetPaymentPlanStartDate();
    return 0;
}

time_t MTRecord::GetTimeBetweenPayments()
{
    if (!IsPaymentPlan())
        return 0;

    OTPaymentPlan  thePlan;
    const OTString strPlan(GetContents().c_str());

    if (thePlan.LoadContractFromString(strPlan) &&
        thePlan.HasPaymentPlan())
        return thePlan.GetTimeBetweenPayments();
    return 0;
}

long MTRecord::GetPaymentPlanAmount()
{
    if (!IsPaymentPlan())
        return 0;

    OTPaymentPlan  thePlan;
    const OTString strPlan(GetContents().c_str());

    if (thePlan.LoadContractFromString(strPlan) &&
        thePlan.HasPaymentPlan())
        return thePlan.GetPaymentPlanAmount();
    return 0;
}

int MTRecord::GetMaximumNoPayments()
{
    if (!IsPaymentPlan())
        return 0;

    OTPaymentPlan  thePlan;
    const OTString strPlan(GetContents().c_str());

    if (thePlan.LoadContractFromString(strPlan) &&
        thePlan.HasPaymentPlan())
        return thePlan.GetMaximumNoPayments();
    return 0;
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
    // Commented out because a transferReceipt is in the inbox, but it represents an outgoing payment.
//    if (this->IsOutgoing()) // If it's outgoing, then it's definitely not an incoming thing you can accept.
//        return false;

    if (this->IsRecord()) // Records must be archived or deleted, not accepted or discarded.
        return false;

    if (this->IsExpired())
        return false;

    if (this->IsReceipt()) // It's NOT a record... If it's a receipt, then yes, we can accept it.
        return true;

    if (this->IsMail()) // Can't "accept" mail, can only delete it.
        return false;

    if (this->IsPending() && this->IsOutgoing()) // It's not a record, it's not a receipt. If it's pending, is it Outgoing pending? (Can only accept INCOMING pending, not outgoing.)
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

    if (!this->IsPending())
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
bool MTRecord::CanDiscardOutgoingCash() const  // For OUTgoing cash. (No way to see if it's been accepted, so this lets you erase the record of sending it.)
{
    if (false == this->IsOutgoing())
        return false;

    if (false == this->IsPending())
        return false;

    if (false == this->IsCash())
        return false;

    if (!(GetBoxIndex() >= 0))
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

    if (this->IsCanceled()) // It's already canceled!
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

bool MTRecord::DiscardOutgoingCash()
{
    if (!this->CanDiscardOutgoingCash())
        return false;
    // -----------------------------
    return OTAPI_Wrap::It()->Nym_RemoveOutpaymentsByIndex(m_str_nym_id, GetBoxIndex());
}

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
                int32_t	nCount = OTAPI_Wrap::It()->GetNym_OutmailCount(m_str_nym_id);
                for (int32_t nIndex = 0; nIndex < nCount; ++nIndex)
                {
                    const std::string str_contents(OTAPI_Wrap::It()->GetNym_OutmailContentsByIndex(m_str_nym_id, nIndex));

                    if (str_contents == m_str_contents) // found it.
                    {
                        return OTAPI_Wrap::It()->Nym_RemoveOutmailByIndex(m_str_nym_id, nIndex);
                    }
                }
            }
            else // incoming mail
            {
                int32_t	nCount = OTAPI_Wrap::It()->GetNym_MailCount(m_str_nym_id);
                for (int32_t nIndex = 0; nIndex < nCount; ++nIndex)
                {
                    const std::string str_contents(OTAPI_Wrap::It()->GetNym_MailContentsByIndex(m_str_nym_id, nIndex));

                    if (str_contents == m_str_contents) // found it.
                    {
                        return OTAPI_Wrap::It()->Nym_RemoveMailByIndex(m_str_nym_id, nIndex);
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
    return OTAPI_Wrap::It()->ClearRecord(m_str_server_id, m_str_nym_id, str_using_account, nIndex, false); //clear all = false. We're only clearing one record.
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
bool MTRecord::IsTransfer()    const  { return (MTRecord::Transfer == m_RecordType); }
// --------------------------------------------
bool MTRecord::IsCash()        const  { return m_bIsCash;          }
bool MTRecord::IsInvoice()     const  { return m_bIsInvoice;       }
bool MTRecord::IsCheque()      const  { return m_bIsCheque;        }
bool MTRecord::IsVoucher()     const  { return m_bIsVoucher;       }
bool MTRecord::IsContract()    const  { return m_bIsSmartContract; }
bool MTRecord::IsPaymentPlan() const  { return m_bIsPaymentPlan;   }
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
            // ------------------------------
            if (0 == m_lTransactionNum)
            {
                if (IsCash())
                {
                    // Maybe it's cash...
                    std::string strOutpayment(OTAPI_Wrap::It()->GetNym_OutpaymentsContentsByIndex(m_str_nym_id, GetBoxIndex()));

                    if (strOutpayment.empty())
                    {
                        long lIndex = static_cast<long>(GetBoxIndex());
                        OTLog::vError("%s: Error: Blank outpayment at index %ld\n", __FUNCTION__, lIndex);
                        return false;
                    }
                    // ------------------------
                    OTString  strPayment(strOutpayment);
                    OTPayment thePayment(strPayment);

                    if (!thePayment.IsValid() || !thePayment.SetTempValues())
                    {
                        long lIndex = static_cast<long>(GetBoxIndex());
                        OTLog::vError("%s: Error: Invalid outpayment at index %ld\n", __FUNCTION__, lIndex);
                        return false;
                    }
                    // ------------------------
                    int64_t lTransNum = 0;
                    thePayment.GetOpeningNum(lTransNum, theNymID);
                    // ------------------------
                    if (0 == lTransNum) // Found it.
                    {
                        long lIndex = static_cast<long>(GetBoxIndex());
                        OTString strIndices;
                        strIndices.Format("%ld", lIndex);
                        const std::string str_indices(strIndices.Get());
                        // ---------------------------------
                        OT_ME madeEasy;

                        return madeEasy.cancel_outgoing_payments(m_str_nym_id, str_using_acct, str_indices);
                    }
                    else
                    {
                        OTLog::vError("%s: Error: Transaction number is non-zero (%ld), for cash outgoing payment for nym id (%s)\n",
                                      __FUNCTION__, lTransNum, m_str_nym_id.c_str());
                        return false;
                    }
                } // IsCash()
                else
                {
                    OTLog::vError("%s: Error: Transaction number is 0, for non-cash outgoing payment for nym id (%s)\n",
                                  __FUNCTION__, m_str_nym_id.c_str());
                    return false;
                }
            }
            // ---------------------------------------
            // Find the payment in the Nym's outpayments box that correlates to this MTRecord.
            //
            int32_t nCount = OTAPI_Wrap::It()->GetNym_OutpaymentsCount(m_str_nym_id);

            for (int32_t nIndex = 0; nIndex < nCount; ++nIndex)
            {
                std::string strOutpayment(OTAPI_Wrap::It()->GetNym_OutpaymentsContentsByIndex(m_str_nym_id, nIndex));

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
                int64_t lTransNum = 0;
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
            } // for
            // ---------------------------------------------------
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
void  MTRecord::SetExpired()   { m_bIsExpired  = true; }
void  MTRecord::SetCanceled()  { m_bIsCanceled = true; }
// ---------------------------------------
bool  MTRecord::IsMail()                          const { return MTRecord::Mail == this->GetRecordType(); }
// ---------------------------------------
bool  MTRecord::IsPending()                       const { return m_bIsPending;            }
bool  MTRecord::IsOutgoing()                      const { return m_bIsOutgoing;           }
bool  MTRecord::IsRecord()                        const { return m_bIsRecord;             }
bool  MTRecord::IsReceipt()                       const { return m_bIsReceipt;            }
bool  MTRecord::HasContents()                     const { return !m_str_contents.empty(); }
bool  MTRecord::HasMemo()                         const { return !m_str_memo.empty();     }
// ---------------------------------------
bool  MTRecord::IsExpired()                       const { return m_bIsExpired;            }
bool  MTRecord::IsCanceled()                      const { return m_bIsCanceled;           }
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
const std::string & MTRecord::GetMemo()           const { return m_str_memo;              }
const std::string & MTRecord::GetContents()       const { return m_str_contents;          }
// ---------------------------------------
int   MTRecord::GetBoxIndex() const         { return m_nBoxIndex;      }
void  MTRecord::SetBoxIndex(int nBoxIndex)  { m_nBoxIndex = nBoxIndex; }
// ---------------------------------------
void  MTRecord::SetMemo          (const std::string & str_memo) { m_str_memo             = str_memo; }
void  MTRecord::SetOtherNymID    (const std::string & str_ID)   { m_str_other_nym_id     = str_ID;   }
void  MTRecord::SetOtherAccountID(const std::string & str_ID)   { m_str_other_account_id = str_ID;   }
// ---------------------------------------
void  MTRecord::SetContents      (const std::string & str_contents)
{
    m_str_contents = str_contents;

    if (!m_str_contents.empty() && (MTRecord::Instrument == this->GetRecordType()))
    {
        OTString  strPayment(m_str_contents);
        OTPayment thePayment(strPayment);

        if (thePayment.IsValid() && thePayment.SetTempValues())
        {
            switch (thePayment.GetType())
            {
                case OTPayment::PURSE:          m_bIsCash           = true;  break;
                case OTPayment::CHEQUE:         m_bIsCheque         = true;  break;
                case OTPayment::VOUCHER:        m_bIsVoucher        = true;  break;
                case OTPayment::INVOICE:        m_bIsInvoice        = true;  break;
                case OTPayment::PAYMENT_PLAN:   m_bIsPaymentPlan    = true;  break;
                case OTPayment::SMART_CONTRACT: m_bIsSmartContract  = true;  break;

                default:
                    break;
            }
        }
    }
}
// ---------------------------------------
time_t MTRecord::GetValidFrom() { return m_ValidFrom; }
// ---------------------------------------
time_t MTRecord::GetValidTo()   { return m_ValidTo;   }
// ---------------------------------------
void MTRecord::SetDateRange(time_t tValidFrom, time_t tValidTo)
{
    m_ValidFrom = tValidFrom;
    m_ValidTo   = tValidTo;
    // ----------------------------------------------------------
    time_t tCurrentTime = static_cast<time_t>(OTAPI_Wrap::It()->GetTime());
    // ----------------------------------------------------------
    if ((tValidTo > 0) && (tCurrentTime > tValidTo) && !IsMail() && !IsRecord())
        SetExpired();
}
// ---------------------------------------
bool MTRecord::operator<(const MTRecord& rhs)
{ return m_ValidFrom < rhs.m_ValidFrom; }
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
m_nBoxIndex(-1),
m_ValidFrom(0),
m_ValidTo(0),
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
m_bIsPaymentPlan(false),
m_bIsSmartContract(false),
m_bIsVoucher(false),
m_bIsCheque(false),
m_bIsInvoice(false),
m_bIsCash(false),
m_bIsExpired(false),
m_bIsCanceled(false),
m_RecordType(eRecordType) { }
// ---------------------------------------




