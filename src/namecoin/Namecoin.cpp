/*
    Namecoin.hpp
    Namecoin credential verification.

    Copyright (c) 2013-2014 by Daniel Kraft <d@domob.eu>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Namecoin.hpp"

#include <core/handlers/DBHandler.hpp>
#include <gui/ui/dlgpassword.hpp>

#include <nmcrpc/RpcSettings.hpp>

#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OTPassword.hpp>

#include <QDebug>
#include <QSqlField>

#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>

/** Namespace used for Namecoin credentials.  */
const std::string NMC_NS = "ot";

/* ************************************************************************** */
/* NMC_Interface.  */

/**
 * Construct the object.
 */
NMC_Interface::NMC_Interface ()
{
  nmcrpc::RpcSettings settings;
  settings.readDefaultConfig ();

  rpc = new nmcrpc::JsonRpc (settings);
  nc = new nmcrpc::NamecoinInterface (*rpc);
}

/**
 * Destroy everything.
 */
NMC_Interface::~NMC_Interface ()
{
  assert (rpc);
  assert (nc);

  delete rpc;
  delete nc;
}

/* ************************************************************************** */
/* NMC_NameManager.  */

/** Singleton instance created (if there is one).  */
NMC_NameManager* NMC_NameManager::instance = nullptr;

/**
 * Construct with NMC_Interface to take the connections from.  It also
 * queries the database to fill in the pending registrations.
 * @param nmc NMC_Interface instance to use.
 */
NMC_NameManager::NMC_NameManager (NMC_Interface& nmc)
  : rpc(nmc.getJsonRpc ()), nc(nmc.getNamecoin ()),
    pendingRegs()
{
  if (instance != nullptr)
    qDebug () << "Error: Already have a NMC_NameManager instance!";
  else
    instance = this;

  const QString query = "SELECT `regData` FROM `nmc_names`"
                        " WHERE (`regData` IS NOT NULL) AND (NOT `active`)";

  /* FIXME: Replace by lambda expression if we use C++11-only.  */
  AddPendingRegFunctor addPendingReg(*this);

  qDebug () << "Loading pending name registrations:";
  DBHandler::getInstance ()->queryMultiple (query, addPendingReg);
}

void
NMC_NameManager::AddPendingRegFunctor::operator() (const QSqlRecord& rec)
{
  nmcrpc::NameRegistration reg(me.rpc, me.nc);
  const QString val = rec.field("regData").value ().toString ();
  std::istringstream in(val.toStdString ());
  in >> reg;
  me.pendingRegs.push_back (reg);

  qDebug () << "  " << QString(reg.getName ().c_str ());
}

/**
 * Destruct, clearing the instance static variable.
 */
NMC_NameManager::~NMC_NameManager ()
{
  if (!instance)
    qDebug () << "Error: Expected instance variable to be"
                 " set for NMC_NameManager";
  instance = nullptr;
}

/**
 * Get the Namecoin name corresponding to a Nym / credentials pair.
 * @param nym Nym id.
 * @param cred Credentials hash.
 * @return Namecoin name at which to register the credentials.
 */
nmcrpc::NamecoinInterface::Name
NMC_NameManager::getNameForNym (const QString& nym, const QString& cred)
{
  return nc.queryName (NMC_NS, cred.toStdString ());
}

/**
 * Start the name registration process of a new credential hash in the
 * Namecoin blockchain.
 * @param nym The Nym hash.
 * @param cred The credential hash.
 */
