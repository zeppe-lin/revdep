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

#define _IN_PACKAGE_
#include "package.h"
#include "utility.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <search.h>

extern PackageList *packages_load_from_database(const char *path)
{
	char *text;
	unsigned int length;
	char name[1024];
	char version[1024];
	char pathname[1024];
	PackageList *pkgs;
	Package *pkg;
	FileList *files;
	File *file;

	if(path == NULL || path[0] == '\0')
	{
		errno = EINVAL;
		return NULL;
	}

	void parse_callback(int record, int field, char *start, char *end)
	{
		if(field == 0)
		{
			snprintf(name, sizeof(name), "%.*s", (int) (end - start), start);
		}
		else if(field == 1)
		{
			snprintf(version, sizeof(version), "%.*s", (int) (end - start), start);
		}
		else if(field == 2)
		{
			snprintf(pathname, sizeof(pathname), "/%.*s", (int) (end - start), start);

			files = new(FileList, 1);
			files->length = 0;
			files->size = 256;
			files->list = new(File, files->size);

			file = &files->list[files->length];
			file->path = xstrdup(pathname);
			file->key = string_crc32(pathname);
			++files->length;

			if(pkgs->length == pkgs->size)
			{
				pkgs->size *= 2;
				pkgs->list = renew(pkgs->list, Package, pkgs->size);
			}

			pkg = &pkgs->list[pkgs->length];
			pkg->name = xstrdup(name);
			pkg->version = xstrdup(version);
			pkg->files = files;
			pkg->key = string_crc32(name);
			pkg->ignore = 0;
			++pkgs->length;
		}
		else
		{
			snprintf(pathname, sizeof(pathname), "/%.*s", (int) (end - start), start);

			if(files->length == files->size)
			{
				files->size *= 2;
				files->list = renew(files->list, File, files->size);
			}

			file = &files->list[files->length];
			file->path = xstrdup(pathname);
			file->key = string_crc32(pathname);
			++files->length;
		}
	}

	if(open_file_in_memory(path, &text, &length) == -1)
	{
		return NULL;
	}

	pkgs = new(PackageList, 1);
	pkgs->length = 0;
	pkgs->size = 256;
	pkgs->list = new(Package, pkgs->size);

	if(parse_file_in_memory(text, parse_callback) == -1)
	{
		packages_free(pkgs);
		close_file_in_memory(text, length);
		return NULL;
	}

	if(pkgs->length == 0)
	{
		packages_free(pkgs);
		close_file_in_memory(text, length);
		return NULL;
	}

	close_file_in_memory(text, length);

	return pkgs;
}

extern void packages_free(PackageList *pkgs)
{
	unsigned int i;
	Package *pkg;

	if(pkgs == NULL)
		return;

	for( i = 0 ; i < pkgs->length ; ++i )
	{
		pkg = &pkgs->list[i];
		free(pkg->name);
		free(pkg->version);
		files_free(pkg->files);
	}

	free(pkgs->list);

	free(pkgs);
}

extern void packages_sort(PackageList *pkgs)
{
	if(pkgs == NULL)
		return;

	int package_compare(const void *A, const void *B)
	{
		const Package *a = A;
		const Package *b = B;

		if(a->key < b->key)
			return -1;

		if(a->key > b->key)
			return 1;

		return 0;
	}

	qsort(pkgs->list, pkgs->length, sizeof(Package), package_compare);
}

extern void packages_sort_all_files(PackageList *pkgs)
{
	unsigned int i;
	FileList *files;

	if(pkgs == NULL)
		return;

	for( i = 0 ; i < pkgs->length ; ++i )
	{
		files = pkgs->list[i].files;
		files_sort(files);
	}
}

extern Package *packages_search(PackageList *pkgs, const char *name)
{
	unsigned int key;

	if(pkgs == NULL || name == NULL || name[0] == '\0')
	{
		errno = EINVAL;
		return NULL;
	}

	int package_compare(const void *A, const void *B)
	{
		const unsigned int *a = A;
		const Package *b = B;

		if(a[0] < b->key)
			return -1;

		if(a[0] > b->key)
			return 1;

		return 0;
	}

	key = string_crc32(name);

	return bsearch(&key, pkgs->list, pkgs->length, sizeof(Package), package_compare);
}

extern void files_sort(FileList *files)
{
	if(files == NULL)
		return;

	int file_compare(const void *A, const void *B)
	{
		const File *a = A;
		const File *b = B;

		if(a->key < b->key)
			return -1;

		if(a->key > b->key)
			return 1;

		return 0;
	}

	qsort(files->list, files->length, sizeof(File), file_compare);
}

extern void files_free(FileList *files)
{
	unsigned int i;
	File *file;

	if(files == NULL)
		return;

	for( i = 0 ; i < files->length ; ++i )
	{
		file = &files->list[i];
		free(file->path);
	}

	free(files->list);

	free(files);
}

extern File *files_search(FileList *files, const char *key, int compare_type)
{
	unsigned int hash;
	size_t n;

	if(files == NULL || key == NULL || key[0] == '\0')
	{
		errno = EINVAL;
		return NULL;
	}

	int file_compare_exact(const void *A, const void *B)
	{
		const unsigned int *a = A;
		const File *b = B;

		if(a[0] < b->key)
			return -1;

		if(a[0] > b->key)
			return 1;

		return 0;
	}

	int file_compare_within(const void *A, const void *B)
	{
		const char *a = A;
		const File *b = B;

		return (strstr(b->path, a) == NULL);
	}

	switch(compare_type)
	{
		case FILE_COMPARE_EXACT:
			hash = string_crc32(key);
			return bsearch(&hash, files->list, files->length, sizeof(File), file_compare_exact);

		case FILE_COMPARE_WITHIN:
			n = files->length;
			return lfind(key, files->list, &n, sizeof(File), file_compare_within);

		default:
			return NULL;
	}
}
