//! \file  utility.h
//! \brief Helper functions definition.
//!
//! \copyright See COPYING for license terms and COPYRIGHT for notices.

#pragma once

#include <string>
#include <vector>

using namespace std;

//! Alias for readability.
typedef vector<string> StringVector;

//! \brief   Split a string by delimiter into StringVector.
//!
//! \tparam  in         The input string to split
//! \tparam  out        The StringVector to store split parts
//! \tparam  delimiter  The delimiter character
void split(const string &in, StringVector &out, char delim);

//! \brief Read paths from revdep.d config file.
//!
//! Skips comments and empty lines.
//! Stores valid lines into 'dirs' StringVector.
//!
//! \param path  The path to the configuration file
//! \param dirs  The StringVector to store the read directories
void ReadRdConf(const string &path, StringVector &dirs);

//! \brief Read paths from ld.so.conf config file.
//!
//! Handles includes, max depth.
//! Stores valid lines (and from includes) into 'dirs'.
//!
//! \param path      The path to the configuration file
//! \param dirs      The StringVector to store the read directories
//! \param maxdepth  Maximum recursion depth for include directives
bool ReadLdConf(const string &path, StringVector &dirs, int maxdepth);

//! \brief  Check if the given path points to a regular file.
//!
//! \tparam path The path to be checked
//!
//! \return \a true if \a path is a regular file, \a false otherwise
bool IsRegularFile(const string &path);

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
