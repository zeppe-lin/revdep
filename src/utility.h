//! \file  utility.h
//! \brief Helper functions definition.
//!        See COPYING and COPYRIGHT files for corresponding information.

#pragma once

#include <string>
#include <vector>

using namespace std;

//! TODO document
typedef vector<string> StringVector;

//! \brief   Split a string into parts
//!
//! \tparam  in         string to split
//! \tparam  out        vector of strings that will contain the result
//! \tparam  delimiter  delimiter
void
split(const string &in, StringVector &out, char delimiter);

//! TODO document
void
ReadRdConf(const string &path, StringVector &dirs);

//! TODO document
bool
ReadLdConf(const string &path, StringVector &dirs, int maxDepth);

//! \brief   Test for a regular file
//!
//! \tparam  path       file path
//!
//! \return  \a true if \a path is a regular file, \a false otherwise
bool
IsRegularFile(const string &path);

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
