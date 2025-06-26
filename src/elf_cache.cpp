//! \file  elf-cache.cpp
//! \brief ElfCache class implementation.
//!
//! This file implements the `ElfCache` class, which manages a cache
//! of parsed ELF files to improve performance by avoiding redundant
//! parsing.  It also provides functionalities to resolve library
//! paths and search for libraries based on different criteria related
//! to ELF dependencies and runtime paths.
//!
//! See COPYING and COPYRIGHT files for corresponding information.

#include <algorithm>   // For std::for_each

#include <libgen.h>    // For dirname()
#include <limits.h>    // For PATH_MAX
#include <sys/auxv.h>  // for getauxval(), AT_PLATFORM

#include "elf_cache.h"

using namespace std;

// Define type alias for pair of string and Elf pointer
typedef pair          <const string &, Elf *>   ElfPair;

// Define type alias for iterator of unordered_map
typedef unordered_map <string, Elf *>::iterator ElfIter;

/*!
 * \brief Deletes an Elf object from an ElfPair.
 *
 * This is a helper function used to delete the Elf* part of an
 * ElfPair.  It is intended to be used with algorithms like
 * std::for_each to clean up a collection of ElfPairs.
 *
 * \param pair The ElfPair containing the Elf object to
 *             be deleted.
 */
static void
deleteElement(ElfPair pair)
{
  delete pair.second;  // Delete the Elf object (pointer)
}

/*!
 * \brief Resolves directory variables within a given path string.
 *
 * This function replaces special variables like $LIB, ${LIB},
 * $PLATFORM, ${PLATFORM}, $ORIGIN, and ${ORIGIN} in a given path
 * string with their corresponding values.
 *
 * - $LIB and ${LIB} are replaced with "lib".
 * - $PLATFORM and ${PLATFORM} are replaced with the platform string
 *   obtained from getauxval(AT_PLATFORM).
 * - $ORIGIN and ${ORIGIN} are replaced with the directory of the ELF
 *   file being processed.
 *
 * \param elf  Pointer to the Elf object context.
 * \param path The path string containing variables to
 *             resolve.
 *
 * \return The path string with variables resolved.
 */
static string
resolveDirVars(const Elf *elf, const string &path)
{
  static const char *lib = "lib";  // Replacement for $LIB

  // Get platform string from aux vector, default to "" if not found
  static const char *platform =
      reinterpret_cast<char *>(getauxval(AT_PLATFORM))
    ? reinterpret_cast<char *>(getauxval(AT_PLATFORM))
    : "";

  // Buffer to store directory name
  char dir[PATH_MAX];

  snprintf(dir, sizeof(dir), "%s", elf->Path().c_str());
  dirname(dir);  // Get directory name from ELF path

  // Structure to define variables and their replacements
  struct {
    const char  *name;   // Variable name (e.g., "$LIB")
    size_t      length;  // Length of variable name
    const char  *s;      // Replacement string
  } vars[] = {
    {        "$LIB",  4,      lib },
    {      "${LIB}",  6,      lib },
    {   "$PLATFORM",  9, platform },
    { "${PLATFORM}", 11, platform },
    {     "$ORIGIN",  7,      dir },
    {   "${ORIGIN}",  9,      dir },
    {          NULL,  0,     NULL }  // NULL terminator for array
  };

  size_t replaces;    // Flag to track if any replacements were made
  string out = path;  // Output path, initially input path

  do
  {
    replaces = 0;  // Reset replacements flag for each iteration

    // Iterate through variable definitions
    for (size_t i = 0; vars[i].name != NULL; ++i)
    {
      size_t j = out.find(vars[i].name);  // Find variable name

      if (j != string::npos)
      {
        out.replace(j, vars[i].length, vars[i].s);  // Replace
        ++replaces;  // Increment replacements counter
      }
    }
  } while (replaces > 0);  // Continue until no more replacements

  return out;  // Return path with variables resolved
}

/*!
 * \brief Resolves directory variables in a vector of paths.
 *
 * This function applies the resolveDirVars function to each path in
 * the input vector of paths.
 *
 * \param elf   Pointer to the Elf object context.
 * \param paths The vector of paths containing variables to resolve.
 *
 * \return A new StringVector with directory variables resolved in
 *         each path.
 */
static StringVector
resolveRunPaths(const Elf *elf, const StringVector &paths)
{
  StringVector out;  // Output vector for resolved paths

  // Resolve variables for each path in the input vector
  for (size_t i = 0; i < paths.size(); ++i)
    out.push_back(resolveDirVars(elf, paths[i]));

  return out;  // Return vector of resolved paths
}

/*!
 * \brief Searches for a library by name within specified directories.
 *
 * This function iterates through the provided directories and
 * attempts to find a library file with the given name.  For each
 * directory, it constructs the full path to the library, resolves it
 * using realpath, looks it up in the ElfCache, and checks for
 * compatibility with the context ELF object.
 *
 * \param elf  Pointer to the Elf object context.
 * \param lib  The name of the library to search for.
 * \param dirs The vector of directories to search in.
 *
 * \return True if a compatible library is found, false otherwise.
 */
