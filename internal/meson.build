conf = configuration_data()
conf.set_quoted('PACKAGE_NAME', meson.project_name())
conf.set_quoted('PACKAGE_VERSION', meson.project_version())
conf.set_quoted('PATH_REVDEPD', revdepd_dir)
conf.set_quoted('PATH_PKGDB', pkgdb_path)
conf.set_quoted('PATH_LDSOCONF', ldsoconf_path)
#conf.set10('...', get_option('...'))

configure_file(
  output: 'revdep-config.h',
  configuration: conf,
)

config_inc = include_directories('.')
