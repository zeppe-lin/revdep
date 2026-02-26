/*!
 * \file revdep_engine.h
 * \brief Synchronous audit entry points for librevdep.
 *
 * The engine analyzes ELF objects and reports missing shared-library
 * dependencies (DT_NEEDED entries that cannot be resolved) using the
 * resolution semantics documented in revdep_semantics(7).
 *
 * The engine never prints.  Results are returned as structured
 * findings or streamed to a sink callback.
 *
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include <functional>
#include <string>

#include "pkg.h"
#include "revdep_context.h"

namespace revdep {

/*!
 * \brief A single diagnostic produced by the audit engine.
 */
struct RevdepFinding {
  enum class Code {
    MissingLibrary,
  };

  Code code {Code::MissingLibrary};
  std::string package;     // package name
  std::string object_path; // ELF being checked
  std::string needed;      // DT_NEEDED entry
  std::string message;     // human short message
};

/*!
 * \brief Streaming sink for findings (called on the current thread).
 */
using RevdepFindingSink = std::function<void(const RevdepFinding&)>;

/*!
 * \brief Audit a single filesystem object.
 * \return true if no missing dependencies were detected for this
 *         object.
 */
bool RevdepAuditFile(const Package& pkg,
                     const std::string& file_path,
                     RevdepContext& ctx,
                     const RevdepFindingSink& sink);

/*!
 * \brief Audit all relevant objects belonging to a package.
 * \return true if no missing dependencies were detected for this package.
 *
 * \warning
 * Skips ignored packages.
 */
bool RevdepAuditPackage(const Package& pkg,
                        RevdepContext& ctx,
                        const RevdepFindingSink& sink);

} // namespace revdep
