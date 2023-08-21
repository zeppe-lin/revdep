//! \file  main.cpp
//! \brief Command-line utility of \a revdep.
//!        See COPYING and COPYRIGHT files for corresponding information.

#include <algorithm>
#include <iostream>

#include <getopt.h>
#include <stdarg.h>
#include <unistd.h>

#include "elf-cache.h"
#include "pkg.h"
#include "pathnames.h"

using namespace std;

/*********************************************************************
 * Globals.
 */

/* Command-line options */
static int           o_verbose   = 0;
static int           o_erroneous = 0;
static int           o_precise   = 0;
static int           o_trace     = 0;
static string        o_revdepdir = _PATH_REVDEPD;
static string        o_pkgdb     = _PATH_PKGDB;
static string        o_ldsoconf  = _PATH_LDSOCONF;

static StringVector  o_IgnoredPackages;
static PackageVector o_packages;

static StringVector  dirs;
static ElfCache      ec;

/* Used for exit status */
enum _revdep_errors {
  E_INVALID_INPUT = 1, /* Failed to parse command-line arguments */
  E_READ_PKGDB    = 2, /* Failed to read package database */
  E_READ_LDSOCONF = 3, /* Failed to read ld.so.conf */
  E_FOUND_MISSING = 4, /* Found at least one missing library */
};

/*********************************************************************
 * Function declarations.
 */

static void ignorePackages(PackageVector &, const StringVector &);
static bool workFile(const Package &, const string &);
static bool workPackage(const Package &);
static int workAllPackages(const PackageVector &);
static int workSpecificPackages(const PackageVector &, int, int, char **);
static void ignorePackages(PackageVector &, const StringVector &);
static int print_help();
static int print_version();

/*********************************************************************
 * Function implementations.
 */

static bool
workFile(const Package &pkg, const string &file)
{
  bool rv = true;

  if (o_trace)
    cout << pkg.Name() << ":" << file << ": checking file" << endl;

  if (!IsRegularFile(file))
    return rv;

  const Elf *elf = ec.LookUp(file);

  if (elf == NULL)
    return rv;

  if (o_trace)
    cout << pkg.Name() << ":" << file << ": is ELF" << endl;

  for (size_t i = 0; i < elf->Needed().size(); ++i)
  {
    const string &lib = elf->Needed()[i];

    if (!ec.FindLibrary(elf, pkg, lib, dirs))
    {
      if (o_precise)
        cout << pkg.Name() << ":" << file << ":" << lib
             << ": missing library" << endl;

      rv = false;
    }
  }

  return rv;
}

static bool
workPackage(const Package &pkg)
{
  bool rv = true;

  if (pkg.Ignore())
    return rv;

  for (size_t i = 0; i < pkg.Files().size(); ++i)
  {
    const string &file = pkg.Files()[i];

    if (!workFile(pkg, file))
    {
      if (o_erroneous)
        cout << pkg.Name() << ":" << file << ": error" << endl;

      rv = false;
    }
  }

  return rv;
}

static int
workAllPackages(const PackageVector &pkgs)
{
  int rc = 0;

  if (o_verbose)
  {
    printf("** checking %zu packages\n", pkgs.size());
    printf("** checking linking\n");
  }

  for (size_t i = 0; i < pkgs.size(); ++i)
  {
    const Package &pkg = pkgs[i];

    if (!workPackage(pkg))
    {
      rc = E_FOUND_MISSING;

      if (o_verbose)
        cout << pkg.Name() << ": error" << endl;
      else
        cout << pkg.Name() << endl;
    }
    else
    {
      if (o_verbose)
        cout << pkg.Name() << ": ok" << endl;
    }
  }

  return rc;
}

static int
workSpecificPackages(const PackageVector &pkgs,
                     int                 i,
                     int                 argc,
                     char                **argv)
{
  int rc = 0;

  if (o_verbose)
  {
    printf("** checking %d packages\n", argc - i);
    printf("** checking linking\n");
  }

  for ( ; i < argc; ++i)
  {
    const string name = argv[i];
    PackageVector::const_iterator pkg =
      find(pkgs.begin(), pkgs.end(), name);

    if (pkg == pkgs.end())
    {
      if (o_verbose)
        cout << name << ": cannot find package information" << endl;

      continue;
    }

    if (!workPackage(pkg[0]))
    {
      rc = E_FOUND_MISSING;

      if (o_verbose)
        cout << pkg->Name() << ": error" << endl;
      else
        cout << pkg->Name() << endl;
    }
    else
    {
      if (o_verbose)
        cout << pkg->Name() << ": ok" << endl;
    }
  }

  return rc;
}

