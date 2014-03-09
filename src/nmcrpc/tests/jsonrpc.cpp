/*  Namecoin RPC library.
 *  Copyright (C) 2013  Daniel Kraft <d@domob.eu>
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

/* Test program for the JSON-RPC interface.  */

#include "JsonRpc.hpp"

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

  JsonRpc::JsonData res = rpc.executeRpc ("getinfo");
  assert (res.isObject ());
  std::cout << "Running version: " << res["version"].asInt () << std::endl;

  res = rpc.executeRpc ("name_show", "id/domob");
  assert (res.isObject ());
  assert (res["name"].asString () == "id/domob");
  assert (res["value"].isString ());
  assert (res["expires_in"].isInt ());

  try
    {
      rpc.executeRpc ("method-does-not-exist", 5, "");
      assert (false);
    }
  catch (const JsonRpc::RpcError& err)
    {
      assert (err.getErrorCode () == -32601);
    }

  try
    {
      rpc.executeRpc ("name_history", "name-does-not-exist");
      assert (false);
    }
  catch (const JsonRpc::RpcError& err)
    {
      assert (err.getErrorCode () == -4);
    }

  return EXIT_SUCCESS;
}
