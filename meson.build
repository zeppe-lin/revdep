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

conf = configuration_data()
conf.set_quoted('PACKAGE_NAME', meson.project_name())
conf.set_quoted('PACKAGE_VERSION', meson.project_version())
conf.set_quoted('PATH_REVDEPD', get_option('revdepd_dir'))
conf.set_quoted('PATH_PKGDB', get_option('pkgdb_path'))
conf.set_quoted('PATH_LDSOCONF', get_option('ldsoconf_path'))

configure_file(
  output: 'revdep-config.h',
  configuration: conf,
)

config_inc = include_directories('.')

subdir('src')
subdir('man')
subdir('completion')
