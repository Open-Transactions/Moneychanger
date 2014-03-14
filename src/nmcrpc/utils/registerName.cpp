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

/* Test program for name registration.  */

#include "JsonRpc.hpp"
#include "NamecoinInterface.hpp"
#include "NameRegistration.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace nmcrpc;

int
main (int argc, char** argv)
{
  try
    {
      if (argc != 2)
        throw std::runtime_error ("Usage: registerName STATE-FILE");
      const std::string fileName = argv[1];

      RpcSettings settings;
      settings.readDefaultConfig ();
      JsonRpc rpc(settings);
      NamecoinInterface nc(rpc);
      NameRegistration reg(rpc, nc);

      std::ifstream fileIn (fileName.c_str ());
      if (fileIn)
        {
          std::cout << "Found state file, reading it and processing further."
                    << std::endl;
          fileIn >> reg;
          fileIn.close ();
          
          if (reg.isFinished ())
            std::cout << "Activation finished." << std::endl;
          else if (reg.canActivate ())
            {
              reg.activate ();
              std::cout << "Activated the name." << std::endl;
            }
          else
            std::cout << "Please wait longer." << std::endl;
        }
      else
        {
          std::string name;
          std::cout << "Name to register: ";
          std::cin >> name;
          reg.registerName (nc.queryName (name));

          std::string value;
          std::cout << "Value to set: ";
          std::cin >> value;
          reg.setValue (value);
        }

      std::ofstream fileOut (fileName.c_str ());
      fileOut << reg;
      fileOut.close ();
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
