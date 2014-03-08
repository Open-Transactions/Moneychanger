/*  Namecoin RPC library.
 *  Copyright (C) 2013-2014  Daniel Kraft <d@domob.eu>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  See the distributed file COPYING for additional permissions in addition
 *  to those of the GNU Affero General Public License.
 */

/* Utility program to update names.  */

#include "JsonRpc.hpp"
#include "NamecoinInterface.hpp"
#include "NameRegistration.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

using namespace nmcrpc;

/**
 * Display the help message.
 */
static void
displayHelp ()
{
  std::cerr << "Usage: nmupdate COMMAND [OPTIONS]"
            << std::endl << std::endl;
  std::cerr << "Possible commands:" << std::endl;
  std::cerr << "  * help: Display this message." << std::endl;
  std::cerr << "  * list: List owned names and their expirey counter."
            << std::endl;
  std::cerr << "  * update NAME [VAL]: Update NAME to VAL (or existing value)."
            << std::endl;
  std::cerr << "  * send NAME ADDR [VAL]: Update NAME and send to ADDR."
            << std::endl;
  std::cerr << "  * update-multi FILE [VAL]: Update all names in FILE"
            << std::endl
            << "                             to VAL or"
            << " their current value." << std::endl;
  std::cerr << "  * send-multi FILE ADDR [VAL]: Send all names in FILE to ADDR."
            << std::endl;
}

/**
 * Read file into list of names.
 * @param fileName Name of file to read in.
 * @param names Store names here.
 * @throws std::runtime_error if reading fails.
 */
static void
readNames (const std::string& fileName, std::vector<std::string>& names)
{
  std::ifstream in (fileName.c_str ());
  if (!in)
    throw std::runtime_error ("Could not read list of names.");

  while (in)
    {
      std::string line;
      std::getline (in, line);

      if (!line.empty ())
        names.push_back (line);
    }
}

/**
 * Perform a name update operation on an array of names.
 * @param rpc Json RPC connection.
 * @param nc Namecoin high-level interface.
 * @param names Array of names to update.
 * @param hasVal Whether to set the value or leave it.
 * @param val The value, is ignored if !hasVal.
 * @param hasAddr Whether to send to a specified address.
 * @param addr Target address, ignored if !hasAddr.
 */
static void
performUpdate (JsonRpc& rpc, NamecoinInterface& nc,
               const std::vector<std::string>& names,
               bool hasVal, const std::string& val,
               bool hasAddr, const std::string& addr)
{
#ifdef CXX_11
  for (const auto& nm : names)
#else /* CXX_11  */
  for (std::vector<std::string>::const_iterator i = names.begin ();
       i != names.end (); ++i)
#endif /* CXX_11  */
    {
#ifndef CXX_11
      const std::string& nm = *i;
#endif /* !CXX_11  */

      std::cout << "Updating " << nm << ": ";
      NamecoinInterface::Name name = nc.queryName (nm);

      NameUpdate updater (rpc, nc, name);
      if (hasVal)
        updater.setValue (val);

      std::string txid;
      if (!hasAddr)
        txid = updater.execute ();
      else
        txid = updater.execute (nc.queryAddress (addr));

      std::cout << txid << std::endl;
    }
}

/**
 * Compare names for sorting by expiration date.
 * @param a First name.
 * @param b Second name.
 * @return True iff a expires later than b.
 */
static bool
compareNames (const NamecoinInterface::Name& a,
              const NamecoinInterface::Name& b)
{
  int diff = a.getExpireCounter () - b.getExpireCounter ();
  if (diff != 0)
    return (diff > 0);
  return a.getName () < b.getName ();
}

/* Functor instead of addName lambda if we don't have C++11.  */
#ifndef CXX_11
class addNameFunctor
{
private:

  std::vector<NamecoinInterface::Name>& names;

public:

  explicit inline
  addNameFunctor (std::vector<NamecoinInterface::Name>& n)
    : names(n)
  {}

