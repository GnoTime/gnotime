# Meson is intended as the future build system of the GnoTime project. For now
# this file serves for IDE integration only and does nothing more than just
# building the application.

project(
  'GnoTime',
  'c',
  license: 'GPL-2.0-or-later',
  meson_version: '>= 1.0.0',
  version: '3.0.0',
)
gtt_version_suffix = '_dev'

add_project_link_arguments(
  '-lm',
  language: 'c',
)

dbus_glib_req = '>= 0.100.2'
gconf_req = '>= 3.2.6'
glib_req = '>= 2.40.2'
gtk_req = '>= 3.20.0'
webkit_gtk_req = '>= 2.32.0'
guile_req = '>= 2.0.9'
libxml_req = '>= 2.9.1'
qof_req = '>= 0.8.7'
x11_req = '>= 1.6.2'
xscrnsaver_req = '>= 1.2.2'

dbus_glib_dep = dependency('dbus-glib-1', required: false, version: dbus_glib_req)
gconf_dep = dependency('gconf-2.0', version: gconf_req)
glib_dep = dependency('glib-2.0', version: glib_req)
gtk_dep = dependency('gtk+-2.0', version: gtk_req)
webkit_gtk_dep = dependency('webkit2gtk-4.0', version: gtk_html_req)
guile_dep = dependency('guile-2.0', version: guile_req)
libxml_dep = dependency('libxml-2.0', version: libxml_req)
qof_dep = dependency('qof', version: qof_req)
x11_dep = dependency('x11', version: x11_req)
xscrnsaver_dep = dependency('xscrnsaver', version: xscrnsaver_req)

subdir('src')
