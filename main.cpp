// See COPYING and COPYRIGHT files for corresponding information.

#include <algorithm>
#include <iostream>

#include <getopt.h>
#include <stdarg.h>
#include <unistd.h>

#include "elf-cache.h"
#include "main.h"
#include "pkg.h"

using namespace std;

static int do_help, do_version;
static int show_verbose, show_erroneous, show_precise, show_trace;
static string path_revdep_d   = SYSCONFDIR"/revdep.d";
static string path_pkg_db     = "/var/lib/pkg/db";
static string path_ld_so_conf = "/etc/ld.so.conf";

static StringVector ignores;
static PackageVector pkgs;
static StringVector dirs;
static ElfCache ec;

static void ignorePackages(PackageVector &pkgs, const StringVector &ignores)
{
  for (size_t i = 0; i < ignores.size(); ++i)
  {
    PackageVector::iterator pkg = find(pkgs.begin(), pkgs.end(), ignores[i]);

    if (pkg == pkgs.end())
      continue;

    pkg->Ignore();
  }
}

static bool workFile(const Package &pkg, const string &file)
{
  bool rv = true;

  if (show_trace)
    cout << pkg.Name() << ":" << file << ": checking file" << endl;

  if (!IsRegularFile(file))
    return rv;

  const Elf *elf = ec.LookUp(file);

  if (elf == NULL)
    return rv;

  if (show_trace)
    cout << pkg.Name() << ":" << file << ": is ELF" << endl;

  for (size_t i = 0; i < elf->Needed().size(); ++i)
  {
    const string &lib = elf->Needed()[i];

    if (!ec.FindLibrary(elf, pkg, lib, dirs))
    {
      if (show_precise)
        cout << pkg.Name() << ":" << file << ":" << lib << ": missing library" << endl;

      rv = false;
    }
  }

  return rv;
}

static bool workPackage(const Package &pkg)
{
  bool rv = true;

  if (pkg.Ignore())
    return rv;

  for (size_t i = 0; i < pkg.Files().size(); ++i)
  {
    const string &file = pkg.Files()[i];

    if (!workFile(pkg, file))
    {
      if (show_erroneous)
        cout << pkg.Name() << ":" << file << ": error" << endl;

      rv = false;
    }
  }

  return rv;
}

static int workAllPackages(const PackageVector &pkgs)
{
  int rc = 0;

  if (show_verbose)
  {
    printf("** checking %zu packages\n", pkgs.size());
    printf("** checking linking\n");
  }

  for (size_t i = 0; i < pkgs.size(); ++i)
  {
    const Package &pkg = pkgs[i];

    if (!workPackage(pkg))
    {
      rc = 4;

      if (show_verbose)
        cout << pkg.Name() << ": error" << endl;
      else
        cout << pkg.Name() << endl;
    }
    else
    {
      if (show_verbose)
        cout << pkg.Name() << ": ok" << endl;
    }
  }

  return rc;
}

static int workSpecificPackages(const PackageVector &pkgs, int i, int argc, char **argv)
{
  int rc = 0;

  if (show_verbose)
  {
    printf("** checking %d packages\n", argc - i);
    printf("** checking linking\n");
  }

  for ( ; i < argc; ++i)
  {
    const string name = argv[i];
    PackageVector::const_iterator pkg = find(pkgs.begin(), pkgs.end(), name);

    if (pkg == pkgs.end())
    {
      if (show_verbose)
        cout << name << ": cannot find package information" << endl;

      continue;
    }

    if (!workPackage(pkg[0]))
    {
      rc = 4;

      if (show_verbose)
        cout << pkg->Name() << ": error" << endl;
      else
        cout << pkg->Name() << endl;
    }
    else
    {
      if (show_verbose)
        cout << pkg->Name() << ": ok" << endl;
    }
  }

  return rc;
}

int main(int argc, char **argv)
{
  static struct option longopts[] = {
    { "ld.so.conf", required_argument,  NULL,             'c' },
    { "pkgdb",      required_argument,  NULL,             'd' },
    { "revdep.d",   required_argument,  NULL,             'r' },
    { "ignore",     required_argument,  NULL,             'i' },
    { "verbose",    no_argument,        &show_verbose,    1   },
    { "erroneous",  no_argument,        &show_erroneous,  1   },
    { "precise",    no_argument,        &show_precise,    1   },
    { "trace",      no_argument,        &show_trace,      1   },
    { "version",    no_argument,        &do_version,      1   },
    { "help",       no_argument,        &do_help,         1   },
    { 0,            0,                  0,                0   },
  };

  int opt;

  while ((opt = getopt_long(argc, argv, ":hd:c:r:i:vVept", longopts, 0)) != -1)
  {
    switch (opt)
    {
      case 'c':
        path_ld_so_conf = optarg;
        break;

      case 'd':
        path_pkg_db = optarg;
        break;

      case 'r':
        path_revdep_d = optarg;
        break;

      case 'i':
        split(optarg, ignores, ',');
        break;

      case 'V':
        show_verbose = 1;
        break;

      case 'e':
        show_erroneous = 1;
        break;

      case 'p':
        show_precise = 1;
        break;

      case 't':
        show_trace = 1;
        break;

      case 'v':
        do_version = 1;
        break;

      case 'h':
        do_help = 1;
        break;

      case ':':
        fprintf(stderr, "%c: missing argument\n", optopt);
        return 1;

      case '?':
        fprintf(stderr, "%c: invalid option\n", optopt);
        return 1;
    }
  }

  if (do_help)
  {
    cout << R"END(Usage: revdep [OPTION]... [PKGNAME]...
Check for missing libraries of installed packages.

Mandatory arguments to long options are mandatory for short options too.
  -c, --ld.so.conf=PATH     specify an alternate location for ld.so.conf
  -d, --pkgdb=PATH          specify an alternate location for the packages database
  -r, --revdep.d=PATH       specify an alternate location for revdep package config
  -i, --ignore=PKGNAME,...  comma-separated list of packages to ignore
  -V, --verbose             formatted listing
  -e, --erroneous           include erroneous files in the output
  -p, --precise             include precise file errors in the output
  -t, --trace               show debug/trace
  -v, --version             print version and exit
  -h, --help                print help and exit
)END";
    return 0;
  }
  else if (do_version)
  {
    cout << "revdep " << VERSION << endl;
    return 0;
  }

  if (!ReadPackages(path_pkg_db, pkgs))
  {
    cerr << "revdep:" << path_pkg_db
         << ": failed to read package database" << endl;
    return 2;
  }

  if (!ReadLdConf(path_ld_so_conf, dirs, 10))
  {
    cerr << "revdep:" << path_ld_so_conf
         << ": failed to read ld configuration" << endl;
    return 3;
  }

  dirs.push_back("/lib");
  dirs.push_back("/usr/lib");

  ReadPackageDirs(path_revdep_d, pkgs);
  ignorePackages(pkgs, ignores);

  if (show_verbose)
    cout << "** calculating deps" << endl;

  if (optind == argc)
    return workAllPackages(pkgs);
  else
    return workSpecificPackages(pkgs, optind, argc, argv);
}

// vim:sw=2:ts=2:sts=2:et:cc=72:tw=70
// End of file.
