//! \file  main.cpp
//! \brief Command-line utility of \a revdep.
//!
//! This file implements the main function for the `revdep`
//! command-line utility.  `revdep` is designed to check for missing
//! shared library dependencies for installed packages.  It parses
//! command-line options, reads package information from a database,
//! analyzes ELF files, and reports any missing dependencies.
//!
//! \copyright See COPYING for license terms and COPYRIGHT for notices.

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

/*!
 * \brief Structure to hold command-line options.
 */
struct Options
{
  int          verbose_output;   //!< Flag for verbose output mode.
  int          erroneous_output; //!< Flag to include erroneous files in output.
  int          precise_output;   //!< Flag for precise error output mode.
  int          trace_output;     //!< Flag for debug/trace output mode.
  string       revdep_directory; //!< Path to the revdep configuration directory.
  string       package_database; //!< Path to the package database file.
  string       ldso_config;      //!< Path to the ld.so.conf file.
  StringVector ignored_packages; //!< Vector of package names to ignore.

  /*!
   * \brief Default constructor for Options.
   *
   * Initializes options with default values, including paths from
   * pathnames.h.
   */
  Options() :
    verbose_output(0),
    erroneous_output(0),
    precise_output(0),
    trace_output(0),
    revdep_directory(_PATH_REVDEPD),
    package_database(_PATH_PKGDB),
    ldso_config(_PATH_LDSOCONF),
    ignored_packages()
  {}
};

static Options       program_options;    //!< Global options structure.
static PackageVector package_list;       //!< Global vector of Package objects.
static StringVector  search_directories; //!< Global vector of directories for library search.
static ElfCache      elf_file_cache;     //!< Global ElfCache object for caching ELF files.

/*!
 * \enum ExitCode
 * \brief Exit status codes for revdep utility.
 *
 * This enumeration defines the possible exit status codes
 * for the `revdep` utility, indicating different types of
 * errors or successful execution.
 */
enum ExitCode
{
  E_INVALID_INPUT = 1, //!< Exit code for invalid command-line input.
  E_READ_PKGDB    = 2, //!< Exit code for failure to read package database.
  E_READ_LDSOCONF = 3, //!< Exit code for failure to read ld.so.conf.
  E_FOUND_MISSING = 4, //!< Exit code indicating missing shared libraries were found.
};

/*********************************************************************
 * Function declarations.
 */

static void ignorePackages(PackageVector &package_list, const StringVector &ignored_packages);
static bool workFile(const Package &package, const string &file_path);
static bool workPackage(const Package &package);
static int workAllPackages(const PackageVector &package_list);
static int workSpecificPackages(const PackageVector &package_list, int start_arg_index, int argc, char **argv);
static int parseCommandLineOptions(int argc, char **argv);
static int loadPackageDatabase();
static int loadLdConfig();
static int initializeSearchDirectories();
static int processPackages(int optind, int argc, char **argv);
static int printHelp();
static int printVersion();

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
 * \param package   The Package object to which the file belongs.
 * \param file_path The path to the file to be checked.
 *
 * \return True if all dependencies are satisfied or the file
 *         is not a regular ELF file, false if any dependency
 *         is missing.
 */

static bool
workFile(const Package &package, const string &file_path)
{
  bool return_value = true;

  if (program_options.trace_output)
    cout << package.Name() << ":" << file_path << ": checking file" << endl;

  if (!IsRegularFile(file_path))
    return return_value;  // Skip if not a regular file

  const Elf *elf_file = elf_file_cache.LookUp(file_path); // Lookup ELF in cache
  if (!elf_file)
    return return_value; // Skip if not an ELF file

  if (program_options.trace_output)
    cout << package.Name() << ":" << file_path << ": is ELF" << endl;

  // Check each needed library
  for (const auto& library_name : elf_file->Needed())
  {
    if (!elf_file_cache.FindLibrary(elf_file, package, library_name, search_directories))
    {
      if (program_options.precise_output)
      {
        cout << package.Name() << ":" << file_path << ":" << library_name
             << ": missing library" << endl;
      }
      return_value = false;  // Dependency missing
    }
  }

  return return_value;  // Return result of dependency check
}

/*!
 * \brief Checks dependencies for all files within a package.
 *
 * Iterates through all files belonging to a package and calls
 * `workFile` for each file to check its dependencies.
 *
 * \param package The Package object to be checked.
 *
 * \return True if all files in the package have satisfied
 *         dependencies, false if any file has missing dependencies.
 */
static bool
workPackage(const Package &package)
{
  if (package.Ignore()) // Skip ignored packages
    return true;

  // Check each file in the package
  bool return_value = true;
  for (const auto& file_path : package.Files())
  {
    if (!workFile(package, file_path))
    {
      if (program_options.erroneous_output)
        cout << package.Name() << ":" << file_path << ": error" << endl;

      return_value = false; // Error found in a file
    }
  }

  return return_value;
}

/*!
 * \brief Checks dependencies for all packages in the database.
 *
 * Iterates through all packages in the `package_list` and calls
 * `workPackage` for each package to perform the dependency check.
 *
 * \param package_list The PackageVector containing all packages to check.
 *
 * \return 0 if all packages are ok, E_FOUND_MISSING if any package
 *           has missing dependencies.
 */
