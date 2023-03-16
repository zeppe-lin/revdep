// See COPYING and COPYRIGHT files for corresponding information.

#pragma once

#include "utility.h"

using namespace std;

class Package {
private:
  string        _name;
  string        _version;
  StringVector  _files;
  StringVector  _dirs;
  bool          _ignore;

public:
  Package(const string       &name,
          const string       &version,
          const StringVector &files);

  const string& Name() const
  {
    return _name;
  }

  const string& Version() const
  {
    return _version;
  }

  const StringVector& Files() const
  {
    return _files;
  }

  const StringVector& Dirs() const
  {
    return _dirs;
  }

  void Dirs(const StringVector &dirs)
  {
    _dirs = dirs;
  }

  bool Ignore() const
  {
    return _ignore;
  }

  void Ignore()
  {
    _ignore = true;
  }

  bool operator ==(const string &name) const
  {
    return _name == name;
  }
}; // class Package

typedef vector <Package> PackageVector;

bool ReadPackages(const string  &path,
                  PackageVector &pkgs);

void ReadPackageDirs(const string  &path,
                     PackageVector &pkgs);

// vim:sw=2:ts=2:sts=2:et:cc=72:tw=70
// End of file.
