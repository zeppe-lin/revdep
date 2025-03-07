//! \file  main.cpp
//! \brief Command-line utility of \a revdep.
//!
//! This file implements the main function for the `revdep`
//! command-line utility.  `revdep` is designed to check for missing
//! shared library dependencies for installed packages.  It parses
//! command-line options, reads package information from a database,
//! analyzes ELF files, and reports any missing dependencies.
//
//! See COPYING and COPYRIGHT files for corresponding information.

#include <algorithm>   // For std::find
#include <iostream>    // For std::cout, std::cerr, std::endl

#include <getopt.h>    // For getopt_long
#include <stdarg.h>    // For va_list, va_start, va_end (not directly used but included)
#include <unistd.h>    // For fprintf, getopt

#include "elf_cache.h"
#include "pkg.h"
#include "pathnames.h"

using namespace std;

/*********************************************************************
 * Globals.
 */

/* Command-line options */
static int           o_verbose   = 0;  //!< Verbose output flag.
static int           o_erroneous = 0;  //!< Include erroneous files in output.
static int           o_precise   = 0;  //!< Precise error output flag.
static int           o_trace     = 0;  //!< Trace/debug output flag.
static string        o_revdepdir = _PATH_REVDEPD;  //!< Revdep config directory.
static string        o_pkgdb     = _PATH_PKGDB;    //!< Package database file path.
static string        o_ldsoconf  = _PATH_LDSOCONF; //!< ld.so.conf file path.

static StringVector  o_IgnoredPackages; //!< Vector of ignored package names.
static PackageVector o_packages; //!< Vector of Package objects.

static StringVector  dirs;  //!< Vector of directories to search for libs.
static ElfCache      ec;    //!< ElfCache object for caching ELF files.

/*!
 * \enum _revdep_errors
 * \brief Exit status codes for revdep utility.
 */
