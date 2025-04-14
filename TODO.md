TODO revdep
===========


Next release
------------

- [ ] fix compilation warnings with `extra/flags-extra.mk`
- [ ] `completion/bash_completion`: fix shellcheck warnings, add
  checking to CI.
- [ ] code clean up: `grep 'XXX\|FIXME\|TODO' --exclude-dir=.git -R .`

No milestone
------------

- [ ] Add support for `NODEFLIB`?
- [ ] Add support for `LD_LIBRARY_PATH`?
- [ ] Add support for hardware capability directories?

Done
----

- [x] add revdep.d(5) manual page
- [?] Clean up the mess with "debug" flags and reflect in the README
  how to build the "debug" binary.
  - [x] splitted debug and release flags into different files
