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

#include <string>

#include <nmcrpc/JsonRpc.hpp>
#include <nmcrpc/NamecoinInterface.hpp>

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
