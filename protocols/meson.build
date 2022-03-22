# Heavily based on wlroot's [protol/meso.build](https://gitlab.freedesktop.org/wlroots/wlroots/-/blob/aaf787ee5650e77f0bda4dea8e3ba8325e0e6b39/protocol/meson.build)
wayland_scanner_dep = dependency('wayland-scanner', native: true)
wayland_scanner = find_program(wayland_scanner_dep.get_variable('wayland_scanner'), native: true)

wayland_protos = dependency('wayland-protocols',
	version: '>=1.25',
	fallback: 'wayland-protocols',
	default_options: ['tests=false'],
)

wl_protocol_dir = wayland_protos.get_variable('pkgdatadir')

protocols = {
 'xdg-output-unstable-v1': wl_protocol_dir / 'unstable/xdg-output/xdg-output-unstable-v1.xml', 
 }

protocols_code = {}
foreach name, path : protocols
	private_code = custom_target(
		name.underscorify() + '_c',
		input: path,
		output: '@BASENAME@-protocol.c',
		command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
	)

	header = custom_target(
		name.underscorify() + '_client_h',
		input: path,
		output: '@BASENAME@-client.h',
		command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
                depends: [private_code],
	)

        proto_lib = static_library(name.underscorify() + '-private',
          private_code,
          header,
          sources: private_code,
          dependencies: [wayland_client])

	protocols_code += { name: proto_lib}
endforeach
