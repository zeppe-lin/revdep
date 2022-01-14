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

// vim:sw=2:ts=2:sts=2:et:cc=72
// End of file.
