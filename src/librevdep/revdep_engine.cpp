/*!
 * \file revdep_engine.cpp
 * \brief Implementation of the synchronous audit engine.
 *
 * \details
 * Implements the functions declared in revdep_engine.h.  This file
 * contains no public contracts; see the header for semantics.
 *
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#include "revdep_engine.h"

#include "utility.h"

namespace revdep {

static inline void
emit(const RevdepFindingSink& sink, const RevdepFinding& f)
{
  if (sink)
    sink(f);
}

bool
RevdepAuditFile(const Package& pkg,
                const std::string& file_path,
                RevdepContext& ctx,
                const RevdepFindingSink& sink)
{
  if (!IsRegularFile(file_path))
    return true;

  const Elf* elf = ctx.Cache().LookUp(file_path);
  if (!elf)
    return true; // historical behavior: not an ELF => ignore

  bool ok = true;
  for (const auto& need : elf->Needed())
  {
    if (!ctx.Cache().FindLibrary(elf, pkg, need, ctx.Config().global_search_dirs))
    {
      RevdepFinding f;
      f.code = RevdepFinding::Code::MissingLibrary;
      f.package = pkg.Name();
      f.object_path = file_path;
      f.needed = need;
      f.message = "missing library";
      emit(sink, f);
      ok = false;
    }
  }

  return ok;
}

bool
RevdepAuditPackage(const Package& pkg,
                   RevdepContext& ctx,
                   const RevdepFindingSink& sink)
{
  if (pkg.Ignore())
    return true;

  bool ok = true;
  for (const auto& path : pkg.Files())
  {
    if (!RevdepAuditFile(pkg, path, ctx, sink))
      ok = false;
  }
  return ok;
}

} // namespace revdep
