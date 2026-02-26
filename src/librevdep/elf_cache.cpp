/*!
 * \file elf_cache.cpp
 * \brief Implementation of ElfCache and resolution helpers.
 *
 * \details
 * Implements the caching and resolution logic declared in
 * elf_cache.h.  Public semantics live in the header; this file
 * provides the concrete token expansion and directory search
 * mechanics.
 *
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#include <mutex>
#include <utility>

#include <sys/auxv.h>  // For getauxval(), AT_PLATFORM

#include "elf_cache.h"
#include "utility.h"

/*!
 * \brief Return parent directory of \p p (like dirname(3), but
 *        without mutation).
 */
static std::string
dir_name(const std::string& p)
{
  auto pos = p.rfind('/');
  if (pos == std::string::npos)
    return ".";
  if (pos == 0)
    return "/";
  return p.substr(0, pos);
}

/*!
 * \brief Expand ld.so-style tokens inside a single directory element.
 *
 * This expands tokens used in DT_RPATH / DT_RUNPATH entries.
 * The element is assumed to be already split on ':' by the ELF parser.
 *
 * Supported tokens (kept aligned with the historical revdep behavior):
 * - $ORIGIN, ${ORIGIN}     -> directory containing \p elf (elf->Path()).
 * - $LIB, ${LIB}           -> "lib".
 * - $PLATFORM, ${PLATFORM} -> AT_PLATFORM from the aux vector (or "").
 *
 * Notes:
 * - Expansion is performed until a fixed point (multiple occurrences).
 * - The dynamic loader treats an empty element as the current
 *   directory; we normalize empty to "." for consistent path joining.
 */
static std::string
resolve_dir_vars(const Elf *elf, const std::string &in)
{
  static const char *kLib = "lib";

  // Get platform string from aux vector, default to "" if not found
  const char *plat = reinterpret_cast<const char *>(getauxval(AT_PLATFORM));
  if (!plat)
    plat = "";

  // Get directory name from ELF path
  const std::string origin = dir_name(elf->Path());

  // Structure to define variables and their replacements
  struct Var {
    const char *name;    // Variable name (e.g., "$LIB")
    const char *value;   // Replacement string
  } vars[] = {
    {"$LIB",        kLib},
    {"${LIB}",      kLib},
    {"$PLATFORM",   plat},
    {"${PLATFORM}", plat},
    {"$ORIGIN",     origin.c_str()},
    {"${ORIGIN}",   origin.c_str()},
    {nullptr,       nullptr},
  };

  std::string out = in;

  // Replace until fixed point (handles multiple occurrences).
  bool changed;
  do
  {
    changed = false;
    for (size_t i = 0; vars[i].name; ++i)
    {
      const std::string needle(vars[i].name);
      const std::string repl(vars[i].value ? vars[i].value : "");
      size_t pos = out.find(needle);
      if (pos != std::string::npos)
      {
        out.replace(pos, needle.size(), repl);
        changed = true;
      }
    }
  } while (changed);

  // Loader treats empty elements as current directory.
  if (out.empty())
    out = ".";

  return out;
}

/*!
 * \brief Lookup and cache a parsed ELF file by path.
 *
 * This function is thread-safe and may be called concurrently.
 * It uses a read/write lock to allow parallel lookups and caches
 * immutable ELF objects (shared ownership).
 *
 * Parsing is performed outside the lock to avoid blocking other
 * readers.
 *
 * \return Pointer to cached ELF on success, or nullptr if the file
 *         cannot be parsed as a valid ELF object.
 */
const Elf *
ElfCache::LookUp(const string &path)
{
  {
    std::shared_lock<std::shared_mutex> lk(_mx);
    auto it = _data.find(path);
    if (it != _data.end())
      return it->second.get();
  }

  // Parse outside lock.
  std::shared_ptr<const Elf> parsed;
  try
  {
    parsed = std::make_shared<Elf>(path);
  }
  catch (...)
  {
    return nullptr;
  }

  if (!parsed || !parsed->Valid())
    return nullptr;

  std::unique_lock<std::shared_mutex> lk(_mx);
  auto [it, inserted] = _data.emplace(path, parsed);
  if (!inserted)
    return it->second.get();
  return parsed.get();
}

/*!
 * \brief Search for \p lib by iterating \p dirs and testing
 *        compatibility.
 *
 * Each directory element is expanded via resolve_dir_vars()
 * (supporting $ORIGIN/$LIB/$PLATFORM) and then joined with \p lib.
 *
 * A candidate is accepted only if it exists, parses as ELF, and is
 * ABI-compatible with \p elf (via Elf::Compatible()).
 *
 * \return true if a compatible library was found, false otherwise.
 */
bool
ElfCache::findLibraryByDirs(const Elf *elf, const string &lib,
                            const StringVector &dirs)
{
  for (const auto &dir : dirs)
  {
    const std::string resolved = resolve_dir_vars(elf, dir);
    string path = resolved;
    if (!path.empty() && path.back() != '/')
      path += '/';
    path += lib;

    const Elf *elf2 = LookUp(path);
    if (!elf2)
      continue;

    if (elf->Compatible(*elf2))
      return true;
  }

  return false;
}

/*!
 * \brief Resolve a DT_NEEDED-like entry that names a path
 *        (contains '/').
 *
 * If \p lib is absolute, it is used as-is. Otherwise it is treated as
 * relative to the audited object's directory (dir_name(elf->Path())).
 *
 * \return true if the target exists, parses as ELF, and is
 *         compatible.
 */
bool
ElfCache::findLibraryByPath(const Elf *elf, const string &lib)
{
  string path;

  if (!lib.empty() && lib[0] == '/')
    path = lib;
  else
    path = dir_name(elf->Path()) + "/" + lib;

  const Elf *elf2 = LookUp(path);
  if (!elf2)
    return false;

  return elf->Compatible(*elf2);
}

/*!
 * \brief Resolve \p lib for \p elf using loader-like search rules.
 *
 * Search order:
 *  1. If \p lib contains '/', treat it as a path (absolute or
 *     relative to the object's directory) via findLibraryByPath().
 *  2. Search DT_RUNPATH directories (expanded tokens).
 *  3. Search DT_RPATH directories (expanded tokens).
 *  4. Search global resolver directories passed as \p dirs.
 *  5. Search package-local directories (pkg.Dirs()).
 *
 * This function does not consult ld.so.cache; \p dirs is expected to
 * model global loader-visible directories (e.g., from ld.so.conf and
 * revdep.d).
 */
bool
ElfCache::FindLibrary(const Elf *elf, const Package &pkg,
                      const string &lib, const StringVector &dirs)
{
  if (lib.find('/') != string::npos)
  {
    if (findLibraryByPath(elf, lib))
      return true;
  }

  if (findLibraryByDirs(elf, lib, elf->RunPath())) return true;
  if (findLibraryByDirs(elf, lib, elf->RPath()))   return true;
  if (findLibraryByDirs(elf, lib, dirs))           return true;
  if (findLibraryByDirs(elf, lib, pkg.Dirs()))     return true;

  return false;     // Library not found in any searched paths
}