bool
ElfCache::findLibraryByDirs(const Elf *elf, const string &lib,
    const StringVector &dirs)
{
  // Iterate through each directory in the provided list
  for (size_t i = 0; i < dirs.size(); ++i)
  {
    string path = dirs[i] + "/" + lib;  // Construct library path
    char realPath[PATH_MAX];            // Buffer for real path

    // Resolve path to canonical real path, skip if fails
    if (realpath(path.c_str(), realPath) == NULL)
      continue;

    const Elf *elf2 = LookUp(realPath);  // Lookup ELF in cache

    if (elf2 == NULL)
      continue;  // Skip if ELF lookup failed

    if (!elf->Compatible(elf2[0]))
      continue;  // Skip if ELFs are not compatible

    return true;  // Compatible library found
  }

  return false;  // Library not found in any of the dirs
}

/*!
 * \brief Searches for a library by its full path or relative path to
 *        the ELF's directory.
 *
 * If the library path starts with '/', it's treated as an absolute
 * path.  Otherwise, it's considered relative to the directory of the
 * context ELF file.  The function resolves the path using realpath,
 * looks up the ELF in the cache, and checks for compatibility.
 *
 * \param elf Pointer to the Elf object context.
 * \param lib The path to the library to search for.
 *
 * \return True if a compatible library is found at the path, false
 *         otherwise.
 */
bool
ElfCache::findLibraryByPath(const Elf *elf, const string &lib)
{
  string path;  // Constructed path to library

  if (lib[0] == '/')
  {
    path = lib;  // Library path is absolute
  }
  else
  {
    char dir[PATH_MAX];  // Buffer for directory of ELF

    snprintf(dir, sizeof(dir), "%s", elf->Path().c_str());
    dirname(dir);  // Get directory of the ELF file

    path = dir + ("/" + lib);  // Construct relative path
  }

  char realPath[PATH_MAX];  // Buffer for real path

  if (realpath(path.c_str(), realPath) == NULL)
    return false;  // Resolve path to canonical real path

  const Elf *elf2 = LookUp(realPath);  // Lookup ELF in cache

  if (elf2 == NULL)
    return false;  // Skip if ELF lookup failed

  return elf->Compatible(elf2[0]);  // Check for compatibility
}

/*!
 * \brief Destructor for the ElfCache class.
 *
 * Iterates through the cached Elf objects and deletes each one to
 * free allocated memory.
 */
ElfCache::~ElfCache()
{
  // Iterate through the cache and delete each Elf object
  for_each(_data.begin(), _data.end(), deleteElement);
}

/*!
 * \brief Looks up an Elf object in the cache by path.
 *
 * If an Elf object for the given path already exists in the cache, it
 * returns the cached object.  Otherwise, it creates a new Elf object
 * by parsing the file at the given path, adds it to the cache, and
 * returns the new object.
 *
 * \param path The path to the ELF file to lookup or create.
 *
 * \return Pointer to the Elf object from the cache or a newly created
 *         one, or NULL if ELF creation fails.
 */
const Elf*
ElfCache::LookUp(const string &path)
{
  ElfIter value = _data.find(path);  // Find path in cache

  if (value != _data.end())
    return value->second;  // Return cached Elf object if found

  Elf *elf = new Elf(path);  // Create a new Elf object

  if (!elf->Valid())
  {
    delete elf;   // Delete newly created Elf object if invalid
    return NULL;  // Return NULL if ELF creation failed
  }

  ElfPair pair = make_pair <const string &, Elf *&> (path, elf);

  _data.insert(pair);  // Insert new Elf object into cache

  return elf;  // Return the newly created (and cached) Elf
}

/*!
 * \brief Finds a library for a given ELF object, package, library
 *        name, and search directories.
 *
 * This is the main function for finding a library.  It checks for the
 * library in the following order:
 *
 * 1. If the library name contains '/', try to find it by absolute or
 *    relative path using findLibraryByPath.
 * 2. Resolve RUNPATH entries of the ELF and search in those
 *    directories using findLibraryByDirs.
 * 3. If RUNPATH is not present or library not found, resolve RPATH
 *    entries and search in those directories.
 * 4. If RPATH is also not present or library not found, search in the
 *    provided 'dirs' using findLibraryByDirs.
 * 5. Finally, if still not found, search in the package directories
 *   (pkg.Dirs()) using findLibraryByDirs.
 *
 * \param elf  Pointer to the Elf object context.
 * \param pkg  The Package object (used for package-specific
 *             directory search).
 * \param lib  The name of the library to find.
 * \param dirs Additional directories to search in.
 *
 * \return True if the library is found, false otherwise.
 */
bool
ElfCache::FindLibrary(const Elf *elf, const Package &pkg,
    const string &lib, const StringVector &dirs)
{
  // If library path is absolute or relative, use path search
  if (lib.find('/') != string::npos)
    return findLibraryByPath(elf, lib);

  StringVector paths;  // Vector to store search paths

  // Search in RUNPATH directories if available
  if (elf->RunPath().size() > 0)
  {
    paths = resolveRunPaths(elf, elf->RunPath());
    if (findLibraryByDirs(elf, lib, paths))
      return true;  // Library found in RUNPATH
  }
  // Else, search in RPATH directories if available
  else if (elf->RPath().size() > 0)
  {
    paths = resolveRunPaths(elf, elf->RPath());
    if (findLibraryByDirs(elf, lib, paths))
      return true;  // Library found in RPATH
  }

  // Search in provided 'dirs' directories
  if (findLibraryByDirs(elf, lib, dirs))
    return true;    // Library found in 'dirs'

  // Search in package directories if available
  if (   pkg.Dirs().size() > 0
      && findLibraryByDirs(elf, lib, pkg.Dirs()))
    return true;    // Library found in package dirs

  return false;     // Library not found in any searched paths
}

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
