/*   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001 Linas Vepstas <linas@linas.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <gconf/gconf.h>
#include <glade/glade.h>
#include <gnome.h>
#include <guile/gh.h>
#include <libgnomeui/gnome-window-icon.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "app.h"
#include "ctree.h"
#include "cur-proj.h"
#include "dialog.h"
#include "err-throw.h"
#include "file-io.h"
#include "gtt.h"
#include "log.h"
#include "menus.h"
#include "menucmd.h"
#include "prefs.h"
#include "shorts.h"		/* SMH 2000-03-22: connect_short_cuts() */
#include "timer.h"
#include "xml-gtt.h"


char *first_proj_title = NULL;  /* command line over-ride */

/* SM == session management */
#define USE_SM


const char *
gtt_gettext(const char *s)
{
	g_return_val_if_fail(s != NULL, NULL);
	if (0 == strncmp(s, "[GTT]", 5))
		return &s[5];
	return s;
}




static char *build_lock_fname(void)
{
	GString *str;
	static char *fname = NULL;
	
	if (fname != NULL) return fname;

	/* note it will handle unset "HOME" fairly gracefully */
	str = g_string_new (g_getenv ("HOME"));
	g_string_append (str, "/.gtimetracker");
#ifdef DEBUG
	g_string_append (str, "-" VERSION);
#endif
	g_string_append (str, ".pid");

	fname = str->str;
	g_string_free (str, FALSE);
	return fname;
}



static void lock_gtt(void)
{
	FILE *f;
	char *fname;
	gboolean warn = FALSE;
	
	fname = build_lock_fname ();

	/* if the pid file exists and such a process exists
	 * and this process is owned by the current user,
	 * else this pid file is very very stale and can be
	 * ignored */
	if (NULL != (f = fopen(fname, "rt"))) {
		int pid;

		if (fscanf (f, "%d", &pid) == 1 &&
		    pid > 0 &&
		    kill (pid, 0) == 0) {
			warn = TRUE;
		}
		fclose(f);
	}
		
	if (warn) 
	{
		GtkWidget *warning;
		warning = gnome_message_box_new(
			_("There seems to be another GTimeTracker running.\n"
			  "Press OK to start GTimeTracker anyway, or press Cancel to quit."),
			GNOME_MESSAGE_BOX_WARNING,
			GTK_STOCK_OK,
			GTK_STOCK_CANCEL,
			NULL);
		if(gnome_dialog_run_and_close(GNOME_DIALOG(warning))!=0)
		{
			exit(0);
		}
	}
	if (NULL == (f = fopen(fname, "wt"))) {
		g_warning(_("Cannot create pid-file!"));
	}
	fprintf(f, "%d\n", getpid());
	fclose(f);
}



void 
unlock_gtt(void)
{
	log_exit();
	unlink(build_lock_fname());
}

static void
post_read_data(void)
{
	gtt_post_data_config();

	err_init();
	ctree_setup(global_ptw);
	init_timer();

	/* plugins need to be added to the main menus dynamically,
	 * after the config file has been read */
	menus_add_plugins (GNOME_APP(window));
	log_start();
	app_show();
}

static void 
read_data_err_run_or_abort (GtkWidget *w, gint butnum)
{
	if (butnum == 1)
	{
		gtk_main_quit();
	}
	else
	{
		post_read_data();
	}
}

static const char *
resolve_path (const char * pathfrag)
{
	const char * fullpath;

	if (('~' != pathfrag[0]) &&
	    ('/' != pathfrag[0]))
	{
		/* if not an absolute filepath ..*/
		fullpath = gnome_config_get_real_path (pathfrag);
	}
	else
	{
		/* I suppose we should look up $HOME if ~ */
		fullpath = pathfrag;
	}

	return fullpath;
}