static void
ignorePackages(PackageVector &pkgs, const StringVector &ignores)
{
  for (size_t i = 0; i < ignores.size(); ++i)
  {
    PackageVector::iterator pkg =
      find(pkgs.begin(), pkgs.end(), ignores[i]);

    if (pkg == pkgs.end())
      continue;

    pkg->Ignore();
  }
}

static int
print_help()
{
  cout << R"(Usage: revdep [OPTION]... [PKGNAME]...
Check for missing libraries of installed packages.

Mandatory arguments to long options are mandatory for short options too.
  -L, --ldsoconf=PATH   specify an alternate location for ld.so.conf
  -D, --pkgdb=PATH      specify an alternate location for the packages database
  -R, --revdepdir=PATH  specify an alternate location for revdep package config
  -I, --ignore=PKGNAME[,...]  comma-separated list of packages to ignore
  -e, --erroneous       include erroneous files in the output
  -p, --precise         include precise file errors in the output
  -t, --trace           show debug/trace
  -v, --verbose         formatted listing
  -V, --version         print version and exit
  -h, --help            print help and exit
)";
  return 0;
}

static int
print_version()
{
  cout << "revdep " VERSION "\n";
  return 0;
}

int
main(int argc, char **argv)
{
  static struct option longopts[] = {
    { "ldsoconf",   required_argument,  NULL,             'L' },
    { "pkgdb",      required_argument,  NULL,             'D' },
    { "revdepdir",  required_argument,  NULL,             'R' },
    { "ignore",     required_argument,  NULL,             'I' },
    { "erroneous",  no_argument,        NULL,             'e' },
    { "precise",    no_argument,        NULL,             'p' },
    { "trace",      no_argument,        NULL,             't' },
    { "verbose",    no_argument,        NULL,             'v' },
    { "version",    no_argument,        NULL,             'V' },
    { "help",       no_argument,        NULL,             'h' },
    { 0,            0,                  0,                0   },
  };

  int opt;

  while ((opt =
          getopt_long(argc, argv, "L:D:R:I:eptvVh", longopts, 0))
      != -1)
  {
    switch (opt)
    {
      case 'L':
        o_ldsoconf = optarg;
        break;

      case 'D':
        o_pkgdb = optarg;
        break;

      case 'R':
        o_revdepdir = optarg;
        break;

      case 'I':
        split(optarg, o_IgnoredPackages, ',');
        break;

      case 'v':
        o_verbose = 1;
        break;

      case 'e':
        o_erroneous = 1;
        break;

      case 'p':
        o_precise = 1;
        break;

      case 't':
        o_trace = 1;
        break;

      case 'V':
        return print_version();

      case 'h':
        return print_help();

      case ':':
        fprintf(stderr, "%c: missing argument\n", optopt);
        return E_INVALID_INPUT;

      case '?':
        fprintf(stderr, "%c: invalid option\n", optopt);
        return E_INVALID_INPUT;
    }
  }

  if (!ReadPackages(o_pkgdb, o_packages))
  {
    cerr << "revdep: " << o_pkgdb << ": failed to read package database\n";
    return E_READ_PKGDB;
  }

  if (!ReadLdConf(o_ldsoconf, dirs, 10))
  {
    cerr << "revdep: " << o_ldsoconf << ": failed to read ld configuration\n";
    return E_READ_LDSOCONF;
  }

  dirs.push_back("/lib");
  dirs.push_back("/usr/lib");

  ReadPackageDirs(o_revdepdir, o_packages);
  ignorePackages(o_packages, o_IgnoredPackages);

  if (o_verbose)
    cout << "** calculating deps" << endl;

  if (optind == argc)
    return workAllPackages(o_packages);
  else
    return workSpecificPackages(o_packages, optind, argc, argv);
}

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
