gnotime_cfg_data = configuration_data()
gnotime_cfg_data.set_quoted('DATADIR', get_option('prefix') / get_option('datadir') / 'gnotime')
gnotime_cfg_data.set_quoted('GETTEXT_PACKAGE', 'gnotime-2.0')
gnotime_cfg_data.set_quoted('GNOME_ICONDIR', 'NONE' / get_option('datadir') / 'pixmaps')
gnotime_cfg_data.set_quoted('GNOMELOCALEDIR', get_option('prefix') / get_option('localedir'))
gnotime_cfg_data.set_quoted('GTTDATADIR', get_option('prefix') / get_option('datadir') / 'gnotime')
gnotime_cfg_data.set_quoted('GTTGLADEDIR', get_option('prefix') / get_option('datadir') / 'gnotime')
gnotime_cfg_data.set_quoted('LIBDIR', get_option('prefix') / get_option('libdir'))
gnotime_cfg_data.set_quoted('PACKAGE', 'gnotime')
gnotime_cfg_data.set_quoted('PREFIX', get_option('prefix'))
gnotime_cfg_data.set_quoted('SYSCONFDIR', get_option('sysconfdir'))
gnotime_cfg_data.set_quoted('VERSION', meson.project_version())
configure_file(
  configuration: gnotime_cfg_data,
  output: 'config.h',
)

gnotime_deps = [
  gconf_dep,
  glib_dep,
  gnome_vfs_dep,
  gtk_dep,
  gtk_html_dep,
  guile_dep,
  libglade_dep,
  libgnome_dep,
  libgnomeui_dep,
  libxml_dep,
  qof_dep,
  x11_dep,
  xscrnsaver_dep,
]
if dbus_glib_dep.found()
  dbus_glib_arg = 'WITH_DBUS=1'
  gnotime_deps += dbus_glib_dep
else
  dbus_glib_arg = 'WITH_DBUS=0'
endif

gnotime_srcs = files(
  'gtt_activation_dialog.c',
  'gtt_application_window.c',
  'gtt_dbus.c',
  'gtt_err_throw.c',
  'gtt_export.c',
  'gtt_file_io.c',
  'gtt_ghtml.c',
  'gtt_ghtml_deprecated.c',
  'gtt_gsettings_gnomeui.c',
  'gtt_gsettings_io.c',
  'gtt_help_popup.c',
  'gtt_idle_dialog.c',
  'gtt_idle_timer.c',
  'gtt_journal.c',
  'gtt_log.c',
  'gtt_menu_commands.c',
  'gtt_menus.c',
  'gtt_myoaf.c',
  'gtt_notes_area.c',
  'gtt_plug_in.c',
  'gtt_plug_in_edit.c',
  'gtt_preferences.c',
  'gtt_project.c',
  'gtt_project_queries.c',
  'gtt_projects_tree.c',
  'gtt_props_dlg_interval.c',
  'gtt_props_dlg_project.c',
  'gtt_props_dlg_task.c',
  'gtt_queries.c',
  'gtt_signal_handlers.c',
  'gtt_status_icon.c',
  'gtt_timer.c',
  'gtt_toolbar.c',
  'gtt_util.c',
  'gtt_xml_read.c',
  'gtt_xml_write.c',
  'main.c',
)

executable(
  'gnotime',
  gnotime_srcs,
  cpp_args: dbus_glib_arg,
  dependencies: gnotime_deps,
)