enum _revdep_errors {
  E_INVALID_INPUT = 1, //!< Invalid command-line arguments.
  E_READ_PKGDB    = 2, //!< Failed to read package database.
  E_READ_LDSOCONF = 3, //!< Failed to read ld.so.conf.
  E_FOUND_MISSING = 4, //!< Found at least one missing library.
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

/*!
 * \brief Checks dependencies for a single file within a package.
 *
 * This function checks if a given file, belonging to a package, has
 * all its shared library dependencies satisfied.  It uses the
 * ElfCache to lookup ELF files and find required libraries.
 *
 * \param pkg  The Package object to which the file belongs.
 * \param file The path to the file to be checked.
 *
 * \return True if all dependencies are satisfied or the file is not a
 *         regular ELF file, false if any dependency is missing.
 */
static bool
workFile(const Package &pkg, const string &file)
{
  bool rv = true;  // Return value, initially true (ok)

  if (o_trace)
    cout << pkg.Name() << ":" << file << ": checking file" << endl;

  if (!IsRegularFile(file))
    return rv;  // Skip if not a regular file

  const Elf *elf = ec.LookUp(file);  // Lookup ELF in cache
  if (elf == NULL)
    return rv;                       // Skip if not an ELF file

  if (o_trace)
    cout << pkg.Name() << ":" << file << ": is ELF" << endl;

  // Check each needed library
  for (size_t i = 0; i < elf->Needed().size(); ++i)
  {
    const string &lib = elf->Needed()[i];  // Library name

    // Find library in search paths
    if (!ec.FindLibrary(elf, pkg, lib, dirs))
    {
      if (o_precise)
        cout << pkg.Name() << ":" << file << ":" << lib
             << ": missing library" << endl;

      rv = false;  // Dependency missing
    }
  }

  return rv;  // Return result of dependency check
}

/*!
 * \brief Checks dependencies for all files within a package.
 *
 * Iterates through all files belonging to a package and calls
 * workFile for each file to check its dependencies.
 *
 * \param pkg The Package object to be checked.
 *
 * \return True if all files in the package have satisfied
 *         dependencies, false if any file has missing dependencies.
 */
static bool
workPackage(const Package &pkg)
{
  bool rv = true;  // Return value, initially true (ok)

  if (pkg.Ignore())
    return rv;  // Skip ignored packages

  // Check each file in the package
  for (size_t i = 0; i < pkg.Files().size(); ++i)
  {
    const string &file = pkg.Files()[i]; // File path

    if (!workFile(pkg, file))
    {
      if (o_erroneous)
        cout << pkg.Name() << ":" << file << ": error" << endl;

      rv = false;  // Error found in a file
    }
  }

  return rv;  // Return result for the package
}

/*!
 * \brief Checks dependencies for all packages in the database.
 *
 * Iterates through all packages in the provided PackageVector and
 * calls workPackage for each package to perform the dependency check.
 *
 * \param pkgs The PackageVector containing all packages to check.
 *
 * \return 0 if all packages are ok, E_FOUND_MISSING if any package
 *         has missing dependencies.
 */
static int
workAllPackages(const PackageVector &pkgs)
{
  int rc = 0;  // Return code, initially 0 (ok)

  if (o_verbose)
  {
    printf("** checking %zu packages\n", pkgs.size());
    printf("** checking linking\n");
  }

  // Check each package
  for (size_t i = 0; i < pkgs.size(); ++i)
  {
    const Package &pkg = pkgs[i];  // Current package

    if (!workPackage(pkg))
    {
      rc = E_FOUND_MISSING;  // Set error code if missing deps

      if (o_verbose)
        cout << pkg.Name() << ": error" << endl;
      else
        cout << pkg.Name() << endl;  // Print package name on error
    }
    else
    {
      if (o_verbose)
        cout << pkg.Name() << ": ok" << endl; // Print package name on success
    }
  }

  return rc;  // Return overall result
}

/*!
 * \brief Checks dependencies for specific packages provided as
 *        command-line arguments.
 *
 * Iterates through the provided package names from command-line,
 * finds corresponding Package objects, and calls workPackage for each
 * to check dependencies.
 *
 * \param pkgs The PackageVector containing all packages.
 * \param i    Starting index in argv for package names.
 * \param argc Argument count from main.
 * \param argv Argument vector from main.
 *
 * \return 0 if all specified packages are ok, E_FOUND_MISSING if any
 *         of the specified packages has missing dependencies.
 */
static int
workSpecificPackages(const PackageVector &pkgs,
                     int                 i,
                     int                 argc,
                     char                **argv)
{
  int rc = 0;  // Return code, initially 0 (ok)

  if (o_verbose)
  {
    printf("** checking %d packages\n", argc - i);
    printf("** checking linking\n");
  }

  // Check each specified package name
  for ( ; i < argc; ++i)
  {
    const string name = argv[i];  // Package name from command line

    // Find package by name
    PackageVector::const_iterator pkg =
      find(pkgs.begin(), pkgs.end(), name);

    if (pkg == pkgs.end())
    {
      if (o_verbose)
        cout << name << ": cannot find package information" << endl;

      continue;  // Skip if package info not found
    }

    if (!workPackage(pkg[0]))
    {
      rc = E_FOUND_MISSING;  // Set error code if missing deps

      if (o_verbose)
        cout << pkg->Name() << ": error" << endl;
      else
        cout << pkg->Name() << endl;  // Print package name on error
    }
    else
    {
      if (o_verbose)
        cout << pkg->Name() << ": ok" << endl;  // Print package name on success
    }
  }

  return rc; // Return overall result
}

/*!
 * \brief Marks specified packages as ignored.
 *
 * Iterates through the list of package names to ignore and marks the
 * corresponding Package objects in the PackageVector as ignored.
 *
 * \param pkgs    The PackageVector to modify.
 * \param ignores The StringVector of package names to ignore.
 */
static void
ignorePackages(PackageVector &pkgs, const StringVector &ignores)
{
  // Iterate through packages to ignore
  for (size_t i = 0; i < ignores.size(); ++i)
  {
    // Find package by name
    PackageVector::iterator pkg =
      find(pkgs.begin(), pkgs.end(), ignores[i]);

    if (pkg == pkgs.end())
      continue;  // Skip if package not found

    pkg->Ignore();  // Mark package as ignored
  }
}

/*!
 * \brief Prints the help message for the revdep utility.
 *
 * Displays usage instructions, command-line options, and a brief
 * description of the utility.
 *
 * \return 0 indicating successful execution (help printed).
 */
static int
print_help()
{
  cout << R"(Usage: revdep [-Vehptv] [-L ldsoconffile ] [-D pkgdbfile] [-R revdepdir]
              [-I pkgname[,...]] [pkgname ...]
Check for missing libraries of installed packages.

Mandatory arguments to long options are mandatory for short options too.
  -L, --ldsoconf=ldsoconffile
                        specify an alternate location for ld.so.conf file
  -D, --pkgdb=pkgdbfile specify an alternate location for the packages database
                        file
  -R, --revdepdir=revdepdir
                        specify an alternate location for revdep's package
                        configuration directory
  -I, --ignore=pkgname[,...]
                        comma-separated list of packages to ignore
  -e, --erroneous       include erroneous files in the output
  -p, --precise         include precise file errors in the output
  -t, --trace           show debug/trace
  -v, --verbose         formatted listing
  -V, --version         print version and exit
  -h, --help            print help and exit
)";
  return 0;
}

