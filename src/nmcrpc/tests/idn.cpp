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

/* Test program for IDN tools.  */

#include "IdnTool.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>

using namespace nmcrpc;

/**
 * Perform a round-trip test.  That is, decode IDN and encode it back, and
 * check that the strings match up.
 * @param idn IDN tool object to use.
 * @param str String to decode and re-encode.
 */
void
roundtrip (IdnTool& idn, const std::string& str)
{
  const std::string dec = idn.decode (str);
  const std::string enc = idn.encode (dec);

  assert (str == enc);
}

int
main ()
{
  IdnTool idn(true);
  std::string strIn, strEnc, strOut;

  std::cout << "Enter string: ";
  std::cin >> strIn;

  strEnc = idn.encode (strIn);
  std::cout << "Encoded: " << strEnc << std::endl;

  strOut = idn.decode (strEnc);
  std::cout << "Decoded: " << strOut << std::endl;

  roundtrip (idn, "d/xn--mnchen-3ya");
  roundtrip (idn, "xn--mnchen-3ya");
  roundtrip (idn, "d/foobar");
  roundtrip (idn, "foobar");
  roundtrip (idn, "id/xn--danielines-v5a0tyc");

  return EXIT_SUCCESS;
}
