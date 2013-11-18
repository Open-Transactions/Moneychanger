/*
    Namecoin.hpp
    Namecoin credential verification.

    Copyright (c) 2013 by Daniel Kraft <d@domob.eu>

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

#include "DBHandler.h"

#include <nmcrpc/RpcSettings.hpp>

#include <QDebug>
#include <QSqlField>
#include <QSqlRecord>

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

/**
 * Construct with NMC_Interface to take the connections from.  It also
 * queries the database to fill in the pending registrations.
 * @param nmc NMC_Interface instance to use.
 */
NMC_NameManager::NMC_NameManager (NMC_Interface& nmc)
  : rpc(nmc.getJsonRpc ()), nc(nmc.getNamecoin ()),
    pendingRegs()
{
  const QString query = "SELECT `regData` FROM `nmc_names`"
                        " WHERE (`regData` IS NOT NULL) AND (NOT `active`)";

  const auto addPendingReg = [this] (const QSqlRecord& rec)
    {
      nmcrpc::NameRegistration reg(rpc, nc);
      const QString val = rec.field("regData").value ().toString ();
      std::istringstream in(val.toStdString ());
      in >> reg;
      pendingRegs.push_back (reg);

      qDebug () << "  " << QString(reg.getName ().c_str ());
    };
  qDebug () << "Loading pending name registrations:";
  DBHandler::getInstance ()->queryMultiple (query, addPendingReg);
}

/**
 * Start the name registration process of a new credential hash in the
 * Namecoin blockchain.
 * @param nym The Nym hash.
 * @param cred The credential hash.
 */
void
NMC_NameManager::startRegistration (const QString nym, const QString cred)
{
  qDebug () << "Registering " << nym << " with credentials " << cred
            << " on the Namecoin blockchain.";

  try
    {
      nmcrpc::NamecoinInterface::Name nm;
      nm = nc.queryName (NMC_NS, cred.toStdString ());

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
      std::unique_ptr<DBHandler::PreparedQuery> qu;
      qu.reset (DBHandler::getInstance ()->prepareQuery (queryStr));
      qu->bind (":name", nm.getName ().c_str ());
      qu->bind (":nym", nym);
      qu->bind (":cred", cred);
      qu->bind (":regData", out.str ().c_str ());
      DBHandler::getInstance ()->runQuery (qu.release ());

      pendingRegs.push_back (reg);
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
