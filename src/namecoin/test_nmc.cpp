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

/* Main program that implements Namecoin credential verification.  */

#include "Namecoin.hpp"

#include <nmcrpc/JsonRpc.hpp>
#include <nmcrpc/NamecoinInterface.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

using namespace nmcrpc;

int
main (int argc, char** argv)
{
  if (argc != 3)
    {
      std::cerr << "Usage: test_nmc SOURCE HASH" << std::endl;
      return EXIT_FAILURE;
    }
  const std::string source = argv[1];
  const std::string hash = argv[2];

  RpcSettings settings;
  settings.readDefaultConfig ();
  JsonRpc rpc(settings);
  NamecoinInterface nc(rpc);
  
  NMC_Verifier verify(nc);
  const bool res = verify.verifyCredentialHashAtSource (hash, source);

  if (res)
    std::cout << "Credential hash is ok." << std::endl;
  else
    std::cout << "Verification failed." << std::endl;

  return EXIT_SUCCESS;
}
