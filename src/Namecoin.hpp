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

#ifndef MONEYCHANGER_NAMECOIN_HPP
#define MONEYCHANGER_NAMECOIN_HPP

#include <QString>

#include <list>
#include <string>

#include <nmcrpc/JsonRpc.hpp>
#include <nmcrpc/NamecoinInterface.hpp>
#include <nmcrpc/NameRegistration.hpp>

/** Namespace used for Namecoin credentials.  */
extern const std::string NMC_NS;

/* ************************************************************************** */
/* NMC_Interface.  */

/**
 * Utility class that can be instantiated to give access to the JSON-RPC
 * connection and high-level Namecoin interface classes of libnmcrpc.
 * The internal details (singletons / created for every use) are handled
 * transparently.
 */
class NMC_Interface
{

private:

  /** JSON-RPC interface used.  */
  nmcrpc::JsonRpc* rpc;
  /** High-level interface used.  */
  nmcrpc::NamecoinInterface* nc;

public:

  /**
   * Construct the object.
   */
  NMC_Interface ();

  /**
   * Destroy everything.
   */
  ~NMC_Interface ();

  // No copying.
  NMC_Interface (const NMC_Interface&) = delete;
  NMC_Interface& operator= (const NMC_Interface&) = delete;

  /**
   * Get JSON-RPC connection.
   * @return JSON-RPC connection that can be used.
   */
  inline nmcrpc::JsonRpc&
  getJsonRpc ()
  {
    return *rpc;
  }

  /**
   * Get high-level Namecoin interface.
   * @return High-level Namecoin interface to be used.
   */
  inline nmcrpc::NamecoinInterface&
  getNamecoin ()
  {
    return *nc;
  }

};

/* ************************************************************************** */
/* NMC_NameManager.  */

/**
 * Interface to the stored data about registered names as in the
 * database's nmc_names table.  It also holds a list of all the
 * nmcrpc::NameRegistration objects still in progress.
 */
class NMC_NameManager
{

private:

  /** JsonRpc connection to be used.  */
  nmcrpc::JsonRpc& rpc;
  /** High-level Namecoin interface to be used.  */
  nmcrpc::NamecoinInterface& nc;

  /** Type for array of NameRegistration objects.  */
  typedef std::list<nmcrpc::NameRegistration> regList;
  /** List of pending name registrations.  */
  regList pendingRegs;

public:

  /**
   * Construct with NMC_Interface to take the connections from.  It also
   * queries the database to fill in the pending registrations.
   * @param nmc NMC_Interface instance to use.
   */
  explicit NMC_NameManager (NMC_Interface& nmc);

  /**
   * Start the name registration process of a new credential hash in the
   * Namecoin blockchain.
   * @param nym The Nym hash.
   * @param cred The credential hash.
   */
  void startRegistration (const QString nym, const QString cred);

};

/* ************************************************************************** */
/* NMC_Verifier.  */

/**
 * Class to handle Namecoin credential verification.  It holds a handle
 * to libnmcrpc, and can be queried to verify credential hashes created
 * based on Namecoin addresses.
 */
class NMC_Verifier
{

private:

  /** Namecoin interface to use.  */
  nmcrpc::NamecoinInterface& nc;

public:

  /**
   * Construct the verifier, given the Namecoin interface to use.
   * @param n The Namecoin interface to use.
   */
  inline explicit NMC_Verifier (nmcrpc::NamecoinInterface& n)
    : nc(n)
  {
    // Nothing more to do.
  }

  // No default constructor or copying.
  NMC_Verifier () = delete;
  NMC_Verifier (const NMC_Verifier&) = delete;
  NMC_Verifier& operator= (const NMC_Verifier&) = delete;

  /**
   * Verify a credentials hash.
   * @param hash The credentials hash.
   * @param source The source (i. e., Namecoin address in this case).
   * @return True iff the credentials are indeed valid for this source.
   */
  bool verifyCredentialHashAtSource (const std::string& hash,
                                     const std::string& source);

};

/* ************************************************************************** */

#endif /* Header guard.  */
