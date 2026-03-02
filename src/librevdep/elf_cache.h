/*!
 * \file elf_cache.h
 * \brief Thread-safe cache of parsed ELF objects and library
 *        resolution helpers.
 *
 * \details
 * Defines ElfCache, which memoizes parsed Elf instances keyed by path
 * and provides loader-like lookup of DT_NEEDED libraries using
 * RUNPATH, RPATH, global search directories, and package-local
 * directories.
 *
 * Threading:
 *   - Lookups are safe to run concurrently.
 *   - Cached Elf objects are shared and treated as immutable.
 *
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>  // For std::unordered_map

#include "elf.h"          // Includes Elf class definition
#include "pkg.h"          // Include Package class definition

using namespace std;

/*!
 * \class ElfCache
 * \brief Cache of parsed ELF objects and helper routines for library
 *        lookup.
 *
 * ElfCache memoizes parsed ELF metadata to avoid redundant parsing.
 * Cached Elf objects are treated as immutable and are shared by
 * pointer.
 *
 * Threading:
 * - LookUp() is thread-safe and may be used concurrently.
 * - The cache uses a read/write lock to allow parallel lookups.
 *
 * Resolution:
 * - FindLibrary() searches in RUNPATH, RPATH, global directories,
 *   then package-local directories, applying loader-style token
 *   expansion for directory elements (e.g. $ORIGIN).
 */
class ElfCache {
private:
  using ElfPtr = std::shared_ptr<const Elf>;
  using ElfMap = std::unordered_map<std::string, ElfPtr>;

  mutable std::shared_mutex _mx;
  ElfMap _data;  //!< Unordered map to cache Elf objects,
                 //!< keyed by file path.

  /*!
   * \brief Searches for a library in specified directories.
   *
   * This is a private helper function to find a library by name
   * within a given set of directories.
   *
   * \param elf  Pointer to the context Elf object.
   * \param lib  The name of the library to search for.
   * \param dirs The vector of directories to search in.
   *
   * \return True if a compatible library is found, False otherwise.
   */
  bool findLibraryByDirs(const Elf *elf, const string &lib,
                         const StringVector &dirs);

  /*!
   * \brief Searches for a library by its full or relative path.
   *
   * This private helper function searches for a library using a
   * provided path, which can be absolute or relative to the ELF's
   * directory.
   *
   * \param elf Pointer to the context Elf object.
   * \param lib The path to the library to search for.
   *
   * \return True if a compatible library is found at the path,
   *         False otherwise.
   */
  bool findLibraryByPath(const Elf *elf, const string &lib);

public:
  ElfCache() = default;
  ~ElfCache() = default;

  /*!
   * \brief Looks up an Elf object in the cache or creates a new one
   *        if not found.
   *
   * This method retrieves an Elf object from the cache if it exists
   * for the given path.  If not, it creates a new Elf object by
   * parsing the file and adds it to the cache before returning it.
   *
   * \param path The path to the ELF file to lookup or create.
   *
   * \return A pointer to the Elf object (cached or newly created), or
   *         nullptr if creation fails.
   */
  const Elf *LookUp(const string &path);

  /*!
   * \brief Finds a library based on various search paths and
   *        criteria.
   *
   * This method orchestrates the library search process, considering
   * RUNPATH, RPATH, provided directories, and package-specific
   * directories to locate a compatible library for a given ELF object
   * and package.
   *
   * \param elf  Pointer to the context Elf object.
   * \param pkg  Reference to the Package object providing
   *             package-specific search paths.
   * \param lib  The name of the library to find.
   * \param dirs Vector of additional directories to search in.
   *
   * \return True if the library is found, false otherwise.
   */
  bool FindLibrary(const Elf *elf, const Package &pkg,
                   const string &lib, const StringVector &dirs);

}; // class ElfCache
