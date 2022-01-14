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

#include <libgen.h>
#include <limits.h>
#include <sys/auxv.h>

#include "elf-cache.h"

using namespace std;

typedef pair <const string &, Elf *> ElfPair;
typedef unordered_map <string, Elf *>::iterator ElfIter;

static void deleteElement(ElfPair pair)
{
  delete pair.second;
}

static string resolveDirVars(const Elf *elf, const string &path)
{
  static const char *lib = "lib";

  static const char *platform =
    (((char *)getauxval(AT_PLATFORM)) ?
     ((char *)getauxval(AT_PLATFORM)) : "");

  char dir[PATH_MAX];

  snprintf(dir, sizeof(dir), "%s", elf->Path().c_str());
  dirname(dir);

  struct {
    const char *name;
    size_t      length;
    const char *s;
  } vars[] = {
    {        "$LIB",  4,      lib },
    {      "${LIB}",  6,      lib },
    {   "$PLATFORM",  9, platform },
    { "${PLATFORM}", 11, platform },
    {     "$ORIGIN",  7,      dir },
    {   "${ORIGIN}",  9,      dir },
    {          NULL,  0,     NULL }
  };

  size_t replaces;
  string out = path;

  do
  {
    replaces = 0;

    for (size_t i = 0; vars[i].name != NULL; ++i)
    {
      size_t j = out.find(vars[i].name);

      if (j != string::npos)
      {
        out.replace(j, vars[i].length, vars[i].s);
        ++replaces;
      }
    }
  } while (replaces > 0);

  return out;
}

static StringVector resolveRunPaths(const Elf          *elf,
                                    const StringVector &paths)
{
  StringVector out;

  for (size_t i = 0; i < paths.size(); ++i)
    out.push_back(resolveDirVars(elf, paths[i]));

  return out;
}

bool ElfCache::findLibraryByDirs(const Elf          *elf,
                                 const string       &lib,
                                 const StringVector &dirs)
{
  for (size_t i = 0; i < dirs.size(); ++i)
  {
    string path = dirs[i] + "/" + lib;
    char realPath[PATH_MAX];

    if (realpath(path.c_str(), realPath) == NULL)
      continue;

    const Elf *elf2 = LookUp(realPath);

    if (elf2 == NULL)
      continue;

    if (!elf->Compatible(elf2[0]))
      continue;

    return true;
  }

  return false;
}

bool ElfCache::findLibraryByPath(const Elf    *elf,
                                 const string &lib)
{
  string path;

  if (lib[0] == '/')
  {
    path = lib;
  }
  else
  {
    char dir[PATH_MAX];

    snprintf(dir, sizeof(dir), "%s", elf->Path().c_str());
    dirname(dir);

    path = dir + ("/" + lib);
  }

  char realPath[PATH_MAX];

  if (realpath(path.c_str(), realPath) == NULL)
    return false;

  const Elf *elf2 = LookUp(realPath);

  if (elf2 == NULL)
    return false;

  return elf->Compatible(elf2[0]);
}

ElfCache::~ElfCache()
{
  for_each(_data.begin(), _data.end(), deleteElement);
}

const Elf *ElfCache::LookUp(const string &path)
{
  ElfIter value = _data.find(path);

  if (value != _data.end())
    return value->second;

  Elf *elf = new Elf(path);

  if (!elf->Valid())
  {
    delete elf;
    return NULL;
  }

  ElfPair pair =
    make_pair <const string &, Elf *&> (path, elf);

  _data.insert(pair);

  return elf;
}

bool ElfCache::FindLibrary(const Elf          *elf,
                           const Package      &pkg,
                           const string       &lib,
                           const StringVector &dirs)
{
  if (lib.find('/') != string::npos)
    return findLibraryByPath(elf, lib);

  StringVector paths;

  if (elf->RunPath().size() > 0)
  {
    paths = resolveRunPaths(elf, elf->RunPath());
    if (findLibraryByDirs(elf, lib, paths))
      return true;
  }
  else if (elf->RPath().size() > 0)
  {
    paths = resolveRunPaths(elf, elf->RPath());
    if (findLibraryByDirs(elf, lib, paths))
      return true;
  }

  if (findLibraryByDirs(elf, lib, dirs))
    return true;

  if (pkg.Dirs().size() > 0 && findLibraryByDirs(elf, lib, pkg.Dirs()))
    return true;

  return false;
}

// vim:sw=2:ts=2:sts=2:et:cc=72
// End of file.
