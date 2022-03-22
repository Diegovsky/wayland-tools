project('wayland-tools', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c11'])

wayland_client = dependency('wayland-client')

if get_option('wl_globals')
  executable('wayland-globals',
    'wl-globals.c',
    dependencies: [
      wayland_client
      ],
    install : true)
endif
