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

#define _IN_MAIN_
#include "main.h"
#include "global.h"
#include "package.h"
#include "utility.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

static int run_ld_on_file(Package *pkg, File *file)
{
	const char *ld;
	char command[4096];
	FILE *in;
	char line[4096];
	char libs[4096];
	char *s;
	char *e;
	File *file2;
	char dir[4096];
	char dirs[4096];

	inline int run_ld(void)
	{
		if((in = popen(command, "r")) == NULL)
		{
			logger(LOG_ERROR, "%s:%s: could not execute LD\n", pkg->name, file->path);
			return -1;
		}

		libs[0] = '\0';

		while(fgets(line, sizeof(line), in) != NULL)
		{
			if(strstr(line, "not found") != NULL)
			{
				if((s = strstr(line, "=>")) != NULL)
				{
					s[0] = '\0';
				}

				snprintf(libs + strlen(libs), sizeof(libs) - strlen(libs), "%s%s",
					(libs[0] == '\0') ? "" : ":", strtrim(line));
			}
		}

		pclose(in);

		if(libs[0] == '\0')
		{
			return 0;
		}

		return 1;
	}

	logger(LOG_DEBUG, "%s:%s: checking file\n", pkg->name, file->path);

	if(get_ld_for_file(pkg->name, file->path, &ld) == -1)
	{
		return 0;
	}

	logger(LOG_DEBUG, "%s:%s: is ELF\n", pkg->name, file->path);

	snprintf(command, sizeof(command), "LD_TRACE_LOADED_OBJECTS=1 '%s' '%s'", ld, file->path);

	switch(run_ld())
	{
		case  0:
			return  0;
		case -1:
			return -1;
		case  1:
			break;
	}

	for( s = libs, dirs[0] = '\0' ; (e = strchr(s, ':')) != NULL ; s = e + 1 )
	{
		e[0] = '\0';

		if((file2 = files_search(pkg->files, s, FILE_COMPARE_WITHIN)) != NULL)
		{
			snprintf(dir, sizeof(dir), "%s", file2->path);
			snprintf(dirs + strlen(dirs), sizeof(dirs) - strlen(dirs), "%s%s",
				(dirs[0] == '\0') ? "" : ":", dirname(dir));
		}
		else
		{
			logger(LOG_ERROR, "%s:%s: missing library\n", pkg->name, file->path);
			return -1;
		}

		e[0] = ':';
	}

	if((file2 = files_search(pkg->files, s, FILE_COMPARE_WITHIN)) != NULL)
	{
		snprintf(dir, sizeof(dir), "%s", file2->path);
		snprintf(dirs + strlen(dirs), sizeof(dirs) - strlen(dirs), "%s%s",
			(dirs[0] == '\0') ? "" : ":", dirname(dir));
	}
	else
	{
		logger(LOG_ERROR, "%s:%s: missing library\n", pkg->name, file->path);
		return -1;
	}

	snprintf(command, sizeof(command), "LD_TRACE_LOADED_OBJECTS=1 LD_LIBRARY_PATH=\"$LD_LIBRARY_PATH:%s\" '%s' '%s'", dirs, ld, file->path);

	switch(run_ld())
	{
		case  0:
			return  0;
		case -1:
			return -1;
		case  1:
			break;
	}

	logger(LOG_ERROR, "%s:%s: missing library\n", pkg->name, file->path);

	return -1;
}

static int work_package(Package *pkg)
{
	unsigned int i;
	FileList *files;
	File *file;
	int rv;

	if(pkg->ignore)
	{
		return 0;
	}

	for( i = rv = 0, files = pkg->files ; i < files->length ; ++i )
	{
		file = &files->list[i];

		if(run_ld_on_file(pkg, file) == -1)
		{
			logger(LOG_INFO_2, "%s:%s: error\n", pkg->name, file->path);

			rv = -1;
		}
	}

	return rv;
}

static void work_all_packages(PackageList *pkgs)
{
	unsigned int i;
	Package *pkg;

	logger(LOG_INFO_1,
		"** checking %u ports\n"
		"** check linking\n",
		pkgs->length
	);

	for( i = 0 ; i < pkgs->length ; ++i )
	{
		pkg = &pkgs->list[i];

		if(work_package(pkg) == -1)
		{
			const char *fmt;
			int level;

			if(rd.verbose == 0)
			{
				fmt = "%s\n";
				level = LOG_BRIEF;
			}
			else
			{
				fmt = "%s: error\n";
				level = LOG_INFO_1;
			}

			logger(level, fmt, pkg->name);
		}
		else
		{
			logger(LOG_INFO_1, "%s: ok\n", pkg->name);
		}
	}
}

static void work_specific_packages(PackageList *pkgs, int i, int argc, char **argv)
{
	char *name;
	Package *pkg;

	logger(LOG_INFO_1,
		"** checking %d ports\n"
		"** check linking\n",
		argc - i
	);

	for( ; i < argc ; ++i )
	{
		name = argv[i];

		if((pkg = packages_search(pkgs, name)) == NULL)
		{
			logger(LOG_INFO_1, "%s: cannot find package information\n", name);
			continue;
		}

		if(work_package(pkg) == -1)
		{
			const char *fmt;
			int level;

			if(rd.verbose == 0)
			{
				fmt = "%s\n";
				level = LOG_BRIEF;
			}
			else
			{
				fmt = "%s: error\n";
				level = LOG_INFO_1;
			}

			logger(level, fmt, pkg->name);
		}
		else
		{
			logger(LOG_INFO_1, "%s: ok\n", pkg->name);
		}
	}
}

extern int main(int argc, char **argv)
{
	static const char DB[] = "/var/lib/pkg/db";
	int start;
	PackageList *pkgs;

	pkgs = packages_load_from_database(DB);

	if(pkgs == NULL)
	{
		logger(LOG_BRIEF, "%s: could not open the database\n", DB);
		return EXIT_FAILURE;
	}

	packages_sort(pkgs);
	packages_sort_all_files(pkgs);

	if(global_setup(argc, argv, &start, pkgs) == -1)
	{
		return EXIT_FAILURE;
	}

	logger(LOG_INFO_1, "** calculating deps\n");

	if(start == argc)
	{
		work_all_packages(pkgs);
	}
	else
	{
		work_specific_packages(pkgs, start, argc, argv);
	}

	packages_free(pkgs);

	return EXIT_SUCCESS;
}
