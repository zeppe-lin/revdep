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

#pragma once

#include <unordered_map>

#include "elf.h"
#include "pkg.h"

using namespace std;

typedef unordered_map <string, Elf *> ElfMap;

class ElfCache {
private:

  ElfMap _data;
  bool findLibraryByDirs(const Elf          *elf,
                         const string       &lib,
                         const StringVector &dirs);

  bool findLibraryByPath(const Elf    *elf,
                         const string &lib);

public:

  ~ElfCache();

  const Elf *LookUp(const string &path);

  bool FindLibrary(const Elf          *elf,
                   const Package      &pkg,
                   const string       &lib,
                   const StringVector &dirs);
};

// vim:sw=2:ts=2:sts=2:et:cc=72
// End of file.
