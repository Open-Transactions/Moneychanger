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

#ifndef MONEYCHANGER_NAMECOIN_HPP
#define MONEYCHANGER_NAMECOIN_HPP

#include <QSqlRecord>
#include <QString>

#include <list>
#include <stdexcept>
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

  // Disable copying.
#ifndef CXX_11
  NMC_Interface (const NMC_Interface&);
  NMC_Interface& operator= (const NMC_Interface&);
#endif /* !CXX_11  */

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
#ifdef CXX_11
  NMC_Interface (const NMC_Interface&) = delete;
  NMC_Interface& operator= (const NMC_Interface&) = delete;
#endif /* CXX_11?  */

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

  /** Singleton instance created (if there is one).  */
  static NMC_NameManager* instance;

  /**
   * Get the Namecoin name corresponding to a Nym / credentials pair.
   * @param nym Nym id.
   * @param cred Credentials hash.
   * @return Namecoin name at which to register the credentials.
   */
  nmcrpc::NamecoinInterface::Name getNameForNym (const QString& nym,
                                                 const QString& cred);

  /* Functor class replacing the lambda expression to add names to
     list of pending registrations in C++03 mode.  */
  /* FIXME: Replace by lambda expression if we can be C++11-only.  */
  class AddPendingRegFunctor
  {

  private:

    NMC_NameManager& me;

    // No default constructor.
    AddPendingRegFunctor ();

  public:
    
    inline explicit
    AddPendingRegFunctor (NMC_NameManager& m)
      : me(m)
    {}

    void operator() (const QSqlRecord& rec);

  };

public:

  /**
   * Construct with NMC_Interface to take the connections from.  It also
   * queries the database to fill in the pending registrations.
   * @param nmc NMC_Interface instance to use.
   */
  explicit NMC_NameManager (NMC_Interface& nmc);

  /**
   * Destruct, clearing the instance static variable.
   */
  ~NMC_NameManager ();

  /**
   * Start the name registration process of a new credential hash in the
   * Namecoin blockchain.
   * @param nym The Nym hash.
   * @param cred The credential hash.
   */
  void startRegistration (const QString& nym, const QString& cred);

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
  bool updateName (const QString& nym, const QString& cred);

  /**
   * Slot called regularly by a timer that handles all name updates
   * where appropriate.
   */
  void timerUpdate ();

  /**
   * Get the singleton instance.
   * @return The singleton instance.
   * @throws std::runtime_error if there is no instance.
   */
  static NMC_NameManager& getInstance ();

};

/* ************************************************************************** */
/* NMC_WalletUnlocker.  */

/**
 * Moneychanger specific wrapper around NamecoinInterface::WalletUnlocker that
 * automatically shows a password dialog when needed.
 */
class NMC_WalletUnlocker
{

private:

  /** High-level Namecoin interface used.  */
  nmcrpc::NamecoinInterface& nc;
  
  /** The "real" wallet unlocker object used behind-the-scenes.  */
  nmcrpc::NamecoinInterface::WalletUnlocker unlocker;

public:

  class UnlockFailure;

  /**
   * Construct it.  This doesn't yet show the dialog or performs unlocking,
   * so that no problems with memory freeing occur in case of exceptions.
   * @param n The high-level interface to use.
   * @see unlock() to perform the unlock itself.
   */
  explicit inline NMC_WalletUnlocker (nmcrpc::NamecoinInterface& n)
    : nc(n), unlocker(n)
  {
    // Nothing more to do.
  }

  /**
   * Try to unlock the wallet.  If a passphrase is needed, a dialog is shown
   * until the correct one is entered or the user cancels the action.  In the
   * latter case, UnlockFailure is thrown.
   * @throws UnlockFailure if the user cancels the unlock.
   */
  void unlock ();

};

/**
 * Exception code if the unlock dialog was cancelled by the user without
 * entering a valid passphrase.
 */
class NMC_WalletUnlocker::UnlockFailure : public std::runtime_error
{

private:

  // No default constructor.
#ifndef CXX_11
  UnlockFailure ();
#endif /* !CXX_11  */

public:

  /**
   * Construct it given the error message.
   * @param msg The error message.
   */
  explicit inline UnlockFailure (const std::string& msg)
    : std::runtime_error(msg)
  {
    // Nothing else to do.
  }

  /* No default constructor, but copying ok.  */
#ifdef CXX_11
  UnlockFailure () = delete;
  UnlockFailure (const UnlockFailure&) = default;
  UnlockFailure& operator= (const UnlockFailure&) = default;
#endif /* CXX_11?  */

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

  // Disable copying.
#ifndef CXX_11
  NMC_Verifier ();
  NMC_Verifier (const NMC_Verifier&);
  NMC_Verifier& operator= (const NMC_Verifier&);
#endif /* !CXX_11  */

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
#ifdef CXX_11
  NMC_Verifier () = delete;
  NMC_Verifier (const NMC_Verifier&) = delete;
  NMC_Verifier& operator= (const NMC_Verifier&) = delete;
#endif /* CXX_11?  */

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
