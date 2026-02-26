/*!
 * \file revdep_parallel.h
 * \brief Optional parallel runner helpers for librevdep.
 *
 * The parallel runner schedules many audits concurrently.  It does
 * not change dependency resolution semantics; it changes only
 * scheduling and emission.
 *
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
  Unordered,
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
