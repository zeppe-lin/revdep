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

#include <algorithm>
#include <fstream>

#include <dirent.h>

#include "pkg.h"

using namespace std;

Package::Package(const string       &name,
                 const string       &version,
                 const StringVector &files)
{
  _name    = name;
  _version = version;
  _files   = files;
  _dirs    = {};
  _ignore  = false;
}

static Package readPackage(istream &in)
{
  string line;
  size_t fields = 0;
  string name;
  string version;
  StringVector files;

  while (getline(in, line) && line != "")
  {
    switch (++fields)
    {
      case 1:                 name = line; break;

      case 2:              version = line; break;

      default: files.push_back("/" + line); break;
    }
  }

  if (fields < 2)
    return Package("", "", files);

  return Package(name, version, files);
}

bool ReadPackages(const string  &path,
                  PackageVector &pkgs)
{
  ifstream fin;

  fin.open(path.c_str());

  if (!fin.is_open())
    return false;

  while (true)
  {
    Package pkg = readPackage(fin);

    if (pkg.Name() == "" && pkg.Version() == "")
      break;

    pkgs.push_back(pkg);
  }

  fin.close();

  return(pkgs.size() > 0);
}

void ReadPackageDirs(const string  &path,
                     PackageVector &pkgs)
{
  DIR *dir;

  dir = opendir(path.c_str());

  if (dir == NULL)
    return;

  struct dirent *de;

  while ((de = readdir(dir)) != NULL)
  {
    if (de->d_type != DT_REG)
      continue;

    StringVector dirs;

    ReadRdConf(path + "/" + de->d_name, dirs);

    if (dirs.size() == 0)
      continue;

    PackageVector::iterator pkg =
      find(pkgs.begin(), pkgs.end(), de->d_name);

    if (pkg == pkgs.end())
      continue;

    pkg->Dirs(dirs);
  }

  closedir(dir);
}

// vim:sw=2:ts=2:sts=2:et:cc=72
// End of file.
