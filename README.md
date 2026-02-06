OVERVIEW
========

`revdep` checks installed packages for missing shared library
dependencies.

This distribution is a fork of CRUX `revdep` (part of `prt-utils`) at
commit `41dfcb6` (Thu Oct 15 2020), with the following differences:
  * Fixed GCC extensions for portability
  * Manual pages in `scdoc(5)` format
    * Split into `revdep(1)` and `revdep.d(5)`
  * GNU-style options, help, and usage output
  * Distinct exit codes for easier scripting
  * Bash completion support
  * Extended ELF support for powerpc{,64}, loongarch{,64} and RISC-V

See the git log for full history.

Original sources:
  * https://git.crux.nu/tools/prt-utils.git

---

REQUIREMENTS
============

Build-time
----------
  * C++11 compiler
  * POSIX `sh(1p)`, `make(1p)`, and "mandatory utilities"
  * `elfutils`
  * `scdoc(1)` to generate manual pages
  * `pkg-config(1)` (optional, for static linking)

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
Default paths are specified in `src/pathnames.h`.

---

DOCUMENTATION
=============

Manual pages are provided in `/man` and installed under the system
manual hierarchy.

---

LICENSE
=======

`revdep` is licensed under the
[GNU General Public License v3 or later](https://gnu.org/licenses/gpl.html).

See `COPYING` for license terms and `COPYRIGHT` for notices.
