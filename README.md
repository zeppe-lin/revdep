OVERVIEW
--------
This directory contains revdep, a package management tool that
checks for missing libraries of installed packages.

This distribution is a fork of CRUX' revdep utility (which is
part of CRUX' prt-utils distribution) as of commit 41dfcb6 (Thu
Oct 15 2020) with the following differences:
- fix GCC extensions for portability
- manual page in mdoc(7) format
- command-line options/usage in GNU style
- different exit codes for ease scripting
- bash completion
- powerpc{,64} and risc-v elf support

See git log for complete/further differences.

The original sources can be downloaded from:
1. git://crux.nu/tools/prt-utils.git                        (git)
2. https://crux.nu/gitweb/?p=tools/prt-utils.git;a=summary  (web)


REQUIREMENTS
------------
**Build time**:
- C++11 compiler
- POSIX sh(1p) and "mandatory utilities"
- GNU make(1)
- pkg-config(1)
- elfutils


INSTALL
-------
The shell commands `make && make install` should build and
install this package.
The shell command `make install_bashcomp` should install bash
completion script.

See `config.mk` file for configuration parameters, and
`src/pathnames.h` for absolute filenames that revdep wants for
various defaults.


LICENSE
-------
revdep is licensed through the GNU General Public License v3
or later <https://gnu.org/licenses/gpl.html>.
Read the COPYING file for copying conditions.
Read the COPYRIGHT file for copyright notices.
