common_c_args = [
  '-D_GNU_SOURCE',
  '-DG_LOG_USE_STRUCTURED',
]

shared_library(
  'wireplumber-module-pipewire',
  [
    'module-pipewire.c',
    'module-pipewire/loop-source.c',
    'module-pipewire/simple-endpoint-link.c',
    'module-pipewire/simple-endpoint.c',
  ],
  c_args : [common_c_args, '-DG_LOG_DOMAIN="m-pipewire"'],
  install : true,
  #install_dir : modules_install_dir,
  dependencies : [wp_dep, pipewire_dep],
)

shared_library(
  'wireplumber-module-pw-alsa-udev',
  [
    'module-pw-alsa-udev.c',
  ],
  c_args : [common_c_args, '-DG_LOG_DOMAIN="m-pw-alsa-udev"'],
  install : true,
  #install_dir : modules_install_dir,
  dependencies : [wp_dep, pipewire_dep],
)

shared_library(
  'wireplumber-module-pw-audio-softdsp-endpoint',
  [
    'module-pw-audio-softdsp-endpoint.c',
  ],
  c_args : [common_c_args, '-DG_LOG_DOMAIN="m-pw-audio-softdsp-endpoint"'],
  install : true,
  #install_dir : modules_install_dir,
  dependencies : [wp_dep, pipewire_dep],
)