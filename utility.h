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

#include <vector>
#include <string>

typedef std::vector <std::string> StringVector;

void split(const std::string  &in,
           StringVector       &out,
           char               delimiter);

void ReadRdConf(const std::string  &path,
                StringVector       &dirs);

bool ReadLdConf(const std::string &path,
                StringVector      &dirs,
                int               maxDepth);

bool IsFile(const std::string &path);

// vim:sw=2:ts=2:sts=2:et:cc=72
// End of file.
