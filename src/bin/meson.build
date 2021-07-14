inc = include_directories('.', '../..')
deps = [
    elm,
    edje,
    ecore,
    ecorex,
    depnls
]

xml_codegen_result = []

foreach file : ['device', 'manager']
  xml_codegen_result += custom_target('dbus_codegen_'+file,
    input: file+'.xml',
    output: ['eldbus_fprint_' + file + '.h', 'eldbus_fprint_' + file + '.c'],
    command: [eldbus_codegen, '-O', meson.current_build_dir(), '@INPUT@'],
  )
endforeach


src = [
  'e-fprint-gui.h',
  'e-fprint-gui.c',
  'e-fprint-fprint.c',
]

executable('e-fprint-gui',
  src + xml_codegen_result,
  include_directories: inc,
  dependencies       : deps,
  gui_app            : true,
  install_dir        : join_paths(dir_bin),
  install            : true)