static void 
read_data(void)
{
	GttErrCode xml_errcode;
	const char * xml_filepath;

	xml_filepath = resolve_path (config_data_url);

	/* Try ... */
	gtt_err_set_code (GTT_NO_ERR);
	gtt_xml_read_file (xml_filepath);

	/* Catch ... */
	xml_errcode = gtt_err_get_code();

	/* If the xml file read bombed because the file doesn't exist,
	 * and yet the project list isn't null, that's because we read
	 * and old-format config file that had the proejcts in it.
	 * This is not an error. This is OK.
	 */
	if (!((GTT_NO_ERR == xml_errcode) ||
	      ((GTT_CANT_OPEN_FILE == xml_errcode) &&
		gtt_get_project_list())
	    ))
	{
		const char *errmsg, *qmsg;
		errmsg = gtt_err_to_string (xml_errcode, xml_filepath);
		qmsg = g_strconcat (errmsg, 
			_("Do you want to continue?"),
			NULL);

		msgbox_ok_cancel(_("Error"),
			 qmsg,
			 GTK_STOCK_YES, 
			 GTK_STOCK_NO,
			 GTK_SIGNAL_FUNC(read_data_err_run_or_abort));
		g_free ((gchar *) qmsg); 
		g_free ((gchar *) errmsg);
	}
	else
	{
		post_read_data ();
	}
}

static void
post_read_config(void)
{
	read_data();
}

static void 
read_config_err_run_or_abort (GtkWidget *w, gint butnum)
{
	if (butnum == 1)
	{
		gtk_main_quit();
	}
	else
	{
		post_read_config();
	}
}

static void 
read_config(void)
{
	GttErrCode conf_errcode;

	/* Try ... */
	gtt_err_set_code (GTT_NO_ERR);
	gtt_load_config (NULL);

	/* Catch ... */
	conf_errcode = gtt_err_get_code();
	if (GTT_NO_ERR != conf_errcode) 
	{
		const char *fp, *errmsg, *qmsg;
		fp = gtt_get_config_filepath();
		errmsg = gtt_err_to_string (conf_errcode, fp);
		qmsg = g_strconcat (errmsg, 
			_("Shall I setup a new configuration?"),
			NULL);

		msgbox_ok_cancel(_("Error"),
			 qmsg,
			 GTK_STOCK_YES, 
			 GTK_STOCK_NO,
			 GTK_SIGNAL_FUNC(read_config_err_run_or_abort));
		g_free ((gchar *) qmsg); 
		g_free ((gchar *) errmsg);
	}
	else 
	{
		post_read_config();
	}
}


#if 0
/* used only to display development version warning messsage */
static void 
beta_run_or_abort(GtkWidget *w, gint butnum)
{
	if (butnum == 1)
	{
		gtk_main_quit();
	}
	else
	{
		read_config();
	}
}
#endif

/* save_all() is a bit sloppy, in that if we get two errors in a row,
 * we'll miss the second one ... but what the hey, who cares.
 */

const char *
save_all (void)
{
	GttErrCode errcode;
	const char *errmsg = NULL;
	const char * xml_filepath;

	xml_filepath = resolve_path (config_data_url);
	/* Try ... */
	gtt_err_set_code (GTT_NO_ERR);
	gtt_xml_write_file (xml_filepath);

	/* Catch */
	errcode = gtt_err_get_code();
	if (GTT_NO_ERR != errcode)
	{
		errmsg = gtt_err_to_string (errcode, xml_filepath);
	}

	/* Try ... */
	gtt_err_set_code (GTT_NO_ERR);
	gtt_save_config (NULL);

	/* Catch */
	errcode = gtt_err_get_code();
	if (GTT_NO_ERR != errcode)
	{
		const char *fp;
		fp = gtt_get_config_filepath();
		errmsg = gtt_err_to_string (errcode, fp);
	}

	return errmsg;
}



#ifdef USE_SM

/*
 * session management
 */

#ifdef DEBUG
#define GTT "/gtt-DEBUG/"
#else /* not DEBUG */
#define GTT "/gtt/"
#endif /* not DEBUG */

