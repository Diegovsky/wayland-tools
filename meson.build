project('wayland-tools', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c11'])

wayland_client = dependency('wayland-client')

features = {
  'xdg-output': get_option('wl_outputs')
}

subdir('protocols')

if get_option('wl_globals')
  executable('wl-globals',
    'wl-globals.c',
    dependencies: [
      wayland_client
      ],
    install : true)
endif

if get_option('wl_outputs')
  xdg_output_proto = protocols_code['xdg-output-unstable-v1']
  executable('wl-outputs',
    'wl-outputs.c',
    'wl-utils.c',
    include_directories: 'protocols',
    link_with: xdg_output_proto,
    # Depends on xdg-output-unstable-v1
    dependencies: [
      wayland_client,
    ],
    install : true)
endif
