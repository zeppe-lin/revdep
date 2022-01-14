// Copyright (C) 2016 James Buren
//
// This file is part of revdep.
//
// revdep is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// revdep is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with revdep.  If not, see <http://www.gnu.org/licenses/>.

#include <fstream>

#include <glob.h>
#include <sys/stat.h>

#include "utility.h"

using namespace std;

void split(const string &in,
           StringVector &out,
           char         delimiter)
{
  size_t i = 0;
  size_t j = in.find(delimiter);

  while (j != string::npos)
  {
    out.push_back(in.substr(i, j - i));
    i = ++j;
    j = in.find(delimiter, j);
  }

  out.push_back(in.substr(i));
}

void ReadRdConf(const string &path,
                StringVector &dirs)
{
  ifstream fin;

  fin.open(path.c_str());

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

bool ReadLdConf(const string &path,
                StringVector &dirs,
                int          maxDepth)
{
  if (maxDepth <= 0)
    return false;

  ifstream fin;

  fin.open(path.c_str());

  if (!fin.is_open())
    return false;

  string line;

  while (getline(fin, line))
  {
    if (line[0] == '#')
      continue;

    if (line.compare(0, 8, "include ") == 0)
    {
      glob_t g;

      if (glob(line.substr(8).c_str(), 0, NULL, &g) == 0)
      {
        for (size_t i = 0; i < g.gl_pathc; ++i)
        {
          if (!ReadLdConf(g.gl_pathv[i], dirs, maxDepth - 1))
          {
            globfree(&g);
            fin.close();
            return false;
          }
        }
      }

      globfree(&g);
    }
    else if (line.length() > 0)
    {
      dirs.push_back(line);
    }
  }

  fin.close();

  return true;
}

bool IsFile(const string &path)
{
  struct stat st;

  if (lstat(path.c_str(), &st) == -1)
    return false;

  return S_ISREG(st.st_mode);
}

// vim:sw=2:ts=2:sts=2:et:cc=72
// End of file.
