//! \file  pkg.h
//! \brief Package class definition.
//!
//! This file defines the `Package` class and related type definitions
//! and function declarations for managing package information.
//!
//! See COPYING and COPYRIGHT files for corresponding information.

#pragma once

#include "utility.h"

using namespace std;

/*!
 * \class Package
 * \brief Represents a software package with its properties and
 *        associated files and directories.
 *
 * The `Package` class stores information about a software package,
 * including its name, version, files, and associated directories.  It
 * provides methods to access these properties and to check package
 * compatibility.
 */
class Package {
private:
  string        _name;     //!< Package name.
  string        _version;  //!< Package version string.
  StringVector  _files;    //!< Vector of file paths in package.
  StringVector  _dirs;     //!< Vector of directories for package.
  bool          _ignore;   //!< Flag to indicate if package is ignored.

public:
  /*!
   * \brief Constructor for the Package class.
   *
   * Initializes a Package object with name, version, and file list.
   *
   * \param name    Package name.
   * \param version Package version.
   * \param files   Vector of file paths for the package.
   */
  Package(const string &name, const string &version,
      const StringVector &files);

  /*!
   * \brief Returns the name of the package.
   *
   * \return Package name as a constant string reference.
   */
  const string& Name() const
  {
    return _name;
  }

  /*!
   * \brief Returns the version of the package.
   *
   * \return Package version as a constant string reference.
   */
  const string& Version() const
  {
    return _version;
  }

  /*!
   * \brief Returns the list of files in the package.
   *
   * \return Vector of file paths as a constant StringVector
   *         reference.
   */
  const StringVector& Files() const
  {
    return _files;
  }

  /*!
   * \brief Returns the list of directories associated with the
   *        package.
   *
   * \return Vector of directory paths as a constant StringVector
   *         reference.
   */
  const StringVector& Dirs() const
  {
    return _dirs;
  }

  /*!
   * \brief Sets the list of directories for the package.
   *
   * \param dirs Vector of directory paths to set for the package.
   */
  void Dirs(const StringVector &dirs)
  {
    _dirs = dirs;
  }

  /*!
   * \brief Checks if the package is marked as ignored.
   *
   * \return True if the package is ignored, false otherwise.
   */
  bool Ignore() const
  {
    return _ignore;
  }

  /*!
   * \brief Marks the package as ignored.
   *
   * Sets the ignore flag to true for this package.
   */
  void Ignore()
  {
    _ignore = true;
  }

  /*!
   * \brief Equality operator overload for Package and string.
   *
   * Compares the package name with a given string.
   *
   * \param name String to compare with the package name.
   *
   * \return True if the package name is equal to the given string,
   *         false otherwise.
   */
  bool operator ==(const string &name) const
  {
    return _name == name;
  }
}; /* class Package */

/*!
 * \brief Type alias for a vector of Package objects.
 *
 * Defines `PackageVector` as a synonym for `std::vector<Package>`.
 */
typedef vector <Package> PackageVector;

/*!
 * \brief Reads packages from a database file.
 *
 * Parses package information from the file specified by 'path' and
 * populates a PackageVector.
 *
 * \param path Path to the package database file.
 * \param pkgs [out] PackageVector to store the read Package objects.
 *
 * \return True if packages are read successfully, false otherwise.
 */
bool ReadPackages(const string &path, PackageVector &pkgs);

/*!
 * \brief Reads package-specific directories from a directory.
 *
 * Reads directory configurations from files in the directory
 * specified by 'path' and associates them with corresponding packages
 * in the provided PackageVector.
 *
 * \param path Path to the directory containing package directory
 *             configuration files.
 * \param pkgs [in/out] PackageVector to update with package
 *             directories.
 */
void ReadPackageDirs(const string &path, PackageVector &pkgs);

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