static int
workAllPackages(const PackageVector &package_list)
{
  int return_value = 0;

  if (program_options.verbose_output)
  {
    printf("** checking %zu packages\n", package_list.size());
    printf("** checking linking\n");
  }

  // Check each package
  for (const auto& package : package_list)
  {
    if (!workPackage(package))
    {
      return_value = E_FOUND_MISSING;

      if (program_options.verbose_output)
        cout << package.Name() << ": error" << endl;
      else
        cout << package.Name() << endl;

    }
    else if (program_options.verbose_output)
      cout << package.Name() << ": ok" << endl;
  }

  return return_value;
}

/*!
 * \brief Checks dependencies for specific packages provided as
 *        command-line arguments.
 *
 * Iterates through the provided package names from command-line,
 * finds corresponding Package objects in `package_list`, and calls
 * `workPackage` for each to check dependencies.
 *
 * \param package_list    The PackageVector containing all packages.
 * \param start_arg_index Starting index in argv for package names.
 * \param argc            Argument count from main.
 * \param argv            Argument vector from main.
 *
 * \return 0 if all specified packages are ok, E_FOUND_MISSING
 *         if any of the specified packages has missing dependencies.
 */
static int
workSpecificPackages(const PackageVector &package_list,
    int start_arg_index, int argc, char **argv)
{
  int return_value = 0;

  if (program_options.verbose_output)
  {
    printf("** checking %d packages\n", argc - start_arg_index);
    printf("** checking linking\n");
  }

  // Check each specified package name
  for (int i = start_arg_index; i < argc; ++i)
  {
    const string package_name = argv[i];

    // Find package by name
    auto package_iterator = find(package_list.begin(), package_list.end(), package_name);
    if (package_iterator == package_list.end())
    {
      if (program_options.verbose_output)
        cout << package_name << ": cannot find package information" << endl;

      continue;  // Skip if package info not found
    }

    if (!workPackage(*package_iterator))
    {
      return_value = E_FOUND_MISSING;  // Set error code if missing deps

      if (program_options.verbose_output)
        cout << package_iterator->Name() << ": error" << endl;
      else
        cout << package_iterator->Name() << endl;
    }
    else if (program_options.verbose_output)
        cout << package_iterator->Name() << ": ok" << endl;
  }

  return return_value;
}

/*!
 * \brief Marks specified packages as ignored.
 *
 * Iterates through the list of package names to ignore and marks the
 * corresponding Package objects in the `package_list` as ignored.
 *
 * \param package_list     The PackageVector to modify.
 * \param ignored_packages The StringVector of package names to ignore.
 */
static void
ignorePackages(PackageVector &package_list, const StringVector &ignored_packages)
{
  // Iterate through packages to ignore
  for (const auto& ignored_package_name : ignored_packages)
  {
    // Find package by name
    auto package_iterator = find(package_list.begin(), package_list.end(), ignored_package_name);
    if (package_iterator != package_list.end())
      package_iterator->Ignore();  // Mark package as ignored
  }
}

/*!
 * \brief Prints the help message for the revdep utility.
 *
 * Displays usage instructions, command-line options, and a brief
 * description of the utility to the standard output.
 *
 * \return 0 indicating successful execution (help printed).
 */
static int
printHelp()
{
  cout << R"(Usage: revdep [-Vehptv] [-L ldso-conf-file] [-D package-db-file]
              [-R revdep-dir] [-I package-list] [package-name ...]
Check installed packages for missing shared libraries.

Mandatory arguments to long options are mandatory for short options too.
  -L, --ldsoconf=ldso-conf-file  Use an alternate library search path
                                 configuration file
  -D, --pkgdb=package-db-file    Use an alternate package database file
  -R, --revdepdir=revdep-dir     Use an alternate directory for
                                 per-package library search lists
  -I, --ignore=package-list      Ignore the listed packages
  -e, --erroneous                Include files with errors in the output
  -p, --precise                  Include precise file error details in
                                 the output
  -t, --trace                    Show trace output
  -v, --verbose                  Produce a more detailed listing
  -V, --version                  Print version and exit
  -h, --help                     Print this help and exit
)";
  return 0;
}

/*!
 * \brief Prints the version information for the revdep utility.
 *
 * Displays the program name and version number to the standard output.
 *
 * \return 0 indicating successful execution (version printed).
 */
static int
printVersion()
{
  cout << "revdep " VERSION "\n";
  return 0;
}

/*!
 * \brief Parses command line options using getopt_long.
 *
 * This function parses the command-line arguments using
 * `getopt_long`, populating the `program_options` structure with the
 * parsed values.
 *
 * \param argc Argument count from command line.
 * \param argv Argument vector from command line.
 *
 * \return 0 on successful parsing, E_INVALID_INPUT if there is an
 *           error in command-line arguments.
 */
