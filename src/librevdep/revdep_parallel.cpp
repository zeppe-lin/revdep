/*!
 * \file revdep_parallel.cpp
 * \brief Implementation of parallel scheduling helpers.
 *
 * \details
 * Implements the parallel runner declared in revdep_parallel.h.
 *
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#include "revdep_parallel.h"

#include <atomic>
#include <mutex>
#include <thread>

namespace revdep {

static std::size_t
normalize_threads(std::size_t n)
{
  if (n > 0)
    return n;
  unsigned hc = std::thread::hardware_concurrency();
  return hc ? static_cast<std::size_t>(hc) : 1u;
}

bool
RevdepAuditWorkItemsParallel(const std::vector<RevdepWorkItem>& items,
                             RevdepContext& ctx,
                             const RevdepParallelOptions& opt,
                             const RevdepFindingSink& sink)
{
  if (items.empty())
    return true;

  const std::size_t threads = normalize_threads(opt.thread_count);

  std::mutex sink_mx;
  auto safe_emit = [&](const RevdepFinding& f) {
    if (!sink)
      return;

    if (opt.serialize_sink)
    {
      std::lock_guard<std::mutex> lk(sink_mx);
      sink(f);
    }
    else
    {
      sink(f);
    }
  };

  std::atomic<std::size_t> idx{0};
  std::atomic<bool> ok{true};

  if (opt.order == RevdepEmitOrder::InputStable)
  {
    std::vector<std::vector<RevdepFinding>> buckets(items.size());
    std::vector<std::thread> pool;
    pool.reserve(threads);

    for (std::size_t t = 0; t < threads; ++t)
    {
      pool.emplace_back([&] {
        for (;;)
        {
          std::size_t i = idx.fetch_add(1, std::memory_order_relaxed);
          if (i >= items.size())
            break;
          const auto& it = items[i];
          if (!it.pkg)
            continue;

          auto local_sink = [&](const RevdepFinding& f) {
            buckets[i].push_back(f);
          };
          if (!RevdepAuditFile(*it.pkg, it.object_path, ctx, local_sink))
            ok.store(false, std::memory_order_relaxed);
        }
      });
    }

    for (auto& th : pool)
      th.join();

    for (const auto& b : buckets)
    {
      for (const auto& f : b)
        safe_emit(f);
    }

    return ok.load(std::memory_order_relaxed);
  }

  std::vector<std::thread> pool;
  pool.reserve(threads);

  for (std::size_t t = 0; t < threads; ++t)
  {
    pool.emplace_back([&] {
      for (;;)
      {
        std::size_t i = idx.fetch_add(1, std::memory_order_relaxed);
        if (i >= items.size())
          break;
        const auto& it = items[i];
        if (!it.pkg)
          continue;

        auto local_sink = [&](const RevdepFinding& f) {
          safe_emit(f);
        };
        if (!RevdepAuditFile(*it.pkg, it.object_path, ctx, local_sink))
          ok.store(false, std::memory_order_relaxed);
      }
    });
  }

  for (auto& th : pool)
    th.join();

  return ok.load(std::memory_order_relaxed);
}

} // namespace revdep
