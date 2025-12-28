//! \file  elf.h
//! \brief Elf class definition.
//!
//! \copyright See COPYING for license terms and COPYRIGHT for notices.

#pragma once

#include "utility.h"

/*!
 * \class Elf
 * \brief Represents an ELF file and provides access to its
 * properties.
 *
 * The `Elf` class encapsulates the data and methods for interacting
 * with ELF files.  It allows querying information like machine type,
 * dynamic dependencies (NEEDED), RPATH, and RUNPATH.
 */
class Elf {
private:

  int           _machine;      //!< Machine architecture type.
  StringVector  _needed;       //!< Vector of needed shared libraries.
  StringVector  _rpath;        //!< Vector of RPATH entries.
  StringVector  _runpath;      //!< Vector of RUNPATH entries.
  std::string   _path;         //!< Path to the ELF file.
  bool          _initialized;  //!< Flag indicating successful init.

public:
  /*!
   * \brief Constructor for the Elf class.
   *
   * Opens and parses the ELF file at the given path.
   *
   * \param path Path to the ELF file to be analyzed.
   */
  explicit Elf(const std::string &path);

  /*!
   * \brief Returns the machine architecture type of the ELF
   * file.
   *
   * \return The machine architecture type (e.g., EM_386, EM_X86_64).
   */
  int Machine() const
  {
    return _machine;
  }

  /*!
   * \brief Returns the vector of needed shared libraries.
   *
   * \return A constant reference to the StringVector containing the
   *         names of needed libraries.
   */
  const StringVector& Needed() const
  {
    return _needed;
  }

  /*!
   * \brief Returns the vector of RPATH entries.
   *
   * \return A constant reference to the StringVector containing the
   *         RPATH entries.
   */
  const StringVector& RPath() const
  {
    return _rpath;
  }

  /*!
   * \brief Returns the vector of RUNPATH entries.
   *
   * \return A constant reference to the StringVector containing the
   *         RUNPATH entries.
   */
  const StringVector& RunPath() const
  {
    return _runpath;
  }

  /*!
   * \brief Returns the path of the ELF file.
   *
   * \return A constant reference to the string containing the ELF
   *         file path.
   */
  const std::string& Path() const
  {
    return _path;
  }

  /*!
   * \brief Checks if the Elf object was successfully initialized.
   *
   * \return True if the object is valid and initialized,
   *         False otherwise.
   */
  bool Valid() const
  {
    return _initialized;
  }

  /*!
   * \brief Checks if this Elf object is compatible with another Elf
   *        object based on machine type.
   *
   * \param elf The other Elf object to compare with.
   *
   * \return True if both Elf objects have the same machine type,
   *         False otherwise.
   */
  bool Compatible(const Elf &elf) const
  {
    return _machine == elf._machine;
  }
};

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