static int
save_state(GnomeClient *client, gint phase, GnomeRestartStyle save_style,
	   gint shutdown, GnomeInteractStyle interact_styyle, gint fast,
	   gpointer data)
{
	const char *errmsg;
	const char *sess_id;
	char *argv[5];
	int argc;
	int x, y, w, h;
	int rc;

	sess_id  = gnome_client_get_id(client);
	if (!window)
		return FALSE;

	gdk_window_get_origin(window->window, &x, &y);
	gdk_window_get_size(window->window, &w, &h);
	argv[0] = (char *)data;
	argv[1] = "--geometry";
	argv[2] = g_strdup_printf("%dx%d+%d+%d", w, h, x, y);
	if ((cur_proj) && (gtt_project_get_title(cur_proj))) {
		argc = 5;
		argv[3] = "--select-project";
		argv[4] = (char *) gtt_project_get_title(cur_proj);
	} else {
		argc = 3;
	}
	gnome_client_set_clone_command(client, argc, argv);
	gnome_client_set_restart_command(client, argc, argv);
	g_free(argv[2]);

	/* save both the user preferences/config and the project lists */
	errmsg = save_all();
	rc = 0;
	if (NULL == errmsg) rc = 1;
	g_free ((gchar *) errmsg);

	return rc;
}


static void
session_die(GnomeClient *client)
{
	quit_app(NULL, NULL);
}

#endif /* USE_SM */


static void
got_signal (int sig)
{
	unlock_gtt ();
	
	/* whack thyself */
	signal (sig, SIG_DFL);
	kill (getpid (), sig);
}


static void 
guile_inner_main(int argc, char **argv)
{
	gtk_main();

	unlock_gtt();
}


int 
main(int argc, char *argv[])
{
	static char *geometry_string = NULL;
#ifdef USE_SM
	GnomeClient *client;
#endif /* USE_SM */
	static const struct poptOption geo_options[] =
	{
		{"geometry", 'g', POPT_ARG_STRING, &geometry_string, 0,
			N_("Specify geometry"), N_("GEOMETRY")},
		{"select-project", 's', POPT_ARG_STRING, &first_proj_title, 0,
			N_("Select a project on startup"), N_("PROJECT")},
		{NULL, '\0', 0, NULL, 0}
	};

	gnome_init_with_popt_table("gtt", VERSION, argc, argv,
				   geo_options, 0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-cromagnon.png");

	bindtextdomain(GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

#ifdef USE_SM
	client = gnome_master_client();
	gtk_signal_connect(GTK_OBJECT(client), "save_yourself",
			   GTK_SIGNAL_FUNC(save_state), (gpointer) argv[0]);
	gtk_signal_connect(GTK_OBJECT(client), "die",
			   GTK_SIGNAL_FUNC(session_die), NULL);
#endif /* USE_SM */

	glade_gnome_init();

	/* gconf init is needed by gtkhtml */
	gconf_init (argc, argv, NULL);

	signal (SIGCHLD, SIG_IGN);
	signal (SIGINT, got_signal);
	signal (SIGTERM, got_signal);
	lock_gtt();
	app_new(argc, argv, geometry_string);

	gtk_signal_connect(GTK_OBJECT(window), "delete_event",
			   GTK_SIGNAL_FUNC(quit_app), NULL);

	/*
	 * Added by SMH 2000-03-22:
	 * Connect short-cut keys. 
	 */
	connect_short_cuts();

#if 0
	msgbox_ok_cancel(_("Warning"),
		"WARNING !!! Achtung !!! Avertisment !!!\n"
		"\n"
		"This is a development version of GTT.  It uses a new\n"
		"file format that may leave your old data damaged and\n"
		"unrecoverable.  There may be incompatible file format\n"
		"changes in the near future. Use at own risk!\n"
		"\n"
		"The last stable, working version can be obtained from\n"
		"version 1.4.0.2 of gnome-utils or from cvs with\n"
		"cvs checkout -D \"Aug 27 2001\" gnome-utils/gtt\n",
	     "Continue", "Exit", 
		GTK_SIGNAL_FUNC(beta_run_or_abort));
#else
	read_config();
#endif

	gh_enter(argc, argv, guile_inner_main);
	return 0; /* not reached !? */
}


