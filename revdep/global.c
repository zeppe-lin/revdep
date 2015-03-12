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

#define _IN_GLOBAL_
#include "global.h"
#include "utility.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <gelf.h>

Global_Data rd =
{
	.verbose = 0
};

static void ignore_packages(char *list, PackageList *pkgs)
{
	char *s;
	char *e;
	Package *pkg;

	for( s = list ; (e = strchr(s, ',')) != NULL ; s = e + 1 )
	{
		e[0] = '\0';

		if((pkg = packages_search(pkgs, s)) != NULL)
		{
			pkg->ignore = 1;
		}

		e[0] = ',';
	}

	if((pkg = packages_search(pkgs, s)) != NULL)
	{
		pkg->ignore = 1;
	}
}

extern int global_setup(int argc, char **argv, int *arg_start, PackageList *pkgs)
{
	static const char options[] = ":vi:h";
	int opt;

	if(argc < 1 || argv == NULL || arg_start == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	if(elf_version(EV_CURRENT) == EV_NONE)
	{
		logger(LOG_BRIEF, "libelf: could not initialize version\n");
		return -1;
	}

	while((opt = getopt(argc, argv, options)) != -1)
	{
		switch(opt)
		{
			case 'v':
				++rd.verbose;
				break;

			case 'i':
				ignore_packages(optarg, pkgs);
				break;

			case 'h':
				logger(LOG_BRIEF,
					"%s version %s\n"
					"Usage: %s [-h] [-v|-vv|-vvv|-vvvv] [package...]\n"
					"Where commands are:\n"
					"\t-h    The option -h displays help.\n"
					"\t-i    Comma-separated list of ports to ignore.\n"
					"\t-v    Formatted listing.\n"
					"\t-vv   Include erroneous files.\n"
					"\t-vvv  Include precise file errors.\n"
					"\t-vvvv Show debug/trace.\n",
					argv[0],
					VERSION,
					argv[0]
				);
				return -1;

			case '?':
				logger(LOG_BRIEF, "%c: unknown option\n", optopt);
				return -1;

			case ':':
				logger(LOG_BRIEF, "%c: missing argument\n", optopt);
				return -1;
		}
	}

	arg_start[0] = optind;

	return 0;
}