/*!
 * \brief Prints the version information for the revdep utility.
 *
 * Displays the program name and version number.
 *
 * \return 0 indicating successful execution (version printed).
 */
static int
print_version()
{
  cout << "revdep " VERSION "\n";
  return 0;
}

/*!
 * \brief Main function for the revdep utility.
 *
 * Parses command-line arguments, reads package database and
 * configuration files, performs dependency checks, and reports
 * results.
 *
 * \param argc Argument count from command line.
 * \param argv Argument vector from command line.
 *
 * \return Exit status code indicating success (0) or failure
 *         (non-zero, see _revdep_errors enum).
 */
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

  int opt;  // Option character

  // Parse command-line options
  while ((opt =
          getopt_long(argc, argv, "L:D:R:I:eptvVh", longopts, 0))
      != -1)
  {
    switch (opt)
    {
      case 'L':
        o_ldsoconf = optarg;  // Set ld.so.conf path
        break;

      case 'D':
        o_pkgdb = optarg;  // Set package database path
        break;

      case 'R':
        o_revdepdir = optarg;  // Set revdep config dir path
        break;

      case 'I':
        split(optarg, o_IgnoredPackages, ',');  // Split ignored packages list
        break;

      case 'v':
        o_verbose = 1;  // Enable verbose output
        break;

      case 'e':
        o_erroneous = 1;  // Enable erroneous file output
        break;

      case 'p':
        o_precise = 1;  // Enable precise error output
        break;

      case 't':
        o_trace = 1;  // Enable trace output
        break;

      case 'V':
        return print_version();  // Print version and exit

      case 'h':
        return print_help();  // Print help and exit

      case ':': // missing argument
      case '?': // invalid options
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return E_INVALID_INPUT; // Return invalid input error
    }
  }

  // Read package database
  if (!ReadPackages(o_pkgdb, o_packages))
  {
    cerr << "revdep: " << o_pkgdb << ": failed to read package database\n";
    return E_READ_PKGDB;
  }

#ifdef __GLIBC__ // Do not try to read ld.so.conf on non-glibc systems
  // Read ld.so.conf configuration
  if (!ReadLdConf(o_ldsoconf, dirs, 10))
  {
    cerr << "revdep: " << o_ldsoconf << ": failed to read ld configuration\n";
    return E_READ_LDSOCONF;
  }
#endif

  // Add default lib dirs
  dirs.push_back("/lib");
  dirs.push_back("/usr/lib");

  // Read package-specific dirs
  ReadPackageDirs(o_revdepdir, o_packages);

  // Apply ignore list
  ignorePackages(o_packages, o_IgnoredPackages);

  if (o_verbose)
    cout << "** calculating deps" << endl;

  // Perform dependency checks based on arguments
  if (optind == argc)
    return workAllPackages(o_packages);
  else
    return workSpecificPackages(o_packages, optind, argc, argv);
}

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
