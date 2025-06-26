TODO revdep
===========


Next release
------------

- [ ] Fix compilation warnings with `extra/flags-extra.mk`

No milestone
------------

- [ ] `completion/bash_completion`: fix shellcheck warnings, add
  checking to CI.
- [ ] Code clean up: `grep 'XXX\|FIXME\|TODO' --exclude-dir=.git -R .`
- [ ] Split `revdep` into `librevdep(3)` and `revdep(1)`?
- [ ] Add support for `NODEFLIB`?
- [ ] Add support for `LD_LIBRARY_PATH`?
- [ ] Add support for hardware capability directories?

Done
----

- [x] Add `revdep.d(5)` manual page
- [?] Clean up the mess with "debug" flags and reflect in the README
  how to build the "debug" binary.
  - [x] Splitted debug and release flags into different files
