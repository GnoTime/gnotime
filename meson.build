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
gtk_req = '>= 2.24.23'
gtk_html_req = '>= 3.32.2'
guile_req = '>= 2.0.9'
libglade_req = '>= 2.6.4'
libgnome_req = '>= 2.32.1'
libgnomeui_req = '>= 2.24.5'
libxml_req = '>= 2.9.1'
qof_req = '>= 0.8.7'
x11_req = '>= 1.6.2'
xscrnsaver_req = '>= 1.2.2'

dbus_glib_dep = dependency('dbus-glib-1', required: false, version: dbus_glib_req)
gconf_dep = dependency('gconf-2.0', version: gconf_req)
glib_dep = dependency('glib-2.0', version: glib_req)
gtk_dep = dependency('gtk+-2.0', version: gtk_req)
gtk_html_dep = dependency('libgtkhtml-3.14', version: gtk_html_req)
guile_dep = dependency('guile-2.0', version: guile_req)
libglade_dep = dependency('libglade-2.0', version: libglade_req)
libgnome_dep = dependency('libgnome-2.0', version: libgnome_req)
libgnomeui_dep = dependency('libgnomeui-2.0', version: libgnomeui_req)
libxml_dep = dependency('libxml-2.0', version: libxml_req)
qof_dep = dependency('qof', version: qof_req)
x11_dep = dependency('x11', version: x11_req)
xscrnsaver_dep = dependency('xscrnsaver', version: xscrnsaver_req)

subdir('src')
