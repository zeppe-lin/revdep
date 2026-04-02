project(
  'revdep',
  'cpp',
  version: '6.3',
  meson_version: '>=1.2.0',
  default_options: [
    'cpp_std=c++17',
    'warning_level=3',
    'b_ndebug=if-release',
    'b_lto=false',
  ],
)

want_static = get_option('link_mode') == 'static'

librevdep_dep = dependency(
  'librevdep',
  required: true,
  static: want_static,
  fallback: ['librevdep', 'librevdep_dep'],
)

sysconfdir = get_option('sysconfdir')
localstatedir = get_option('localstatedir')

revdepd_dir = get_option('revdepd_dir') == '' ? sysconfdir / 'revdep.d' : get_option('revdepd_dir')
pkgdb_path = get_option('pkgdb_path')  == '' ? localstatedir / 'lib/pkg/db' : get_option('pkgdb_path')
ldsoconf_path = get_option('ldsoconf_path')

config_inc = include_directories('.')

subdir('internal')
subdir('src')
subdir('man')
subdir('completion')
