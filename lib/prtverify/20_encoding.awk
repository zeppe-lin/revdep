#
# 20_encoding.awk
#
# Version 0.1 - 2009-03-09
# Jürgen Daubert <jue at jue dot li>
#
# Detect non-ASCII characters
#


loglevel_ok(WARN) && FILENAME ~ PKGFILE {

    if ($0 ~ /[^\001-\177]/) {
        perror(WARN, "non-ASCII character found, Pkgfile line " NR)
    }
}

