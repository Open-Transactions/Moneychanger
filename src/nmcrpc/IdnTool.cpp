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

/* Source code for IdnTool.hpp.  */

#include "IdnTool.hpp"

#include "NamecoinInterface.hpp"

#include <idna.h>
#include <stringprep.h>

#include <cassert>
#include <clocale>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>

namespace nmcrpc
{

/**
 * Default constructor.  Allow to specify whether or not to handle
 * namespaces, default is true.
 * Also call setlocale() to properly set up the locale settings
 * from the environment.
 * @param ns Handle namespaces for codings?
 */
IdnTool::IdnTool (bool ns)
  : handleNS(ns)
{
  std::setlocale (LC_ALL, "");
}

/**
 * Decode IDN to native locale string.  This routine handles the full string
 * and does not split off a namespace prefix.
 * @param in Input string, encoded as IDN.
 * @return Decoded output string.
 */
std::string
IdnTool::decodeFull (const std::string& in) const
{
  char* buf = nullptr;

  const Idna_flags flags = static_cast<Idna_flags> (0);
  Idna_rc rc;
  rc = static_cast<Idna_rc> (idna_to_unicode_lzlz (in.c_str (), &buf, flags));

  if (rc != IDNA_SUCCESS)
    {
      std::free (buf);
      std::ostringstream msg;
      msg << "IDNA decoding failed: " << idna_strerror (rc);
      throw std::runtime_error (msg.str ());
    }
  assert (buf);

  const std::string res(buf);
  std::free (buf);

  return res;
}

/**
 * Encode a native string to IDN.  Handle the full string and do not split
 * off a namespace prefix.
 * @param in Input string in native locale encoding.
 * @return IDN encoded output string.
 */
std::string
IdnTool::encodeFull (const std::string& in) const
{
  char* buf = nullptr;

  const Idna_flags flags = static_cast<Idna_flags> (0);
  Idna_rc rc;
  rc = static_cast<Idna_rc> (idna_to_ascii_lz (in.c_str (), &buf, flags));

  if (rc != IDNA_SUCCESS)
    {
      std::free (buf);
      std::ostringstream msg;
      msg << "IDNA encoding failed: " << idna_strerror (rc);
      throw std::runtime_error (msg.str ());
    }
  assert (buf);

  const std::string res(buf);
  std::free (buf);

  return res;
}

/**
 * Decode IDN string to local encoding.  According to handleNS setting,
 * possibly split off a namespace before coding.
 * @param in Input string in IDN encoding.
 * @return Decoded string in local encoding.
 */
std::string
IdnTool::decode (const std::string& in) const
{
  if (!handleNS)
    return decodeFull (in);

  std::string ns, trimmed;
  const bool hasNS = NamecoinInterface::Name::split (in, ns, trimmed);
  if (!hasNS)
    return decodeFull (in);

  trimmed = decodeFull (trimmed);
  std::ostringstream buf;
  buf << ns << '/' << trimmed;

  return buf.str ();
}

/**
 * Encode string in local encoding to IDN.  According to handleNS setting,
 * possibly split off a namespace before coding.
 * @param in Input string in local encoding.
 * @return IDN encoded string.
 */
std::string
IdnTool::encode (const std::string& in) const
{
  if (!handleNS)
    return encodeFull (in);

  std::string ns, trimmed;
  const bool hasNS = NamecoinInterface::Name::split (in, ns, trimmed);
  if (!hasNS)
    return encodeFull (in);

  trimmed = encodeFull (trimmed);
  std::ostringstream buf;
  buf << ns << '/' << trimmed;

  return buf.str ();
}

} // namespace nmcrpc
