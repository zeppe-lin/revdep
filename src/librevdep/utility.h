/*!
 * \file utility.h
 * \brief Small utility helpers used across librevdep.
 *
 * \details
 * This header declares lightweight helpers for string splitting and
 * reading simple configuration-style path lists.
 *
 * The helpers are intentionally minimal and have no global state.
 *
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#pragma once

#include <string>
#include <vector>

using namespace std;

//! Alias for readability.
typedef vector<string> StringVector;

/*!
 * \brief Split a string on a single-character delimiter.
 *
 * The delimiter is not included in the output.  Empty fields are
 * preserved (e.g. "a::b" yields {"a", "", "b"}).
 *
 * \param in    Input string to split.
 * \param out   Output vector receiving fields (appended to).
 * \param delim Delimiter character.
 */
void split(const string &in, StringVector &out, char delim);

/*!
 * \brief Read paths from revdep.d config file.
 *
 * Skips comments and empty lines.
 * Stores valid lines into 'dirs' StringVector.
 *
 * \param path  The path to the configuration file
 * \param dirs  The StringVector to store the read directories
 */
void ReadRdConf(const string &path, StringVector &dirs);

/*!
 * \brief Read paths from ld.so.conf config file.
 *
 * Handles includes, max depth.
 * Stores valid lines (and from includes) into 'dirs'.
 *
 * \param path      The path to the configuration file
 * \param dirs      The StringVector to store the read directories
 * \param maxdepth  Maximum recursion depth for include directives
 */
bool ReadLdConf(const string &path, StringVector &dirs, int maxdepth);

/*!
 * \brief Test whether \p path refers to a regular file.
 *
 * This follows lstat(2) semantics (i.e. it does not follow symlinks).
 *
 * \param path Filesystem path to test.
 * \return true if \p path exists and is a regular file, false otherwise.
 */
bool IsRegularFile(const string &path);
