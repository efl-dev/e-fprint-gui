inc = include_directories('.', '../..')
deps = [
    elm,
    edje,
    ecore,
    ecorex,
    depnls
]

src = [
  'e-fprint-gui.h',
  'e-fprint-gui.c',
  'e-fprint-fprint.c',
]
executable('e-fprint-gui', 
  src,
  include_directories: inc,
  dependencies       : deps,
  gui_app            : true,
  install_dir        : join_paths(dir_bin),
  install            : true)
