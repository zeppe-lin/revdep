if get_option('bashcomp')
  bashcompdir = get_option('bashcompdir')
  if bashcompdir == ''
    bashcompdir = get_option('datadir') / 'bash-completion' / 'completions'
  endif

  install_data('revdep.bash', install_dir: bashcompdir, rename: 'revdep')
endif
