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

#include <nmcrpc/JsonRpc.hpp>

#include <iostream>

/** Namespace used for Namecoin credentials.  */
const std::string NMC_NS = "ot";

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
