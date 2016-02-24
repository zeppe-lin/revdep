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

class Package {
private:
	std::string _name;
	std::string _version;
	StringVector _files;
	StringVector _dirs;
	bool _ignore;

public:
	Package(const std::string &name, const std::string &version, const StringVector &files);

	const std::string&  Name()                         const { return _name;    }
	const std::string&  Version()                      const { return _version; }
	const StringVector& Files()                        const { return _files;   }
	const StringVector& Dirs()                         const { return _dirs;    }
	      void          Dirs(const StringVector &dirs)       { _dirs = dirs;    }
	      bool          Ignore()                       const { return _ignore;  }
	      void          Ignore()                             { _ignore = true;  }

	bool operator == (const std::string &name) const { return _name == name; }
};

typedef std::vector <Package> PackageVector;

bool ReadPackages(const std::string &path, PackageVector &pkgs);
void ReadPackageDirs(const std::string &path, PackageVector &pkgs);
