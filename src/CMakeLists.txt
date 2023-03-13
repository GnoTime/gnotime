cmake_minimum_required(VERSION 3.13.0)

set(DATADIR "/usr/local/share/gnotime")
set(GETTEXT_PACKAGE "gnotime-2.0")
set(GNOMELOCALEDIR "/usr/local/share/locale")
set(GNOME_ICONDIR "NONE/share/pixmaps")
set(GTTDATADIR "/usr/local/share/gnotime")
set(GTTGLADEDIR "/usr/local/share/gnotime")
set(LIBDIR "/usr/local/lib/x86_64-linux-gnu")
set(PACKAGE "gnotime")
set(PREFIX "/usr/local")
set(SYSCONFDIR "etc")
set(VERSION "${PROJECT_VERSION}${GTT_VERSION_SUFFIX}")
configure_file(cmake_config.h.in config.h
)

add_executable(${PROJECT_NAME}
    active-dialog.c
    app.c
    dbus.c
    dialog.c
    err.c
    err-throw.c
    export.c
    file-io.c
    gconf-gnomeui.c
    gconf-io.c
    ghtml.c
    ghtml-deprecated.c
    idle-dialog.c
    idle-timer.c
    journal.c
    log.c
    main.c
    menucmd.c
    menus.c
    notes-area.c
    plug-edit.c
    plug-in.c
    prefs.c
    proj.c
    projects-tree.c
    proj-query.c
    props-invl.c
    props-proj.c
    props-task.c
    query.c
    status-icon.c
    timer.c
    toolbar.c
    util.c
    xml-read.c
    xml-write.c)
target_include_directories(${PROJECT_NAME} SYSTEM
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${DBUS_GLIB_INCLUDE_DIRS}
    PRIVATE ${GCONF_INCLUDE_DIRS}
    PRIVATE ${GLIB_INCLUDE_DIRS}
    PRIVATE ${GTK_INCLUDE_DIRS}
    PRIVATE ${GTK_HTML_INCLUDE_DIRS}
    PRIVATE ${GUILE_INCLUDE_DIRS}
    PRIVATE ${LIBGLADE_INCLUDE_DIRS}
    PRIVATE ${LIBGNOME_INCLUDE_DIRS}
    PRIVATE ${LIBGNOMEUI_INCLUDE_DIRS}
    PRIVATE ${LIBXML_INCLUDE_DIRS}
    PRIVATE ${QOF_INCLUDE_DIRS}
    PRIVATE ${X11_INCLUDE_DIRS}
    PRIVATE ${XSCRNSAVER_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}
    PRIVATE -lm
    PRIVATE ${DBUS_GLIB_LINK_LIBRARIES}
    PRIVATE ${GCONF_LINK_LIBRARIES}
    PRIVATE ${GLIB_LINK_LIBRARIES}
    PRIVATE ${GTK_LINK_LIBRARIES}
    PRIVATE ${GTK_HTML_LINK_LIBRARIES}
    PRIVATE ${GUILE_LINK_LIBRARIES}
    PRIVATE ${LIBGLADE_LINK_LIBRARIES}
    PRIVATE ${LIBGNOME_LINK_LIBRARIES}
    PRIVATE ${LIBGNOMEUI_LINK_LIBRARIES}
    PRIVATE ${LIBXML_LINK_LIBRARIES}
    PRIVATE ${QOF_LINK_LIBRARIES}
    PRIVATE ${X11_LINK_LIBRARIES}
    PRIVATE ${XSCRNSAVER_LINK_LIBRARIES})
