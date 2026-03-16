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

librevdep_static = get_option('link_mode') == 'static'

librevdep_dep = dependency(
  'librevdep',
  required: true,
  static: librevdep_static,
)

revdepd_dir_opt = get_option('revdepd_dir')
pkgdb_path_opt = get_option('pkgdb_path')
ldsoconf_path = get_option('ldsoconf_path')

revdepd_dir = revdepd_dir_opt != '' ? revdepd_dir_opt : join_paths(get_option('sysconfdir'), 'revdep.d')
pkgdb_path = pkgdb_path_opt != '' ? pkgdb_path_opt : join_paths(get_option('localstatedir'), 'lib', 'pkg', 'db')

config_inc = include_directories('.')

subdir('internal')
subdir('src')
subdir('man')
subdir('completion')
