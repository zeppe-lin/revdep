/*!
 * \file revdep_format.cpp
 * \brief Implementation of finding formatting helpers.
 *
 * \details
 * Implements the formatter declared in revdep_format.h
 *
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#include "revdep_format.h"

#include <sstream>

namespace revdep {

std::string
RevdepFormatFinding(const RevdepFinding& f)
{
  std::ostringstream os;

  if (!f.package.empty())
    os << f.package << ":";

  os << f.object_path;

  if (!f.needed.empty())
    os << ":" << f.needed;

  if (!f.message.empty())
    os << ": " << f.message;

  return os.str();
}

} // namespace revdep