void
NMC_NameManager::startRegistration (const QString& nym, const QString& cred)
{
  qDebug () << "Registering " << nym << " with credentials " << cred
            << " on the Namecoin blockchain.";

  try
    {
      const nmcrpc::NamecoinInterface::Name nm = getNameForNym (nym, cred);

      NMC_WalletUnlocker unlocker (nc);
      try
        {
          unlocker.unlock ();
        }
      catch (const NMC_WalletUnlocker::UnlockFailure& exc)
        {
          qDebug () << "Unlock failed.";
          return;
        }

      nmcrpc::NameRegistration reg(rpc, nc);
      reg.registerName (nm);
      /* Don't yet set the value (with the Namecoin signature).  We have to
         do a name_update anyway in order to send the name to its target
         address.  */

      std::ostringstream out;
      out << reg;

      const QString queryStr = "INSERT INTO `nmc_names`"
                               "  (`name`, `nym`, `cred`, `active`, `regData`)"
                               "  VALUES (:name, :nym, :cred, 0, :regData)";
#ifdef CXX_11
      std::unique_ptr<DBHandler::PreparedQuery> qu;
#else /* CXX_11?  */
      std::auto_ptr<DBHandler::PreparedQuery> qu;
#endif /* CXX_11?  */
      qu.reset (DBHandler::getInstance ()->prepareQuery (queryStr));
      qu->bind (":name", nm.getName ().c_str ());
      qu->bind (":nym", nym);
      qu->bind (":cred", cred);
      qu->bind (":regData", out.str ().c_str ());
      DBHandler::getInstance ()->runQuery (qu.release ());

      pendingRegs.push_back (reg);
    }
  catch (const nmcrpc::JsonRpc::RpcError& exc)
    {
      qDebug () << "NMC RPC Error: " << exc.getErrorMessage ().c_str ();
    }
  catch (const std::exception& exc)
    {
      qDebug () << "Error: " << exc.what ();
    }
}

/**
 * Update the name corresponding to a nym and credentials hash.  This assumes
 * that the name is already registered and belongs to the user, and tries
 * to send (name_update) it to the NMC address that is the Nym's source.   It
 * also sets the value to the correct signed credentials hash.  If the name
 * is not available or has been taken by someone else after expirey, false
 * is returned.
 * @param nym The Nym hash.
 * @param cred The credential hash.
 * @return True iff the name_update was issued successfully.
 */
bool
NMC_NameManager::updateName (const QString& nym, const QString& cred)
{
  std::string addrStr = OTAPI_Wrap::It()->GetNym_SourceForID (nym.toStdString ());
  const nmcrpc::NamecoinInterface::Address addr = nc.queryAddress (addrStr);

  if (!addr.isValid () || !addr.isMine ())
    {
      qDebug () << "Nym source " << addrStr.c_str ()
                << " is not a valid Namecoin address,"
                   " or it is not owned by you.";
      return false;
    }

  const nmcrpc::NamecoinInterface::Name nm = getNameForNym (nym, cred);
  nmcrpc::NameUpdate upd(rpc, nc, nm);

  /* The wallet needs to be already unlocked from the caller.  Otherwise,
     the timer update may ask for the password multiple times during a single
     update timer event, for instance.  */
  if (nc.needWalletPassphrase ())
    {
      qDebug () << "Wallet should be unlocked already for updateName(),"
                << " but is not.";
      return false;
    }

  const std::string sig = addr.signMessage (cred.toStdString ());
  nmcrpc::JsonRpc::JsonData data(Json::objectValue);
  data["nmcsig"] = sig;
  upd.setValue (nmcrpc::JsonRpc::encodeJson (data));

  try
    {
      const std::string txid = upd.execute (addr);

      QString queryStr = "UPDATE `nmc_names`"
                         "  SET `updateTx` = :txid"
                         "  WHERE `name` = :name";
#ifdef CXX_11
      std::unique_ptr<DBHandler::PreparedQuery> qu;
#else /* CXX_11?  */
      std::auto_ptr<DBHandler::PreparedQuery> qu;
#endif /* CXX_11?  */
      qu.reset (DBHandler::getInstance ()->prepareQuery (queryStr));
      qu->bind (":txid", txid.c_str ());
      qu->bind (":name", nm.getName ().c_str ());
      DBHandler::getInstance ()->runQuery (qu.release ());
      qu.release ();
    }
  catch (const nmcrpc::NamecoinInterface::NoPrivateKey& exc)
    {
      qDebug () << "Name cannot be updated, as you don't own the private key.";
      return false;
    }

  return true;
}

/**
 * Slot called regularly by a timer that handles all name updates
 * where appropriate.
 */
