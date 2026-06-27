project(
  'revdep',
  'cpp',
  version: '5.0',
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

subdir('internal')
subdir('src')
subdir('man')
subdir('completion')
