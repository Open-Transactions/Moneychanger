/*  Namecoin RPC library.
 *  Copyright (C) 2014  Daniel Kraft <d@domob.eu>
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

/* Test program for listing of (all) names.  */

#include "JsonRpc.hpp"
#include "NamecoinInterface.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace nmcrpc;

int
main ()
{
  RpcSettings settings;
  settings.readDefaultConfig ();
  JsonRpc rpc(settings);
  NamecoinInterface nc(rpc);

  unsigned cnt = 0;
  const auto cb = [&cnt] (const std::string& str)
    {
      /* FIXME: Adapt to change in call-back interface.  */
      std::cout << str << std::endl;
      ++cnt;
    };
  nc.forAllNames (cb);
  std::cout << "Total: " << cnt << " names" << std::endl;

  return EXIT_SUCCESS;
}
