//! \file  elf-cache.h
//! \brief ElfCache class definition.
//!        See COPYING and COPYRIGHT files for corresponding information.

#pragma once

#include <unordered_map>

#include "elf.h"
#include "pkg.h"

using namespace std;

typedef unordered_map <string, Elf *> ElfMap;

class ElfCache {
private:

  ElfMap _data;

  bool findLibraryByDirs(const Elf *elf, const string &lib,
      const StringVector &dirs);

  bool findLibraryByPath(const Elf *elf, const string &lib);

public:

  ~ElfCache();

  const Elf *LookUp(const string &path);

  bool FindLibrary(const Elf *elf, const Package &pkg,
      const string &lib, const StringVector &dirs);

}; // class ElfCache

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
