/*!
 * \file revdep_context.h
 * \brief Configuration and shared context for revdep auditing.
 *
 * \details
 * Defines RevdepConfig and RevdepContext, which bundle
 * caller-provided configuration (e.g., global search directories)
 * with shared analysis state (notably the thread-safe ElfCache).
 *
 * The library is designed for embedding: all mutable state is
 * explicit and caller-owned; no hidden process-global state is
 * required.
 *
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "elf_cache.h"

namespace revdep {

/*!
 * \brief Configuration for dependency resolution.
 *
 * This models loader-visible global resolver directories.
 */
struct RevdepConfig {
  std::vector<std::string> global_search_dirs;
};

/*!
 * \brief Explicit analysis context (configuration + shared state).
 *
 * Threading: safe to share across threads if configuration is treated
 * as immutable during a run.  The embedded ELF cache is thread-safe.
 */
class RevdepContext {
public:
  explicit RevdepContext(RevdepConfig cfg, std::shared_ptr<ElfCache> cache = {})
    : _cfg(std::move(cfg)), _cache(std::move(cache))
  {
    if (!_cache)
      _cache = std::make_shared<ElfCache>();
  }

  const RevdepConfig& Config() const { return _cfg; }
  ElfCache& Cache() { return *_cache; }
  const ElfCache& Cache() const { return *_cache; }
  std::shared_ptr<ElfCache> CachePtr() const { return _cache; }

private:
  RevdepConfig _cfg;
  std::shared_ptr<ElfCache> _cache;
};

} // namespace revdep
