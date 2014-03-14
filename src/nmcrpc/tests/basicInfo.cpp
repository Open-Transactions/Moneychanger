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

/* Test program for high-level query of basic infos.  */

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

  std::string testMsg;
  bool ok = nc.testConnection (testMsg);
  std::cout << "Test result: " << testMsg << std::endl;
  assert (ok);
  ok = nc.testConnection ();
  assert (ok);

  NamecoinInterface::Address addr;
  addr = nc.queryAddress ("foobar-invalid-address");
  assert (!addr.isValid () && !addr.isMine ());
  addr = nc.queryAddress ("NFUJUGVzjTuef8bX7dd3BfXekfu8cdzkuH");
  assert (addr.isValid ());
  /* Can't check for mine, because this depends on the current wallet!
     Instead, print at least whether the address is mine or not.  */
  std::cout << addr.getAddress () << ": ";
  if (addr.isMine ())
    std::cout << "mine";
  else
    std::cout << "not mine";
  std::cout << std::endl;

  NamecoinInterface::Name name;
  name = nc.queryName ("id/domob");
  assert (name.exists () && !name.isExpired ());
  assert (!name.isExpired ());
  std::cout << name.getName () << ": " << name.getAddress ().getAddress ()
            << ", expires in " << name.getExpireCounter () << std::endl;
  name = nc.queryName ("id", "dani");
  assert (name.exists () && !name.isExpired ());
  assert (name.getJsonValue ()["email"].asString () == "d@domob.eu");
  try
    {
      name = nc.queryName ("name-is-not-yet-registered");
      assert (!name.exists ());
      name.getStringValue ();
      // Here the exception should be thrown.
      assert (false);
    }
  catch (const NamecoinInterface::NameNotFound& exc)
    {
      // This is expected.
    }
  name = nc.queryName ("a", "-");
  assert (name.exists () && name.isExpired ());

  return EXIT_SUCCESS;
}
