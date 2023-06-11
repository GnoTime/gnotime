gnotime_cfg_data = configuration_data()
gnotime_cfg_data.set_quoted('DATADIR', get_option('prefix') / get_option('datadir') / 'gnotime')
gnotime_cfg_data.set_quoted('GETTEXT_PACKAGE', 'gnotime-2.0')
gnotime_cfg_data.set_quoted('GNOME_ICONDIR', 'NONE' / get_option('datadir') / 'pixmaps')
gnotime_cfg_data.set_quoted('GNOMELOCALEDIR', get_option('prefix') / get_option('localedir'))
gnotime_cfg_data.set_quoted('GTTDATADIR', get_option('prefix') / get_option('datadir') / 'gnotime')
gnotime_cfg_data.set_quoted('GTTUIDIR', get_option('prefix') / get_option('datadir') / 'gnotime')
gnotime_cfg_data.set_quoted('LIBDIR', get_option('prefix') / get_option('libdir'))
gnotime_cfg_data.set_quoted('PACKAGE', 'gnotime')
gnotime_cfg_data.set_quoted('PREFIX', get_option('prefix'))
gnotime_cfg_data.set_quoted('SYSCONFDIR', get_option('sysconfdir'))
gnotime_cfg_data.set_quoted('VERSION', meson.project_version() + gtt_version_suffix)
configure_file(
  configuration: gnotime_cfg_data,
  output: 'config.h',
)

gnotime_deps = [
  gconf_dep,
  glib_dep,
  gtk_dep,
  gtk_html_dep,
  guile_dep,
  libgnome_dep,
  libgnomeui_dep,
  libxml_dep,
  qof_dep,
  x11_dep,
  xscrnsaver_dep,
]
if dbus_glib_dep.found()
  dbus_glib_arg = '-DWITH_DBUS=1'
  gnotime_deps += dbus_glib_dep
else
  dbus_glib_arg = '-DWITH_DBUS=0'
endif

gnotime_srcs = files(
  'active-dialog.c',
  'app.c',
  'dbus.c',
  'dialog.c',
  'err.c',
  'err-throw.c',
  'export.c',
  'file-io.c',
  'gconf-gnomeui.c',
  'gconf-io.c',
  'ghtml.c',
  'ghtml-deprecated.c',
  'gtt-date-edit.c',
  'gtt-gsettings-gnomeui.c',
  'gtt-gsettings-io.c',
  'gtt-gsettings-io-p.c',
  'gtt-select-list.c',
  'gtt-history-list.c',
  'idle-dialog.c',
  'idle-timer.c',
  'journal.c',
  'log.c',
  'main.c',
  'menucmd.c',
  'menus.c',
  'notes-area.c',
  'plug-edit.c',
  'plug-in.c',
  'prefs.c',
  'proj.c',
  'projects-tree.c',
  'proj-query.c',
  'props-invl.c',
  'props-proj.c',
  'props-task.c',
  'query.c',
  'status-icon.c',
  'timer.c',
  'toolbar.c',
  'util.c',
  'xml-read.c',
  'xml-write.c',
)

executable(
  'gnotime',
  gnotime_srcs,
  cpp_args: dbus_glib_arg,
  dependencies: gnotime_deps,
)
