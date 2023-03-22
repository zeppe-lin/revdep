/* See COPYING and COPYRIGHT files for corresponding information. */

#pragma once

#include <string>
#include <vector>

using namespace std;

typedef vector<string> StringVector;

void split(const string &in,
           StringVector &out,
           char         delimiter);

void ReadRdConf(const string &path,
                StringVector &dirs);

bool ReadLdConf(const string &path,
                StringVector &dirs,
                int          maxDepth);

bool IsRegularFile(const string &path);

/* vim:sw=2:ts=2:sts=2:et:cc=72:tw=70
 * End of file. */
