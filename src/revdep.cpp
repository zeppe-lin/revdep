/*!
 * \file  revdep.cpp
 * \brief revdep(1) command line interface, implemented as an adapter
 *        over librevdep.
 *
 * This file intentionally contains **no dependency-resolution logic**.
 * Its job is orchestration.
 *
 * - Parse CLI options and map them to librevdep knobs.
 * - Load the installed package database.
 * - Load global resolver directories from ld.so.conf.
 * - Build a stable work list (package order, then file order).
 * - Delegate auditing and scheduling to librevdep:
 *   - Serial path: revdep::RevdepAuditFile()
 *   - Parallel path: revdep::RevdepAuditWorkItemsParallel()
 *
 * Keeping the CLI thin prevents accidental global state and ensures
 * that parallel execution remains correct.  Deterministic output
 * under parallel execution is controlled via --order.
 *
 * \copyright See COPYING for license terms and COPYRIGHT for notices.
 */

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "librevdep.h"
#include "revdep-config.h"

using namespace std;
using std::string;

/*!
 * \brief Structure to hold command-line options.
 */
struct Options
{
  // Flag for verbose output mode.  Default: 0.
  int verbose_output{0};

  // Flag to include erroneous files in output.  Default: 0.
  int erroneous_output{0};

  // Flag for precise error output mode.  Default: 0.
  int precise_output{0};

  // Flag for debug/trace output mode.  Default: 0.
  int trace_output{0};

  // Number of jobs for parallel execution.  Default: 1.
  size_t jobs{1};

  // Serialize sink invocation under parallel execution.
  // Default: true.
  bool serialize_sink{true};

  // Control emission order of findings under parallel execution.
  // Default: input - stable output in the same order as the input
  // work list - deterministic.
  revdep::RevdepEmitOrder order{revdep::RevdepEmitOrder::InputStable};

  // Path to the revdep configuration directory.
  string revdep_directory{PATH_REVDEPD};

  // Path to the package database file.
  string package_database{PATH_PKGDB};

  // Path to the ld.so.conf file.
  string ldso_config{PATH_LDSOCONF};

  // Vector of package names to ignore.
  StringVector ignored_packages;
};

static Options       program_options;    //!< Global options structure.
static PackageVector package_list;       //!< Global vector of Package objects.
static StringVector  search_directories; //!< Global vector of directories for library search.

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

/*!
 * \brief Marks specified packages as ignored.
 *
 * Iterates through the list of package names to ignore and marks the
 * corresponding Package objects in the `pkgs` as ignored.
 *
 * \param pkgs     The PackageVector to modify.
 * \param ignored  The StringVector of package names to ignore.
 */
static void
ignorePackages(PackageVector &pkgs, const StringVector &ignored)
{
  for (const auto& n : ignored)
  {
    for (auto& p : pkgs)
    {
      if (p == n)
        p.Ignore();
    }
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
              [-R revdep-dir] [-I package-list]
              [-j jobs] [--order mode] [no-serialize-sink]
              [package-name ...]
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
  -j, --jobs=N                   Work in parallel using N threads
      --order=mode               Control emission order under parallel
                                 execution
      --no-serialize-sink        Do not serialize sink invocation
                                 under parallel execution
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
  cout << PACKAGE_NAME << " " << PACKAGE_VERSION << endl;
  return 0;
}

/*!
 * \brief Parse the jobs argument into a numeric value.
 *
 * Converts the string argument from the command line into a positive
 * integer representing the number of concurrent jobs.
 *
 * - If \p optarg is null or empty, the number of jobs defaults to the
 *   hardware concurrency reported by
 *   std::thread::hardware_concurrency(), or 1 if unavailable.
 * - If \t optarg contains a valid non-zero integer, that value is
 *   stored in \p out.
 * - If parsing fails (non-numeric characters, zero value), the
 *   function returns false.
 *
 * \param optarg   The command-line argument string to parse.
 * \param[out] out Reference to store the resulting job count.
 *
 * \return True if parsing succeeded and \p out was set;
 *         False if the argument was invalid.
 */
static bool
parseJobsArg(const char* optarg, size_t& out)
{
  if (!optarg || *optarg == '\0')
  {
    unsigned hc = std::thread::hardware_concurrency();
    out = hc ? static_cast<size_t>(hc) : 1u;
    return true;
  }
  char* end = nullptr;
  unsigned long v = std::strtoul(optarg, &end, 10);
  if (!end || *end != '\0' || v == 0)
    return false;
  out = static_cast<size_t>(v);
  return true;
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
  enum {
    OPT_ORDER = 1000,
    OPT_NO_SERIALIZE_SINK,
  };

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
    { "jobs",        optional_argument,  nullptr,  'j' },
    { "order",       required_argument,  nullptr,  OPT_ORDER },
    { "no-serialize-sink", no_argument,  nullptr,  OPT_NO_SERIALIZE_SINK },
    { "version",     no_argument,        nullptr,  'V' },
    { "help",        no_argument,        nullptr,  'h' },
    { nullptr,       0,                  nullptr,   0  } // Sentinel
  };

  // "j::" => optional arg for -j.
  int option_char;
  while ((option_char = getopt_long(argc, argv, "L:D:R:I:eptvj::Vh", long_options, nullptr)) != -1)
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
      case 'j':
        if (!parseJobsArg(optarg, program_options.jobs))
        {
          fprintf(stderr, "revdep: invalid -j/--jobs value\n");
          return E_INVALID_INPUT;
        }
        break;
      case OPT_ORDER:
        if (std::strcmp(optarg, "input") == 0)
        {
          program_options.order = revdep::RevdepEmitOrder::InputStable;
        }
        else if (std::strcmp(optarg, "unordered") == 0)
        {
          program_options.order = revdep::RevdepEmitOrder::Unordered;
        }
        else
        {
          cerr << "revdep: invalid --order mode (use: input|unordered)\n";
          return E_INVALID_INPUT;
        }
        break;
      case OPT_NO_SERIALIZE_SINK:
        program_options.serialize_sink = false;
        break;
      case ':': /* missing argument */
      case '?': /* invalid options */
      default:
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        return E_INVALID_INPUT;
    }
  }
  if (program_options.jobs == 0)
    program_options.jobs = 1;
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

