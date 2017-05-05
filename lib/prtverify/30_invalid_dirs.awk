#
# 30_invalid_dirs.awk
#
# Version 0.1.4 - 2017-05-05
# Juergen Daubert <jue at jue dot li>


BEGIN {

    invalid_dirs[1] = "^usr/man/$"
    invalid_dirs[2] = "^usr/local/$"
    invalid_dirs[3] = "^usr/share/locale/$"
    invalid_dirs[4] = "^usr/info/$"
    invalid_dirs[5] = "^usr/share/info/$"
    invalid_dirs[6] = "^usr/libexec/$"
    invalid_dirs[7] = "^usr/man/../$"
    invalid_dirs[8] = "^usr/share/man/../$"
}


loglevel_ok(ERROR) && FILENAME ~ FOOTPRINT {

    for (d in invalid_dirs) {
        if ($3 ~ invalid_dirs[d])
            perror(ERROR, "directory not allowed: " $3)
    }
}

