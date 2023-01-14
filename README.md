ABOUT
-----
This directory contains *revdep*, a package management tool that
checks for missing libraries of installed packages.

This *revdep* distribution is a fork of CRUX' *revdep* utility (which
is part of CRUX' *prt-utils* distribution) as of commit 41dfcb6
(Thu Oct 15 2020) with the following differences:

  * the man page have been rewritten in POD format
  * command-line arguments have been rewritten and adjusted to GNU
    standards
  * added exit codes for ease scripting
  * added bash completion

See git log for further differences.

The original sources can be downloaded from:
  1. git://crux.nu/tools/prt-utils.git                        (git)
  2. https://crux.nu/gitweb/?p=tools/prt-utils.git;a=summary  (web)

REQUIREMENTS
------------
Built time:
  * c99 compiler
  * POSIX sh(1p), make(1p) and "mandatory utilities"
  * pod2man(1pm) to build man page
  * elfutils

Tests:
  * podchecker(1pm) to check PODs for errors
  * curl(1) to check URLs for response code

INSTALL
-------
The shell commands `make && make install` should build and install
this package.  See *config.mk* file for configuration parameters.

The shell command `make check` should start some tests.

LICENSE
-------
*revdep* is licensed through the GNU General Public License v3 or
later <https://gnu.org/licenses/gpl.html>.
Read the *COPYING* file for copying conditions.
Read the *COPYRIGHT* file for copyright notices.

<!-- vim:sw=2:ts=2:sts=2:et:cc=72:tw=70
End of file. -->
