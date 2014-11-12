#
# 20_duplicated_deps.awk
#
# Version 0.1 - 2014-11-12
# Juergen Daubert <jue at jue dot li>
#
# Test if we have the same dependency more than once


loglevel_ok(WARN) && FILENAME ~ PKGFILE {

    if ( $0 ~ ("^# Depends on:") ) {

        split($0, ac, /:[[:space:]]*/)
        split(ac[2], ad, /[[:space:]]*,[[:space:]]*|[[:space:]]+/)

        for (d in ad) {
	    if (ad[d] in ae) {
                perror(WARN, "duplicated dependency: " ad[d])
            } else {
                ae[ad[d]]
            }
        }
    }
}

