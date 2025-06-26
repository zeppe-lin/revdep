//! \file  utility.cpp
//! \brief Helper functions implementation.
//!
//! See COPYING and COPYRIGHT files for corresponding information.

#include <fstream>

#include <glob.h>
#include <sys/stat.h>

#include "utility.h"

using namespace std;

void
split(const string &in, StringVector &out, char delim)
{
  size_t start = 0;
  size_t end = in.find(delim);

  while (end != string::npos)
  {
    out.push_back(in.substr(start, end - start));
    start = ++end;
    end = in.find(delim, end);
  }

  out.push_back(in.substr(start));
}

void
ReadRdConf(const string &path, StringVector &dirs)
{
  ifstream fin(path.c_str());
  if (!fin.is_open())
    return;

  string line;

  while (getline(fin, line))
  {
    if (line[0] != '#' && line.length() > 0)
      dirs.push_back(line);
  }

  fin.close();
}

bool
ReadLdConf(const string &path, StringVector &dirs, int maxdepth)
{
  if (maxdepth <= 0) // Check recursion depth limit
    return false;    // Depth limit exceeded, return fail

  // Open config file
  ifstream fin(path.c_str());
  if (!fin.is_open())
    return false;

  // Read config file line by line
  string line;
  while (getline(fin, line))
  {

    // Skip comment lines
    if (line[0] == '#')
      continue;

    // Handle "include "
    if (line.compare(0, 8, "include ") == 0)
    {
      glob_t g; // Glob struct for file expansion
      string includePath = line.substr(8); // Path after "include "
                                           //
      if (glob(includePath.c_str(), 0, NULL, &g) == 0) // Expand path
      {
        // Iterate paths
        for (size_t i = 0; i < g.gl_pathc; ++i)
        {
          // Recursive call for included file, depth - 1
          if (!ReadLdConf(g.gl_pathv[i], dirs, maxdepth - 1))
          {
            globfree(&g);  // Free glob memory
            fin.close();   // Close current file
            return false;  // Propagate failure upwards
          }
        }
      }

      globfree(&g);  // Free glob memory after use
    }
    // If not include and not empty - add valid line to dirs
    else if (!line.empty())
    {
      dirs.push_back(line);
    }
  }

  fin.close(); // Close config file
  return true; // Successfully processed
}

bool
IsRegularFile(const string &file_path)
{
  struct stat file_stat;

  if (lstat(file_path.c_str(), &file_stat) == -1)
    return false; // lstat failed, path likely invalid or doesn't
                  // exist

  return S_ISREG(file_stat.st_mode); // Check if a regular file
}

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
