revdepddir = get_option('revdepd_dir')
if revdepddir == ''
  revdepddir = get_option('sysconfdir') / 'revdep.d'
endif

pkgdbpath = get_option('pkgdb_path')
if pkgdbpath == ''
  pkgdbpath = get_option('sharedstatedir') / 'pkg' / 'db'
endif

ldsoconfpath = get_option('ldsoconf_path')
if ldsoconfpath == ''
  ldsoconfpath = get_option('sysconfdir') / 'ld.so.conf'
endif

conf = configuration_data()
conf.set_quoted('PACKAGE_NAME', meson.project_name())
conf.set_quoted('PACKAGE_VERSION', meson.project_version())
conf.set_quoted('PATH_REVDEPD', revdepddir)
conf.set_quoted('PATH_PKGDB', pkgdbpath)
conf.set_quoted('PATH_LDSOCONF', ldsoconfpath)

configure_file(
  output: 'revdep-config.h',
  configuration: conf,
)

internal_inc = include_directories('.')