  inline void
  operator() (const NamecoinInterface::Name& nm)
  {
    names.push_back (nm);
  }

};
#endif /* !CXX_11  */

/**
 * Main routine with the usual interface.
 */
int
main (int argc, char** argv)
{
  try
    {
      if (argc < 2)
        {
          displayHelp ();
          return EXIT_FAILURE;
        }
      const std::string command = argv[1];

      if (command == "help")
        {
          displayHelp ();
          return EXIT_SUCCESS;
        }

      RpcSettings settings;
      settings.readDefaultConfig ();
      JsonRpc rpc(settings);
      NamecoinInterface nc(rpc);

      if (command == "list")
        {
          std::vector<NamecoinInterface::Name> names;

#ifdef CXX_11
          const auto addName = [&names] (const NamecoinInterface::Name& nm)
            {
              names.push_back (nm);
            };
#else /* CXX_11  */
          addNameFunctor addName(names);
#endif /* CXX_11  */

          nc.forMyNames (addName);

          std::sort (names.begin (), names.end (), &compareNames);

#ifdef CXX_11
          for (const auto& el : names)
#else /* CXX_11  */
          std::vector<NamecoinInterface::Name>::const_iterator i;
          for (i = names.begin (); i != names.end (); ++i)
#endif /* CXX_11  */
            {
#ifndef CXX_11
              const NamecoinInterface::Name& el = *i;
#endif /* !CXX_11  */

              std::cout.width (30);
              if (!el.isExpired ())
                std::cout << el.getName () << ": "
                          << el.getExpireCounter () << std::endl;
            }
        }
      else
        {
          std::string passphrase;
          if (nc.needWalletPassphrase ())
            {
              std::cout << "Enter wallet passphrase: ";
              std::getline (std::cin, passphrase);
            }
          NamecoinInterface::WalletUnlocker unlock(nc);
          unlock.unlock (passphrase);

          std::vector<std::string> names;

          if (command == "update")
            {
              if (argc < 3 || argc > 4)
                throw std::runtime_error ("Expected: nmupdate update"
                                          " NAME [VAL]");

              const std::string name = argv[2];
              std::string val;
              if (argc == 4)
                val = argv[3];

              names.push_back (name);
              performUpdate (rpc, nc, names, argc == 4, val, false, "");
            }
          else if (command == "update-multi")
            {
              if (argc < 3 || argc > 4)
                throw std::runtime_error ("Expected: nmupdate update-multi"
                                          " FILE [VAL]");

              const std::string file = argv[2];
              std::string val;
              if (argc == 4)
                val = argv[3];

              readNames (file, names);
              performUpdate (rpc, nc, names, argc == 4, val, false, "");
            }
          else if (command == "send")
            {
              if (argc < 4 || argc > 5)
                throw std::runtime_error ("Expected: nmupdate send"
                                          " NAME ADDR [VAL]");

              const std::string name = argv[2];
              const std::string addr = argv[3];
              std::string val;
              if (argc == 5)
                val = argv[4];

              names.push_back (name);
              performUpdate (rpc, nc, names, argc == 5, val, true, addr);
            }
          else if (command == "send-multi")
            {
              if (argc < 4 || argc > 5)
                throw std::runtime_error ("Expected: nmupdate send-multi"
                                          " FILE ADDR [VAL]");

              const std::string file = argv[2];
              const std::string addr = argv[3];
              std::string val;
              if (argc == 5)
                val = argv[4];

              readNames (file, names);
              performUpdate (rpc, nc, names, argc == 5, val, true, addr);
            }
          else
            throw std::runtime_error ("Unknown command '" + command + "'.");
        }
    }
  catch (const JsonRpc::RpcError& exc)
    {
      std::cerr << "JSON-RPC error:" << std::endl;
      std::cerr << exc.getErrorMessage () << std::endl;
      return EXIT_FAILURE;
    }
  catch (const std::exception& exc)
    {
      std::cerr << "Error: " << exc.what () << std::endl;
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
