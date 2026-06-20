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

internal_inc = include_directories('.')
