OVERVIEW
========

`revdep` checks installed packages for missing shared library
dependencies.

It is a fork of CRUX' `revdep` (part of `prt-utils`) at commit
`41dfcb6` (Thu Oct 15 2020), with the following changes:
  * Fix GCC extensions for portability
  * Manual pages converted to `scdoc(7)`
  * Split into `revdep(1)` and `revdep.d(5)`
  * GNU-style options, help, and usage output
  * Distinct exit codes for easier scripting
  * Bash completion
  * Support for powerpc{,64}, loongarch{,64} and risc-v ELF

See git log for the full history.

Original sources: https://git.crux.nu/tools/prt-utils.git


REQUIREMENTS
============

Build-time
----------
  * C++11 compiler
  * POSIX `sh(1p)`, `make(1p)` and "mandatory utilities"
  * `elfutils`
  * `scdoc(1)` for building manual pages
  * `pkg-config(1)` (optional, for static linking)


INSTALLATION
============

To build and install this package, run:

    make && make install

For static linking (requires `pkg-config(1)`):

    make LDFLAGS="-static $(pkg-config --static --libs libelf)"

Configuration parameters are in `config.mk`.

Default paths are defined in `src/pathnames.h`.


DOCUMENTATION
=============

Manual pages are in `/man`.


LICENSE
=======

`revdep` is licensed through the GNU General Public License v3 or
later <https://gnu.org/licenses/gpl.html>.
See `COPYING` for license terms and `COPYRIGHT` for notices.
