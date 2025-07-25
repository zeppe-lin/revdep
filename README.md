OVERVIEW
========

This repository contains `revdep`, a package management tool that
checks for missing libraries of installed packages.

This distribution is a fork of CRUX' `revdep` utility (which is part of
CRUX' `prt-utils` distribution) as of commit 41dfcb6 (Thu Oct 15 2020)
with the following differences:
  * fix GCC extensions for portability
  * manual page in `scdoc(7)` format
  * split `revdep(1)` manual page into `revdep(1)` and `revdep.d(5)`
  * GNU-style options/help/usage
  * different exit codes for ease scripting
  * bash completion
  * powerpc{,64}, loongarch{,64} and risc-v elf support

See git log for complete/further differences.

The original sources can be downloaded from:
  * https://git.crux.nu/tools/prt-utils.git


REQUIREMENTS
============

Build time
----------
  * C++11 compiler
  * POSIX `sh(1p)`, `make(1p)` and "mandatory utilities"
  * `pkg-config(1)` is optional, for static linking
  * `elfutils`
  * `scdoc(1)` to build manual pages


INSTALL
=======

To build and install this package, run:

    make && make install

For static linking you need `pkg-config(1)` and run `make(1p)` as the
following:

    make LDFLAGS="-static $(pkg-config --static --libs libelf)"

See `config.mk` file for configuration parameters, and
`src/pathnames.h` for absolute filenames that `revdep` wants for
various defaults.


DOCUMENTATION
=============

See `/man` directory for manual pages.


LICENSE
=======

`revdep` is licensed through the GNU General Public License v3 or
later <https://gnu.org/licenses/gpl.html>.
Read the COPYING file for copying conditions.
Read the COPYRIGHT file for copyright notices.
