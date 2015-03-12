// Copyright (C) 2015 James Buren
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

typedef struct
{
	char *path;
	unsigned int key;
} File;

typedef struct
{
	File *list;
	unsigned int length;
	unsigned int size;
} FileList;

typedef struct
{
	char *name;
	char *version;
	FileList *files;
	unsigned int key;
	unsigned int ignore;
} Package;

typedef struct
{
	Package *list;
	unsigned int length;
	unsigned int size;
} PackageList;

enum
{
	FILE_COMPARE_EXACT,
	FILE_COMPARE_WITHIN
};

extern PackageList *packages_load_from_database(const char *path);
extern void packages_free(PackageList *pkgs);
extern void packages_sort(PackageList *pkgs);
extern void packages_sort_all_files(PackageList *pkgs);
extern Package *packages_search(PackageList *pkgs, const char *name);
extern void files_sort(FileList *files);
extern void files_free(FileList *files);
extern File *files_search(FileList *files, const char *key, int compare_type);
