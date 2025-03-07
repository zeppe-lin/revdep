//! \file  pkg.cpp
//! \brief Package class implementation.
//!
//! This file implements the `Package` class and related functions for
//! reading package information from a database and associating
//! directory configurations with packages.
//!
//! See COPYING and COPYRIGHT files for corresponding information.

#include <algorithm>  // For std::find
#include <fstream>    // For std::ifstream

#include <dirent.h>   // For DIR, opendir, readdir, closedir

#include "pkg.h"

using namespace std;

/*!
 * \brief Constructor for the Package class.
 *
 * Initializes a Package object with name, version, and file list.
 * Directory list is initialized as empty.
 *
 * \param name    Package name.
 * \param version Package version.
 * \param files   Vector of file paths belonging to the package.
 */
Package::Package(const string &name, const string &version,
    const StringVector &files)
  : _name(name),
    _version(version),
    _files(files),
    _dirs({}),      // Initialize directory list as empty
    _ignore(false)  // Initialize ignore flag as false
{}

/*!
 * \brief Reads package information from an input stream.
 *
 * Reads package name, version, and file paths from the provided input
 * stream. The format is expected to be: package name, package
 * version, followed by file paths each on a new line, and an empty
 * line to separate packages.
 *
 * \param in Input stream to read package data from.
 *
 * \return A Package object populated with data from the input stream.
 *         Returns an empty Package if fewer than 2 fields (name and
 *         version) are read.
 */
static Package
readPackage(istream &in)
{
  string       line;        // Line read from input stream
  size_t       fields = 0;  // Count of fields read
  string       name;        // Package name
  string       version;     // Package version
  StringVector files;       // Vector of file paths

  // Read until empty line or end of stream
  while (getline(in, line) && line != "")
  {
    switch (++fields)
    {
      case 1:                 name = line ; break; // Read package name
      case 2:              version = line ; break; // Read package version
      default: files.push_back("/" + line); break; // Read file path
    }
  }

  // Return empty Package if name and version not found
  if (fields < 2)
    return Package("", "", files);

  return Package(name, version, files);  // Construct and return Package
}

/*!
 * \brief Reads packages from a database file.
 *
 * Opens and reads the package database file specified by 'path'.
 * Parses each package entry using readPackage and stores the Package
 * objects in the provided PackageVector.
 *
 * \param path Path to the package database file.
 * \param pkgs [out] PackageVector to store read Package objects.
 *
 * \return True if packages are successfully read (at least one
 *         package), false if file cannot be opened or no packages are
 *         read.
 */
bool
ReadPackages(const string &path, PackageVector &pkgs)
{
  ifstream fin(path.c_str());
  if (!fin.is_open())
    return false; // Return false if packages database file cannot be opened

  // Read packages until end of file
  while (true)
  {
    Package pkg = readPackage(fin);  // Read single package

    // Break loop if empty package (end of packages)
    if (pkg.Name() == "" && pkg.Version() == "")
      break;

    pkgs.push_back(pkg);  // Add read package to vector
  }

  fin.close();  // Close package database file

  return (pkgs.size() > 0);  // Return true if packages read
}

/*!
 * \brief Reads package-specific directories from files in a
 *        directory.
 *
 * Opens the directory specified by 'path' and reads each regular file
 * within it.  For each file, it reads directory paths using
 * ReadRdConf and associates these directories with a Package object
 * whose name matches the filename.
 *
 * \param path Path to the directory containing package directory
 *             configuration files.
 * \param pkgs [in/out] PackageVector to update with package-specific
 *             directories.
 */
void
ReadPackageDirs(const string &path, PackageVector &pkgs)
{
  DIR *dir;  // Directory stream

  dir = opendir(path.c_str());  // Open directory

  if (dir == NULL)
    return;  // Return if directory cannot be opened

  struct dirent *de;  // Directory entry

  // Read directory entries
  while ((de = readdir(dir)) != NULL)
  {
    if (de->d_type != DT_REG)
      continue;  // Skip if not a regular file

    StringVector dirs;  // Vector to store directory paths

    // Read directory paths from config file
    ReadRdConf(path + "/" + de->d_name, dirs);

    if (dirs.size() == 0)
      continue;  // Skip if no directories read

    // Find package by name (filename)
    PackageVector::iterator pkg =
      find(pkgs.begin(), pkgs.end(), de->d_name);

    if (pkg == pkgs.end())
      continue;  // Skip if package not found

    pkg->Dirs(dirs);  // Set directories for the package
  }

  closedir(dir);  // Close directory stream
}

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
