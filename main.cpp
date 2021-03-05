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

#include "main.h"
#include "pkg.h"
#include "elf-cache.h"
#include <algorithm>
#include <unistd.h>
#include <stdarg.h>

enum {
	BRIEF,
	INFO1,
	INFO2,
	ERROR,
	DEBUG
};

static std::string PKG_DB_PATH = "/var/lib/pkg/db";
static std::string LD_CONF_PATH = "/etc/ld.so.conf";
static std::string RD_CONF_PATH = "/etc/revdep.d";
static StringVector ignores;
static int level = BRIEF;
static PackageVector pkgs;
static StringVector dirs;
static ElfCache ec;

static void logger(int lvl, const char *fmt, ...) __attribute__((format(printf,2,3)));

static void logger(int lvl, const char *fmt, ...) {
	if(lvl > level) {
		return;
	}

	va_list args;

	va_start(args, fmt);

	vfprintf(stdout, fmt, args);

	va_end(args);
}

static bool parseArgs(int argc, char **argv) {
	static const char OPTIONS[] = ":hd:c:r:i:v";
	int opt;

	while((opt = getopt(argc, argv, OPTIONS)) != -1) {
		switch(opt) {
			case 'h':
				logger(BRIEF, "%s version %s\n", argv[0], VERSION);
				logger(BRIEF, "Usage: %s [-h] [-v|-vv|-vvv|-vvvv] [package...]\n", argv[0]);
				logger(BRIEF, "Where commands are:\n");
				logger(BRIEF, "\t-h    The option -h displays help\n");
				logger(BRIEF, "\t-d    Specify an alternate location for the package database\n");
				logger(BRIEF, "\t-c    Specify an alternate location for ld.so.conf\n");
				logger(BRIEF, "\t-r    Specify an alternate directory for revdep package config\n");
				logger(BRIEF, "\t-i    Comma-separated list of packages to ignore\n");
				logger(BRIEF, "\t-v    Formatted listing\n");
				logger(BRIEF, "\t-vv   Include erroneous files\n");
				logger(BRIEF, "\t-vvv  Include precise file errors\n");
				logger(BRIEF, "\t-vvvv Show debug/trace\n");
				return false;

			case 'd':
				PKG_DB_PATH = optarg;
				break;

			case 'c':
				LD_CONF_PATH = optarg;
				break;

			case 'i':
				split(optarg, ignores, ',');
				break;

			case 'r':
				RD_CONF_PATH = optarg;
				break;

			case 'v':
				++level;
				break;

			case ':':
				logger(BRIEF, "%c: missing argument\n", optopt);
				return false;

			case '?':
				logger(BRIEF, "%c: invalid option\n", optopt);
				return false;
		}
	}

	return true;
}

static void ignorePackages(PackageVector &pkgs, const StringVector &ignores) {
	for( size_t i = 0 ; i < ignores.size() ; ++i ) {
		PackageVector::iterator pkg = std::find(pkgs.begin(), pkgs.end(), ignores[i]);

		if(pkg == pkgs.end()) {
			continue;
		}

		pkg->Ignore();
	}
}

static bool workFile(const Package &pkg, const std::string &file) {
	bool rv = true;

	logger(DEBUG, "%s:%s: checking file\n", pkg.Name().c_str(), file.c_str());

	if(!IsFile(file)) {
		return rv;
	}

	const Elf *elf = ec.LookUp(file);

	if(elf == NULL) {
		return rv;
	}

	logger(DEBUG, "%s:%s: is ELF\n", pkg.Name().c_str(), file.c_str());

	for( size_t i = 0 ; i < elf->Needed().size() ; ++i ) {
		const std::string &lib = elf->Needed()[i];

		if(!ec.FindLibrary(elf, pkg, lib, dirs)) {
			logger(ERROR, "%s:%s:%s: missing library\n", pkg.Name().c_str(), file.c_str(), lib.c_str());
			rv = false;
		}
	}

	return rv;
}

static bool workPackage(const Package &pkg) {
	bool rv = true;

	if(pkg.Ignore()) {
		return rv;
	}

	for( size_t i = 0 ; i < pkg.Files().size() ; ++i ) {
		const std::string &file = pkg.Files()[i];

		if(!workFile(pkg, file)) {
			logger(INFO2, "%s:%s: error\n", pkg.Name().c_str(), file.c_str());
			rv = false;
		}
	}

	return rv;
}

static int workAllPackages(const PackageVector &pkgs) {
	int rc = 0;

	logger(INFO1, "** checking %zu packages\n", pkgs.size());
	logger(INFO1, "** checking linking\n");

	for( size_t i = 0 ; i < pkgs.size() ; ++i ) {
		const Package &pkg = pkgs[i];

		if(!workPackage(pkg)) {
			int lvl;
			const char *fmt;
			rc = 4;

			if(level > BRIEF) {
				lvl = INFO1;
				fmt = "%s: error\n";
			} else {
				lvl = BRIEF;
				fmt = "%s\n";
			}

			logger(lvl, fmt, pkg.Name().c_str());
		} else {
			logger(INFO1, "%s: ok\n", pkg.Name().c_str());
		}
	}

	return rc;
}

static int workSpecificPackages(const PackageVector &pkgs, int i, int argc, char **argv) {
	int rc = 0;

	logger(INFO1, "** checking %d packages\n", argc - i);
	logger(INFO1, "** checking linking\n");

	for( ; i < argc ; ++i ) {
		const std::string name = argv[i];
		PackageVector::const_iterator pkg = std::find(pkgs.begin(), pkgs.end(), name);

		if(pkg == pkgs.end()) {
			logger(INFO1, "%s: cannot find package information\n", name.c_str());
			continue;
		}

		if(!workPackage(pkg[0])) {
			int lvl;
			const char *fmt;
			rc = 4;

			if(level > BRIEF) {
				lvl = INFO1;
				fmt = "%s: error\n";
			} else {
				lvl = BRIEF;
				fmt = "%s\n";
			}

			logger(lvl, fmt, pkg->Name().c_str());
		} else {
			logger(INFO1, "%s: ok\n", pkg->Name().c_str());
		}
	}

	return rc;
}

int main(int argc, char **argv) {
	if(!parseArgs(argc, argv)) {
		return 1;
	}

	if(!ReadPackages(PKG_DB_PATH, pkgs)) {
		logger(BRIEF, "%s:%s: failed to read package database\n", argv[0], PKG_DB_PATH.c_str());
		return 2;
	}

	if(!ReadLdConf(LD_CONF_PATH, dirs, 10)) {
		logger(BRIEF, "%s:%s: failed to read ld conf\n", argv[0], LD_CONF_PATH.c_str());
		return 3;
	}

	dirs.push_back("/lib");
	dirs.push_back("/usr/lib");

	ReadPackageDirs(RD_CONF_PATH, pkgs);
	ignorePackages(pkgs, ignores);

	logger(INFO1, "** calculating deps\n");

	if(optind == argc) {
		return workAllPackages(pkgs);
	} else {
		return workSpecificPackages(pkgs, optind, argc, argv);
	}
}
