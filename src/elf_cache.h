//! \file  elf-cache.h
//! \brief ElfCache class definition.
//!
//! This file defines the `ElfCache` class, which is responsible for
//! managing a cache of parsed ELF files and providing functionalities
//! to search for libraries based on various criteria, including
//! runtime paths (RUNPATH and RPATH) and package-specific directories.
//!
//! \copyright See COPYING and COPYRIGHT files for corresponding information.

#pragma once

#include <unordered_map>  // For std::unordered_map

#include "elf.h"          // Includes Elf class definition
#include "pkg.h"          // Include Package class definition

using namespace std;

// Define type alias for the Elf cache map
typedef unordered_map <string, Elf *> ElfMap;

/*!
 * \class ElfCache
 * \brief Manages a cache of Elf objects and provides library
 *        searching functionalities.
 *
 * The `ElfCache` class is responsible for caching parsed `Elf`
 * objects to avoid redundant parsing of the same ELF files.  It also
 * provides methods to search for shared libraries based on different
 * paths and rules, considering ELF properties like RPATH and RUNPATH.
 */
class ElfCache {
private:

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
   * \return True if a compatible library is found, false otherwise.
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
   * \return True if a compatible library is found at the path, false
   *         otherwise.
   */
  bool findLibraryByPath(const Elf *elf, const string &lib);

public:

  /*!
   * \brief Destructor for the ElfCache class.
   *
   * Frees the memory allocated for cached Elf objects.
   */
  ~ElfCache();

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
   *         NULL if creation fails.
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

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
