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

/* Test program for message signing and verification.  */

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

  const std::string testAddr = "NFppu8bRjGVYTjyVrFZE9cGmjvzD6VUo5m";
  const std::string testMsg = "This is a test message.";
  const std::string testSig = "HH7ssAn7gH5579y5wcaLZTVtzxOk+R9lOwgn/V/Oh7S1LF"
                              "XZOxq2a1DWpPRTauGMDrz+KPCxhBFUVGP228Kt69s=";

  NamecoinInterface::Address addr;
  addr = nc.queryAddress (testAddr);
  assert (addr.verifySignature (testMsg, testSig));
  assert (!addr.verifySignature ("Wrong message.", testSig));
  assert (!addr.verifySignature (testMsg, "Wrong signature."));
  addr = nc.queryAddress ("NKj3oHnX9PMjJhD124Pi41wa2WoV43aFDW");
  assert (!addr.verifySignature (testMsg, testSig));
  addr = nc.queryAddress ("invalid-address");
  assert (!addr.verifySignature (testMsg, testSig));

  std::string signAddr, passphrase;
  std::cout << "Enter address for testing message signing (or 'auto'): ";
  getline (std::cin, signAddr);
  bool passphraseNeeded;
  passphraseNeeded = nc.needWalletPassphrase ();
  if (passphraseNeeded)
    {
      std::cout << "Enter wallet passphrase: ";
      getline (std::cin, passphrase);
    }

  try
    {
      NamecoinInterface::WalletUnlocker unlock(nc);
      unlock.unlock (passphrase);

      addr = nc.queryAddress ("NELdJ5BTQuV6gUrxuA9L7My2Q6CyXeR3Ud");
      try
        {
          addr.signMessage (testMsg);
          // This should have thrown.
          assert (false);
        }
      catch (const NamecoinInterface::NoPrivateKey& exc)
        {
          // This is expected.
        }

      if (signAddr == "auto")
        {
          addr = nc.createAddress ();
          std::cout << "Using signing address " << addr.getAddress ()
                    << " as test." << std::endl;
        }
      else
        addr = nc.queryAddress (signAddr);

      if (addr.isValid ())
        {
          const std::string sig = addr.signMessage (testMsg);
          assert (addr.verifySignature (testMsg, sig));
        }
      else
        std::cout << "Signing address is invalid, ignoring this test."
                  << std::endl;
    }
  catch (const NamecoinInterface::UnlockFailure& exc)
    {
      std::cout << "Unlock failed, ignoring those tests." << std::endl;
    }
  assert (passphraseNeeded == nc.needWalletPassphrase ());

  return EXIT_SUCCESS;
}
