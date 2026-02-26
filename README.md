OVERVIEW
========

`revdep` audits installed packages for missing shared library
dependencies.

The project now consists of:
- `revdep(1)` - command-line interface
- `librevdep` - reusable C++17 library implementing the audit engine
- Manual pages documenting CLI, configuration, semantics, and API

The resolver models the dynamic loader search semantics (DT_NEEDED,
RUNPATH, RPATH, and configurable search directories) and reports
unresolved dependencies per file and per package.

The engine can be used synchronously or via an optional parallel
scheduler.

This distribution originated as a fork of CRUX `revdep`
(part of `prt-utils`) at commit `41dfcb6` (Thu Oct 15 2020), and has
since been substantially refactored.

Major differences from the original CRUX version include:
- Rewritten as a C++17 library (`librevdep`)
- Clear separation of engine, context, formatting, and parallel
  scheduler
- Parallel audit support
- GNU-style CLI and consistent exit codes
- Extended ELF support (powerpc{,64}, loongarch{,64}, RISC-V)
- Manual pages in `scdoc(5)` format:
  - `revdep(1)`
  - `revdep.d(5)`
  - `revdep_semantics(7)`
  - `librevdep(3)` and related 3-section pages
  - `librevdep(7)`

See the git history for detailed changes.

Original sources:  
https://git.crux.nu/tools/prt-utils.git

---

ARCHITECTURE
============

The system is layered:

CLI (`revdep(1)`)  
Parses options, reads package database, invokes engine.

Engine (`revdep_engine`)  
Audits ELF objects and emits structured findings.

Context (`revdep_context`)  
Holds configuration and shared state (including ELF cache).

ELF layer (`elf`, `elf_cache`)  
Parses ELF objects and performs loader-like resolution.

Formatting (`revdep_format`)  
Converts findings into stable textual representations.

Parallel scheduler (`revdep_parallel`)  
Optional concurrency layer for large audits.

The CLI is a thin frontend over `librevdep`.

---

NON-GOALS
=========

`revdep` is not a dynamic loader implementation.

Specifically:

- It does not use or parse `/etc/ld.so.cache`.
- It does not implement full glibc `ld.so` behavior.
- It does not execute binaries.
- It does not resolve `dlopen(3)` at runtime.
- It does not validate symbol-level resolution.
- It does not perform ABI compatibility analysis.

The tool focuses strictly on static DT_NEEDED dependency resolution
using documented search rules (see `revdep_semantics(7)`).

---

EMBEDDING librevdep
===================

`librevdep` provides a C++ API for programmatic audits.

Minimal example:

```cpp
#include <librevdep/librevdep.h>
#include <iostream>

int main() {
    RevdepConfig cfg;
    cfg.searchDirs = {"/lib", "/usr/lib"};

    RevdepContext ctx(cfg);

    auto sink = [](const RevdepFinding& f) {
        std::cout << RevdepFormatFinding(f) << "\n";
    };

    Package pkg;
    pkg.name = "example";
    pkg.files = {"/usr/bin/example"};

    RevdepAuditPackage(pkg, ctx, sink);
}
```

The engine is sink-based: findings are delivered incrementally
via a callback.  No I/O is performed by the library.

Parallel scheduling is available via `RevdepAuditWorkItemsParallel()`
(see `revdep_parallel(3)`).

---

ABI AND API STABILITY
=====================

The `librevdep` API is considered source-stable within a major
version.

ABI stability is **not** guaranteed across major releases.

Consumers are encouraged to:

- Link dynamically when possible.
- Rebuild against new releases.
- Consult `librevdep(3)` for authoritative API documentation.

The CLI interface (`revdep(1)`) is considered stable across minor
releases unless otherwise noted in the changelog.

---

REQUIREMENTS
============

Build-time
----------

- C++17 compiler
- POSIX `sh(1p)`, `make(1p)`, and "mandatory utilities"
- `elfutils` (`libelf`)
- `scdoc(1)` to generate manual pages
- `pkg-config(1)` (optional, for static linking)

Runtime
-------

- ELF-based system
- Package database in expected format (see `revdep(1)`)

---

INSTALLATION
============

To build and install:

```sh
make
make install   # as root
```

For static linking (requires `pkg-config(1)`):

```sh
make LDFLAGS="-static $(pkg-config --static --libs libelf)"
```

Configuration parameters are defined in `config.mk`.  
Default filesystem paths are specified in `src/pathnames.h`.

---

DOCUMENTATION
=============

Manual pages are provided in `/man` and installed under the system
manual hierarchy.

Key entry points:

- `revdep(1)` - CLI usage
- `revdep.d(5)` - configuration directory
- `revdep_semantics(7)` - resolver rules
- `librevdep(3)` - public C++ API
- `librevdep(7)` - library overview

---

LICENSE
=======

`revdep` and `librevdep` are licensed under the
[GNU General Public License v3 or later](https://gnu.org/licenses/gpl.html).

See `COPYING` for license terms and `COPYRIGHT` for notices.
