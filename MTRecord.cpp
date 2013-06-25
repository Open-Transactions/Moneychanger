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
MTRecord::MTRecordType MTRecord::GetRecordType()  { return m_RecordType;       }
// ---------------------------------------
bool  MTRecord::IsPending()                       { return m_bIsPending;       }
bool  MTRecord::IsOutgoing()                      { return m_bIsOutgoing;      }
bool  MTRecord::IsRecord()                        { return m_bIsRecord;        }
bool  MTRecord::IsReceipt()                       { return m_bIsReceipt;       }
// ---------------------------------------
const std::string & MTRecord::GetServerID()       { return m_str_server_id;    }
const std::string & MTRecord::GetAssetID()        { return m_str_asset_id;     }
const std::string & MTRecord::GetCurrencyTLA()    { return m_str_currency_tla; }
const std::string & MTRecord::GetNymID()          { return m_str_nym_id;       }
const std::string & MTRecord::GetAccountID()      { return m_str_account_id;   }
// ---------------------------------------
const std::string & MTRecord::GetName()           { return m_str_name;         }
const std::string & MTRecord::GetDate()           { return m_str_date;         }
const std::string & MTRecord::GetAmount()         { return m_str_amount;       }
const std::string & MTRecord::GetInstrumentType() { return m_str_type;         }
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
m_bIsPending(bIsPending),
m_bIsOutgoing(bIsOutgoing),
m_bIsRecord(bIsRecord),
m_bIsReceipt(bIsReceipt),
m_RecordType(eRecordType) { }
// ---------------------------------------