void
NMC_NameManager::timerUpdate ()
{
  qDebug () << "Namecoin update timer called.";

  /* See if we need to unlock the wallet and do it if necessary.  */
  NMC_WalletUnlocker unlocker (nc);
  try
    {
      bool needUnlock = false;
      for (regList::const_iterator i = pendingRegs.begin ();
           i != pendingRegs.end (); ++i)
        if (i->canActivate () || i->isFinished ())
          needUnlock = true;

      if (needUnlock)
        {
          qDebug () << "Need to unlock the wallet, trying to do it.";
          try
            {
              unlocker.unlock ();
            }
          catch (const NMC_WalletUnlocker::UnlockFailure& exc)
            {
              qDebug () << "Unlock failed, cancelling the timer update.";
              return;
            }
        }
      else
        qDebug () << "No operations necessary that need an unlocked wallet.";
    }
  catch (const nmcrpc::JsonRpc::RpcError& exc)
    {
      qDebug () << "NMC RPC Error: " << exc.getErrorMessage ().c_str ();
      return;
    }
  catch (const std::exception& exc)
    {
      qDebug () << "Error: " << exc.what ();
      return;
    }

  /* Go over pending registrations and try to update them.  Here, we have the
     try-block inside, so that if one of those names fails (because of whatever
     reason associated to this particular name), this doesn't prevent other
     names from updating.  */
  regList::iterator i = pendingRegs.begin ();
  while (i != pendingRegs.end ())
    {
      try
        {
          /* If a name registration is finished (i. e., the name_firstupdate
             is already confirmed), remove it from the list and perform the
             actual update.  */
          if (i->isFinished ())
            {
              qDebug () << "Registration finished for "
                        << i->getName ().c_str ();

              DBHandler& db = *DBHandler::getInstance ();

              QString queryStr = "UPDATE `nmc_names`"
                                 "  SET `regData` = NULL, `active` = 1"
                                 "  WHERE `name` = :name";
#ifdef CXX_11
              std::unique_ptr<DBHandler::PreparedQuery> qu;
#else /* CXX_11?  */
              std::auto_ptr<DBHandler::PreparedQuery> qu;
#endif /* CXX_11?  */
              qu.reset (db.prepareQuery (queryStr));
              qu->bind (":name", i->getName ().c_str ());
              db.runQuery (qu.release ());
              qu.release ();

              queryStr = "SELECT `nym`, `cred` FROM `nmc_names`"
                         "  WHERE `name` = :name";
              qu.reset (db.prepareQuery (queryStr));
              qu->bind (":name", i->getName ().c_str ());
              QSqlRecord rec = db.queryOne (qu.release ());

              const QString nym = rec.field ("nym").value ().toString ();
              const QString cred = rec.field ("cred").value ().toString ();
              const bool success = updateName (nym, cred);
              if (success)
                qDebug () << "Issued name_update successfully.";
              else
                qDebug () << "name_update failed.";

              i = pendingRegs.erase (i);
              continue;
            }

          /* If we can active (send the name_firstupdate), do it.  */
          if (i->canActivate ())
            {
              qDebug () << "Activating " << i->getName ().c_str ();

              i->activate ();

              std::ostringstream out;
              out << *i;

              const QString queryStr = "UPDATE `nmc_names`"
                                       "  SET `regData` = :regData"
                                       "  WHERE `name` = :name";
#ifdef CXX_11
              std::unique_ptr<DBHandler::PreparedQuery> qu;
#else /* CXX_11?  */
              std::auto_ptr<DBHandler::PreparedQuery> qu;
#endif /* CXX_11?  */
              qu.reset (DBHandler::getInstance ()->prepareQuery (queryStr));
              qu->bind (":name", i->getName ().c_str ());
              qu->bind (":regData", out.str ().c_str ());
              DBHandler::getInstance ()->runQuery (qu.release ());
            }
        }
      catch (const nmcrpc::JsonRpc::RpcError& exc)
        {
          qDebug () << "NMC RPC Error: " << exc.getErrorMessage ().c_str ();
        }
      catch (const std::exception& exc)
        {
          qDebug () << "Error: " << exc.what ();
        }

      ++i;
    }
}

