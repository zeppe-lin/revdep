revdep(1) "version #VERSION#" "Package Manager Manual"

# NAME

revdep - checks for missing libraries of installed packages

# SYNOPSIS

*revdep* [*-d*] [*-c*] [*-r*] [*-i*] [*-v* | *-vv* | *-vvv* | *-vvvv*] [_PKGNAME_]

*revdep* [*-h*]

# DESCRIPTION

*revdep* checks for missing libraries of installed packages using elfutils'
libelf and the same algorithm that *ld.so*(8) uses.

# OPTIONS

*-h*
	Display this help and exit.

*-d*
	Specify an alternate location for the package database.

*-c*
	Specify an alternate location for _ld.so.conf_.

*-r*
	Specify an alternate directory for *revdep* package config.

*-i*
	Comma-separated list of packages to ignore.

*-v*
	Formatted listing.

*-vv*
	Include erroneous files.

*-vvv*
	Include precise file errors.

*-vvvv*
	Show debug/trace.

# EXAMPLE

Try this command to list broken packages:

```
$ revdep
```

Use this command to rebuild packages on your system:

```
# pkgman update -fr $(revdep)
```

# ADDITIONAL LIBRARY DIRECTORIES

Upon startup, *revdep* searches the _/etc/revdep.d_ directory for files named
after packages. If any are found, any directories (assumed to be absolute
paths and delimited by newlines) within the file are added to a list of
package specific directories to search if a library dependency is not found
in one of the many standard locations (as specified by *ld.so*(8)).

If a package is discovered to have library dependencies that are in a
non-standard location, then a file, as specified above, should be created.

For example, if the package "firefox" has a "libxul.so" dependency in a
the non-standard directory "/usr/lib/firefox" then a file named "firefox"
should be created under "/etc/revdep.d" with a single line containing
"/usr/lib/firefox". This will tell *revdep* where to find firefox's "libxul.so"
when looking for its library dependencies, reducing one source of false
positives. Ideally, this file would be included with the package's own packaging
as well.

# EXIT STATUS

[[ 0
:< There are no missing libraries found. All is ok.
|  1
:  Failed to parse command-line arguments.
|  2
:  Failed to read package database.
|  3
:  Failed to read ld conf.
|  4
:  Found at least one missing library.

# NOTES

C++ rewrite inspired by Johannes Winkelmann's <jw@tks6.net> shell *revdep*.

Contributed to design Danny Rawlins <contact@romster.me>.

Adjusted for zeppelin by Alexandr Savca <alexandr.savca89@gmail.com>.

# SEE ALSO

*ld.so*(8), *pkgman*(8), *pkginfo*(8)

# AUTHOR

James Buren, ryuo @ freenode

; End of file.
