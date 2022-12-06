// See COPYING and COPYRIGHT files for corresponding information.

#pragma once

#include "utility.h"

class Elf {
private:

  int          _machine;
  StringVector _needed;
  StringVector _rpath;
  StringVector _runpath;
  std::string  _path;
  bool         _initialized;

public:
  Elf(const std::string &path);

  int Machine() const
  {
    return _machine;
  }

  const StringVector& Needed() const
  {
    return _needed;
  }

  const StringVector& RPath() const
  {
    return _rpath;
  }

  const StringVector& RunPath() const
  {
    return _runpath;
  }

  const std::string& Path() const
  {
    return _path;
  }

  bool Valid() const
  {
    return _initialized;
  }

  bool Compatible(const Elf &elf) const
  {
    return _machine == elf._machine;
  }
};

// vim:sw=2:ts=2:sts=2:et:cc=72:tw=70
// End of file.