static int
parseCommandLineOptions(int argc, char **argv)
{
  static struct option long_options[] =
  {
    { "ldsoconf",    required_argument,  nullptr,  'L' },
    { "pkgdb",       required_argument,  nullptr,  'D' },
    { "revdepdir",   required_argument,  nullptr,  'R' },
    { "ignore",      required_argument,  nullptr,  'I' },
    { "erroneous",   no_argument,        nullptr,  'e' },
    { "precise",     no_argument,        nullptr,  'p' },
    { "trace",       no_argument,        nullptr,  't' },
    { "verbose",     no_argument,        nullptr,  'v' },
    { "version",     no_argument,        nullptr,  'V' },
    { "help",        no_argument,        nullptr,  'h' },
    { nullptr,       0,                  nullptr,   0  } // Sentinel
  };

  int option_char;
  while ((option_char = getopt_long(argc, argv, "L:D:R:I:eptvVh", long_options, nullptr)) != -1)
  {
    switch (option_char)
    {
      case 'L':
        program_options.ldso_config = optarg;
        break;
      case 'D':
        program_options.package_database = optarg;
        break;
      case 'R':
        program_options.revdep_directory = optarg;
        break;
      case 'I':
        split(optarg, program_options.ignored_packages, ',');
        break;
      case 'v':
        program_options.verbose_output = 1;
        break;
      case 'e':
        program_options.erroneous_output = 1;
        break;
      case 'p':
        program_options.precise_output = 1;
        break;
      case 't':
        program_options.trace_output = 1;
        break;
      case 'V':
        printVersion();
        exit(0);
      case 'h':
        printHelp();
        exit(0);
      case ':': /* missing argument */
      case '?': /* invalid options */
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return E_INVALID_INPUT;
    }
  }

  return 0; // Command line options parsed successfully
}

/*!
 * \brief Loads the package database.
 *
 * Reads package information from the database file specified in
 * `program_options.package_database`.
 *
 * \return 0 on success, E_READ_PKGDB on error if the database
 *         cannot be read.
 */
static int
loadPackageDatabase()
{
  if (!ReadPackages(program_options.package_database, package_list))
  {
    cerr << "revdep: " << program_options.package_database << ": failed to read package database\n";
    return E_READ_PKGDB;  // Indicate package database read error
  }

  return 0;
}

/*!
 * \brief Loads the ld.so.conf configuration.
 *
 * Reads `ld.so.conf` configuration file from the path specified in
 * `program_options.ldso_config` and populates the
 * `search_directories` vector.
 *
 * \return 0 on success, E_READ_LDSOCONF on error if the configuration
 *           cannot be read.
 */
static int
loadLdConfig()
{
#ifdef __GLIBC__ // Do not try to read ld.so.conf on non-glibc systems
  if (!ReadLdConf(program_options.ldso_config, search_directories, 10))
  {
    cerr << "revdep: " << program_options.ldso_config << ": failed to read ld configuration\n";
    return E_READ_LDSOCONF; // Indicate ld.so.conf read error
  }
#endif
  return 0;
}

/*!
 * \brief Initializes the default search directories.
 *
 * Adds default library directories ("/lib", "/usr/lib") to the
 * `search_directories` vector and reads package-specific directories
 * from the revdep configuration directory.  Also applies the list of
 * ignored packages.
 *
 * \return 0 on success.
 */
static int
initializeSearchDirectories()
{
  search_directories.push_back("/lib");
  search_directories.push_back("/usr/lib");
  ReadPackageDirs(program_options.revdep_directory, package_list);
  ignorePackages(package_list, program_options.ignored_packages);
  return 0;
}

/*!
 * \brief Processes packages based on command line arguments.
 *
 * Checks dependencies for all packages or specific packages based on
 * whether package names are provided as command-line arguments.
 *
 * \param optind Index of the first package argument in argv.
 * \param argc   Argument count from main.
 * \param argv   Argument vector from main.
 *
 * \return Exit status code indicating success or failure
 *         (see `ExitCode` enum).
 */
static int
processPackages(int optind, int argc, char **argv)
{
  if (program_options.verbose_output)
    cout << "** calculating deps" << endl;

  if (optind == argc)
  {
    // Check all packages if no names provided
    return workAllPackages(package_list);
  }
  else
  {
    // Check specific packages
    return workSpecificPackages(package_list, optind, argc, argv);
  }
}

/*!
 * \brief Main function for the revdep utility.
 *
 * The entry point of the `revdep` utility. It orchestrates the
 * parsing of command-line options, loading of package database and
 * configurations, initialization of search directories, and execution
 * of dependency checks.
 *
 * \param argc Argument count from command line.
 * \param argv Argument vector from command line.
 *
 * \return Exit status code indicating success (0) or failure
 *         (non-zero, see `ExitCode` enum).
 */
int
main(int argc, char **argv)
{
  if (int error_code = parseCommandLineOptions(argc, argv)) return error_code;
  if (int error_code = loadPackageDatabase()) return error_code;
  if (int error_code = loadLdConfig()) return error_code;
  if (int error_code = initializeSearchDirectories()) return error_code;

  return processPackages(optind, argc, argv);
}

// vim: sw=2 ts=2 sts=2 et cc=72 tw=70
// End of file.