static vector<const Package*>
selectPackages(int optind, int argc, char** argv)
{
  vector<const Package*> out;
  if (optind == argc)
  {
    out.reserve(package_list.size());
    for (const auto& p : package_list)
      out.push_back(&p);
    return out;
  }

  // Preserve argv order.
  for (int i = optind; i < argc; ++i)
  {
    string name = argv[i];
    auto it = std::find_if(package_list.begin(), package_list.end(),
        [&](const Package& p){ return p == name; });
    if (it != package_list.end())
      out.push_back(&*it);
    else
      cerr << "revdep: " << name << ": package not found\n";
  }
  return out;
}

static int runAudit(const vector<const Package*>& pkgs)
{
  if (program_options.verbose_output)
  {
    printf("** checking %zu packages\n", pkgs.size());
    printf("** checking linking\n");
  }

  revdep::RevdepConfig cfg;
  cfg.global_search_dirs.assign(search_directories.begin(), search_directories.end());
  revdep::RevdepContext ctx(std::move(cfg));

  // Build work list in deterministic order (pkg order, file order).
  vector<revdep::RevdepWorkItem> items;
  for (const Package* p : pkgs)
  {
    if (!p || p->Ignore())
      continue;
    for (const auto& f : p->Files())
      items.push_back({p, f});
  }

  // Per-package status tracking.
  unordered_map<string, bool> pkg_bad;
  unordered_set<string> file_bad; // key: pkg\npath
  mutex track_mx;

  auto sink = [&](const revdep::RevdepFinding& f) {
    if (program_options.trace_output)
    {
      // Trace is serialized by parallel runner (serialize_sink=true).
      cout << f.package << ":" << f.object_path << ": missing " << f.needed << "\n";
    }

    {
      lock_guard<mutex> lk(track_mx);
      pkg_bad[f.package] = true;
      file_bad.insert(f.package + "\n" + f.object_path);
    }

    if (program_options.precise_output)
    {
      cout << revdep::RevdepFormatFinding(f) << "\n";
    }
  };

  revdep::RevdepParallelOptions opt;
  opt.thread_count = program_options.jobs;
  opt.order = program_options.order;
  opt.serialize_sink = program_options.serialize_sink;

  bool ok = true;
  if (program_options.jobs <= 1)
  {
    // Serial execution through library engine.
    for (const Package* p : pkgs)
    {
      if (!p || p->Ignore())
        continue;
      if (!revdep::RevdepAuditPackage(*p, ctx, sink))
        ok = false;
    }
  }
  else
  {
    ok = revdep::RevdepAuditWorkItemsParallel(items, ctx, opt, sink);
  }

  int ret = ok ? 0 : E_FOUND_MISSING;

  // Summary output matches historical behavior.
  for (const Package* p : pkgs)
  {
    if (!p || p->Ignore())
      continue;

    bool bad = false;
    {
      lock_guard<mutex> lk(track_mx);
      auto it = pkg_bad.find(p->Name());
      bad = (it != pkg_bad.end() && it->second);
    }

    if (bad)
    {
      if (program_options.erroneous_output &&
          !program_options.precise_output)
      {
        // In old CLI, -e printed per-file "error" lines.
        for (const auto& f : p->Files())
        {
          bool fb = false;
          {
            lock_guard<mutex> lk(track_mx);
            fb = file_bad.count(p->Name() + "\n" + f) != 0;
          }
          if (fb)
          {
            cout << p->Name() << ":" << f << ": error\n";
          }
        }
      }

      if (program_options.verbose_output)
      {
        cout << p->Name() << ": error\n";
      }
      else if (!program_options.precise_output &&
          !program_options.erroneous_output)
      {
        cout << p->Name() << "\n";
      }
    }
    else if (program_options.verbose_output)
    {
      cout << p->Name() << ": ok\n";
    }
  }

  return ret;
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

  auto pkgs = selectPackages(optind, argc, argv);
  return runAudit(pkgs);
}
