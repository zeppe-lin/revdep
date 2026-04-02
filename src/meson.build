common_deps = [librevdep_dep]
common_incs = [config_inc]
common_link_args = []

if want_static
  common_link_args += ['-static']
endif

exe_kwargs = {
  'dependencies': common_deps,
  'include_directories': [common_incs],
  'link_args': common_link_args,
  'install': true,
}

executable(
  'revdep',
  'revdep.cpp',
  kwargs: exe_kwargs,
  install_dir: get_option('bindir')
)
