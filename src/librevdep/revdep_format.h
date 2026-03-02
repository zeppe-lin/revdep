/*!
 * \file revdep_format.h
 * \brief Formatting helpers for revdep findings.
 *
 * \details
 * Declares helpers that convert RevdepFinding into stable textual
 * forms suitable for CLI output and logs.
 * The formatter performs no I/O.
 *
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include <string>

#include "revdep_engine.h"

namespace revdep {

/*!
 * \brief Format a finding as a single revdep-like line.
 */
std::string RevdepFormatFinding(const RevdepFinding& f);

} // namespace revdep
