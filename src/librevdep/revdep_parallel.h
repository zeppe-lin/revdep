/*!
 * \file revdep_parallel.h
 * \brief Optional parallel scheduling helpers for revdep auditing.
 *
 * \details
 * Declares a parallel runner that schedules many audits concurrently.
 * It does not change resolution semantics; it only changes scheduling
 * and (optionally) the emission ordering of findings.
 *
 * \important
 * Deterministic output is supported via an explicit emission order
 * policy.
 *
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "revdep_engine.h"

namespace revdep {

/*!
 * \brief One audit unit for the parallel runner.
 */
struct RevdepWorkItem {
  const Package* pkg;
  std::string object_path;
};

/*!
 * \brief Finding emission order under parallel execution.
 */
enum class RevdepEmitOrder {
  //! Emit findings as soon as they are produced (fastest,
  //! nondeterministic).
  Unordered,

  //! Buffer per work item and emit in the same order as the input
  //! list.
  InputStable,
};

/*!
 * \brief Parallel runner configuration.
 */
struct RevdepParallelOptions {
  std::size_t thread_count {0};
  RevdepEmitOrder order {RevdepEmitOrder::Unordered};
  bool serialize_sink {true};
};

/*!
 * \brief Audit many work items, optionally in parallel.
 * \return true if no missing dependencies were detected across all
 *         items.
 */
bool RevdepAuditWorkItemsParallel(const std::vector<RevdepWorkItem>& items,
                                  RevdepContext& ctx,
                                  const RevdepParallelOptions& opt,
                                  const RevdepFindingSink& sink);

} // namespace revdep
