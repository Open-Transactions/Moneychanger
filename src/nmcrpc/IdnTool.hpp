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

#ifndef NMCRPC_IDNTOOL_HPP
#define NMCRPC_IDNTOOL_HPP

#include <cstddef>
#include <string>

namespace nmcrpc
{

/**
 * Class with basic IDN encoding and decoding capabilities.
 */
class IdnTool
{

private:
  
  /** Whether or not to split off namespaces.  */
  bool handleNS;

  /**
   * Decode IDN to native locale string.  This routine handles the full string
   * and does not split off a namespace prefix.
   * @param in Input string, encoded as IDN.
   * @return Decoded output string.
   */
  std::string decodeFull (const std::string& in) const;

  /**
   * Encode a native string to IDN.  Handle the full string and do not split
   * off a namespace prefix.
   * @param in Input string in native locale encoding.
   * @return IDN encoded output string.
   */
  std::string encodeFull (const std::string& in) const;

public:

  /**
   * Default constructor.  Allow to specify whether or not to handle
   * namespaces, default is true.
   * Also call setlocale() to properly set up the locale settings
   * from the environment.
   * @param ns Handle namespaces for codings?
   */
  IdnTool (bool ns = true);

  // Allow copying.
#ifdef CXX_11
  IdnTool (const IdnTool&) = default;
  IdnTool& operator= (const IdnTool&) = default;
#endif /* CXX_11?  */

  /**
   * Set namespace setting.  If set to true, decode() and encode() will
   * split off a namespace prefix if one is present before performing
   * the coding.
   * @param ns New setting.
   */
  inline void handleNamespace (bool ns)
  {
    handleNS = ns;
  }

  /**
   * Decode IDN string to local encoding.  According to handleNS setting,
   * possibly split off a namespace before coding.
   * @param in Input string in IDN encoding.
   * @return Decoded string in local encoding.
   */
  std::string decode (const std::string& in) const;

  /**
   * Encode string in local encoding to IDN.  According to handleNS setting,
   * possibly split off a namespace before coding.
   * @param in Input string in local encoding.
   * @return IDN encoded string.
   */
  std::string encode (const std::string& in) const;

};

} // namespace nmcrpc

#endif /* Header guard.  */
