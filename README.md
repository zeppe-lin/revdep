OVERVIEW
========

`revdep` audits installed packages for missing shared library
dependencies of ELF-based systems.

It is a command-line frontend built on top of `librevdep`, a reusable
C++17 audit library extracted from the older coupled implementation.

This repository contains:
- `revdep(1)` - command-line interface
- Manual pages documenting CLI behavior and configuration.

The audit engine itself now lives in a separate repository:

https://github.com/zeppe-lin/librevdep

`revdep` models static shared-library dependency resolution using
loader-like search rules (`DT_NEEDED`, `RUNPATH`, `RPATH`, and
configured search directories) and reports unresolved dependencies per
file and per package.

The CLI is responsible for:
- option parsing
- package database interpretation (*FIXME: not yet*)
- configuration loading
- output formatting and reporting policy
  (*FIXME: semantics leaked into library*)

The underlying engine is responsible for ELF parsing and dependency
resolution.

This distribution originated as fork of CRUX `revdep`
(part of `prt-utils`) at commit `41dfcb6` (Thu Oct 15 2020), and has
since been substantially refactored.

Major differences from the historical CRUX version include:
- extracted reusable code into library (`librevdep`)
- clearer separation between CLI policy and audit engine
- optional parallel audit support through the library backend
- GNU-style CLI and consistent exit codes
- Extended ELF support (powerpc{,64}, loongarch{,64}, RISC-V)
- Manual pages in `scdoc(5)` format: `revdep(1)`, `revdep.d(5)`

See the git history for detailed changes.

Original sources:

https://git.crux.nu/tools/prt-utils.git

---

ARCHITECTURE
============

The system is split into frontend and engine:

CLI (`revdep(1)`)  
Parses options, reads configuration, interprets package database state
(*FIXME: not yet*), invokes the audit engine, and renders output.

Engine (`librevdep`)  
Resolves ELF dependencies and emits structured findings.

---

NON-GOALS
=========

`revdep` is not a dynamic loader.

Specifically:

- It does not use or parse `/etc/ld.so.cache`.
- It does not implement full glibc `ld.so` behavior.
- It does not execute binaries.
- It does not resolve `dlopen(3)` at runtime.
- It does not validate symbol-level resolution.
- It does not perform ABI compatibility analysis.

The tool focuses strictly on static `DT_NEEDED` dependency resolution
using documented search rules.

See `revdep_semantics(7)` of `librevdep` project for the normative
resolution contract.

---

REQUIREMENTS
============

Build-time
----------

- C++17 compiler
- Meson
- Ninja
- `elfutils` (`libelf`)
- `librevdep`
- `scdoc(1)` to generate manual pages (if manpage build is enabled)
- `pkg-config(1)` for dependency discovery

Runtime
-------

- ELF-based system
- Package database in expected format
- `librevdep` shared library (when dynamically linked)

---

INSTALLATION
============

Configure and build with Meson:

```sh
meson setup build \
    --buildtype=plain \
    --wrap-mode=nodownload \

ninja -C build
```

Install:

```sh
DESTDIR="$PKG" ninja -C build install
```

Common options:

```sh
meson setup build \
    --prefix=/usr \
    -D build_man=true \
    -D b_lto=false \
```

Use `meson configure build` to inspect available options.

---

CONFIGURATION
=============

`revdep` reads package-specific (*FIXME: not yet*) and local
configuration from `revdep.d(5)` inputs.

The exact semantics of configuration and dependency search are
documented in:
- `revdep.d(5)`
- `revdep_semantics(7)` (part of `librevdep` project)

Some compiled-in defaults may also be provided at build time for:
- package database location
- dynamic loader configuration location
- configuration directory location

These are defaults, not hard laws.

---

DOCUMENTATION
=============

Manual pages are provided in `/man` and installed under the system
manual hierarchy.

Key entry points:
- `revdep(1)` - command-line usage
- `revdep.d(5)` - configuration directory

For the underlying engine and embedding API, see the separate
`librevdep` project and its manual pages.

---

LICENSE
=======

`revdep` is licensed under the
[GNU General Public License v3 or later](https://gnu.org/licenses/gpl.html).

See `COPYING` for license terms and `COPYRIGHT` for notices.