/**
 * Get the singleton instance.
 * @return The singleton instance.
 * @throws std::runtime_error if there is no instance.
 */
NMC_NameManager&
NMC_NameManager::getInstance ()
{
  if (!instance)
    throw std::runtime_error ("No NMC_NameManager instance exists.");

  return *instance;
}

/* ************************************************************************** */
/* NMC_WalletUnlocker.  */

/**
 * Try to unlock the wallet.  If a passphrase is needed, a dialog is shown
 * until the correct one is entered or the user cancels the action.  In the
 * latter case, UnlockFailure is thrown.
 * @throws UnlockFailure if the user cancels the unlock.
 */
void
NMC_WalletUnlocker::unlock ()
{
  std::string pwd;

  qDebug () << "Trying to unlock the Namecoin wallet.";

  /* If we need a password, show the dialog.  */
  if (nc.needWalletPassphrase ())
    {
      OTPassword otPwd;

      MTDlgPassword dlg (nullptr, otPwd);
      dlg.setDisplay ("Your Namecoin wallet is locked.  For the operations to"
                      " proceed, please enter the passphrase to temporarily"
                      " unlock the wallet.");
      const int res = dlg.exec ();

      /* Return code is 0 for cancel button or closing the window.
         It is 1 in case of ok.  */
      if (res == 0)
        {
          qDebug () << "Wallet unlock was cancelled.";
          throw UnlockFailure("Wallet unlock was cancelled.");
        }

      dlg.extractPassword ();
      pwd = otPwd.getPassword ();
    }

  /* Now try to unlock.  If the passphrase is wrong, retry by a tail-recursive
     call to unlock().  */
  try
    {
      unlocker.unlock (pwd);
      qDebug () << "Unlock successful (or not necessary).";
    }
  catch (const nmcrpc::NamecoinInterface::UnlockFailure& exc)
    {
      qDebug () << "Wrong passphrase, retrying.";
      unlock ();
    }
  catch (const nmcrpc::JsonRpc::RpcError& exc)
    {
      qDebug () << "NMC RPC Error " << exc.getErrorCode ()
                << ": " << exc.getErrorMessage ().c_str ();
    }
  catch (const std::exception& exc)
    {
      qDebug () << "Error: " << exc.what ();
    }
}

/* ************************************************************************** */
/* NMC_Verifier.  */

/**
 * Verify a credentials hash.
 * @param hash The credentials hash.
 * @param source The source (i. e., Namecoin address in this case).
 * @return True iff the credentials are indeed valid for this source.
 */
bool
NMC_Verifier::verifyCredentialHashAtSource (const std::string& hash,
                                            const std::string& source)
{
  static const bool verbose = true;

  if (verbose)
    std::cout << "Verifying credential hash to Namecoin source: "
              << std::endl << "  " << hash << std::endl << "  " << source
              << std::endl;

  const nmcrpc::NamecoinInterface::Name nm = nc.queryName (NMC_NS, hash);

  try
    {
      const nmcrpc::JsonRpc::JsonData val = nm.getJsonValue ();
      const nmcrpc::JsonRpc::JsonData sigval = val["nmcsig"];

      if (!sigval.isString ())
        {
          if (verbose)
            std::cout << "'nmcsig' field is not present or no string."
                      << std::endl;
          return false;
        }
      const std::string sig = sigval.asString ();

      const nmcrpc::NamecoinInterface::Address addr = nm.getAddress ();
      if (source != addr.getAddress ())
        {
          if (verbose)
            std::cout << "Address of name is not Nym source." << std::endl;
          return false;
        }

      return addr.verifySignature (hash, sig);
    }
  catch (const nmcrpc::NamecoinInterface::NameNotFound& exc)
    {
      if (verbose)
        std::cout << "The name does not exist." << std::endl;
      return false;
    }
  catch (const nmcrpc::JsonRpc::JsonParseError& exc)
    {
      if (verbose)
        std::cout << "The name doesn't hold valid JSON data." << std::endl;
      return false;
    }
}
