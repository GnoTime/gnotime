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

#include <errno.h>
#include <gconf/gconf.h>
#include <glade/glade.h>
#include <gnome.h>
#include <guile/gh.h>
#include <libgnomeui/gnome-window-icon.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

#include "app.h"
#include "ctree.h"
#include "ctree-gnome2.h"
#include "cur-proj.h"
#include "dialog.h"
#include "err-throw.h"
#include "file-io.h"
#include "gtt.h"
#include "log.h"
#include "menus.h"
#include "menucmd.h"
#include "prefs.h"
#include "timer.h"
#include "xml-gtt.h"


char *first_proj_title = NULL;  /* command line over-ride */

static gboolean first_time_ever = FALSE;  /* has gtt ever run before? */

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
	g_string_append (str, "/.gnotime");
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

/* Return a 1 if the indicated directory did not exist, and
 * and was successfully created.  Else return 0.
 */
static int
create_data_dir (const char * fpath)
{
	struct stat fsb;
	char * filepath;
	int rc;
	char * sep;

	filepath = g_strdup (fpath);
	sep = rindex (filepath, '/');
	sep ++;
	*sep = 0x0;   /* null terminate */

	/* See if directory exists */
	rc = stat (filepath, &fsb);
	if (0 > rc)
	{
		int norr = errno;
		if (norr)
		{
			/* If we are here, directory does not exist. */
			/* Try to create it */
			rc = mkdir (filepath, S_IRWXU);
			if (0 == rc)
			{
				/* If we are here, the create directory succeeded. */
				g_free (filepath);
				return 1;
			}
		}
	}
	g_free (filepath);
	return 0;
}


static void
post_read_data(void)
{
	gtt_post_data_config();

	err_init();
	ctree_setup(global_ptw);
	gtt_post_ctree_config();
	menu_set_states();
	init_timer();

	/* plugins need to be added to the main menus dynamically,
	 * after the config file has been read */
	menus_add_plugins (GNOME_APP(app_window));
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

static char *
resolve_path (const char * pathfrag)
{
	char * fullpath;

	if (('~' != pathfrag[0]) &&
	    ('/' != pathfrag[0]))
	{
		/* if not an absolute filepath ..*/
		fullpath = gnome_config_get_real_path (pathfrag);
	}
	else
	{
		/* I suppose we should look up $HOME if ~ */
		fullpath = g_strdup (pathfrag);
	}

	return fullpath;
}


static void 
read_data(void)
{
	GttErrCode xml_errcode;
	char * xml_filepath;
	gboolean read_is_ok;
	char *errmsg, *qmsg;

	xml_filepath = resolve_path (config_data_url);

	/* Try ... */
	gtt_err_set_code (GTT_NO_ERR);
	gtt_xml_read_file (xml_filepath);

	/* Catch ... */
	xml_errcode = gtt_err_get_code();

	read_is_ok = (GTT_NO_ERR == xml_errcode);
	
	/* If the xml file read bombed because the file doesn't exist,
	 * and yet the project list isn't null, that's because we read
	 * and old-format config file that had the proejcts in it.
	 * This is not an error. This is OK.
	 */
	read_is_ok |= (GTT_CANT_OPEN_FILE == xml_errcode) &&
	               gtt_get_project_list();

	/* Its possible that the read failed because this is the
	 * first time that gnotime is being run.  In this case,
	 * create the data directory, and don't display any errors.
	 */
	if (GTT_CANT_OPEN_FILE == xml_errcode)
	{
		/* If the create directory succeeded, its not an error
		 * if GTT is being run for the first time.
		 */
		if (create_data_dir (xml_filepath))
		{
			read_is_ok |= first_time_ever;
		}
	}

	if (read_is_ok)
	{
		post_read_data ();
		g_free (xml_filepath);
		return;
	}

	/* Else handle an error. */
	errmsg = gtt_err_to_string (xml_errcode, xml_filepath);
	qmsg = g_strconcat (errmsg, 
			_("Do you want to continue?"),
			NULL);

	msgbox_ok_cancel(_("Error"),
			 qmsg,
			 GTK_STOCK_YES, 
			 GTK_STOCK_NO,
			 G_CALLBACK(read_data_err_run_or_abort));
	g_free (qmsg); 
	g_free (errmsg);
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
		first_time_ever = TRUE;
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
		const char *fp;
		char *errmsg, *qmsg;
		fp = gtt_get_config_filepath();
		errmsg = gtt_err_to_string (conf_errcode, fp);
		qmsg = g_strconcat (errmsg, 
			_("Shall I setup a new configuration?"),
			NULL);

		msgbox_ok_cancel(_("Error"),
			 qmsg,
			 GTK_STOCK_YES, 
			 GTK_STOCK_NO,
			 G_CALLBACK(read_config_err_run_or_abort));
		g_free (qmsg); 
		g_free (errmsg);
	}
	else 
	{
		post_read_config();
	}
}


#if DEVEL_VERSION_WARNING
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

/* The make_backup() routine save backup copies of the data file
 * every time that its called.  Its structured so that older copies
 * are saved exponentially less often.  This results in a logarithmic
 * distribution of backups; a realatively small number of files, of which
 * few are old, and most are younger.  The idea is that this 
 * should get you out of a jam, no matter how old your mistake is.
 * Sure wish I'd had this implemented earlier in my debugging cycle :-(
 *
 * Note on the algorithm: do *not* change this!  Its rather subtle,
 * as to which copies it keeps, and which it discards; fiddling with
 * the algo will result in the tail end copies being whacked incorrectly.
 * It took me some work to get this right.
 */
static void
make_backup (const char * filename)
{
	extern int save_count;
	char *old_name, *new_name;
	size_t len;
	struct stat old_stat;
	struct utimbuf ub;
	int suffix=0;
	int lm, i;

	/* Figure out how far to back up.  This computes a
	 * logarithm base BK_FREQ */
	save_count ++;
	lm = save_count;
	while (lm)
	{
#define BK_FREQ 4
		if (0 == lm%BK_FREQ) suffix ++;
		else break;
		lm /= BK_FREQ;
	}
	lm %= BK_FREQ;

	/* Build filenames */
	len = strlen (filename);
	old_name = g_new0 (char, len+20);
	new_name = g_new0 (char, len+20);
	strcpy (old_name, filename);
	strcpy (new_name, filename);
		

	if ((0 < suffix) && (0<lm))
	{
		/* Shuffle files, but preserve datestamps */
		/* Don't report errors, as user may have erased 
		 * some of these older files by hand */
		sprintf (new_name+len, ".%d.%d", suffix, lm); 
		sprintf (old_name+len, ".%d.2", suffix-1); 
		stat (old_name, &old_stat);
		rename (old_name, new_name);
		ub.actime = old_stat.st_atime;
		ub.modtime = old_stat.st_mtime;
		utime (new_name, &ub);
	}
	
	sprintf (new_name+len, ".0.%d",lm); 
	stat (filename, &old_stat);
	rename (filename, new_name);
	ub.actime = old_stat.st_atime;
	ub.modtime = old_stat.st_mtime;
	utime (new_name, &ub);

	g_free (old_name);
	g_free (new_name);
}

/* save_all() saves both data and config file, and does this 
 * without involving the GUI.
 * It is a bit sloppy, in that if we get two errors in a row,
 * we'll miss the second one ... but what the hey, who cares.
 */

char *
save_all (void)
{
	GttErrCode errcode;
	char *errmsg = NULL;
	char * xml_filepath;

	xml_filepath = resolve_path (config_data_url);

	make_backup (xml_filepath);

	/* Try ... */
	gtt_err_set_code (GTT_NO_ERR);
	gtt_xml_write_file (xml_filepath);

	/* Catch */
	errcode = gtt_err_get_code();
	if (GTT_NO_ERR != errcode)
	{
		errmsg = gtt_err_to_string (errcode, xml_filepath);
	}
	g_free (xml_filepath);

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

/* Save application properties, use GUI to indicate problem */

void
save_properties (void)
{
	GttErrCode errcode;

	/* Try ... */
	gtt_err_set_code (GTT_NO_ERR);
	gtt_save_config (NULL);

	/* Catch */
	errcode = gtt_err_get_code();
	if (GTT_NO_ERR != errcode)
	{
		const char *fp = gtt_get_config_filepath();
		char *errmsg = gtt_err_to_string (errcode, fp);

		msgbox_ok(_("Warning"),
		     errmsg,
		     GTK_STOCK_OK,
		     NULL);
		g_free (errmsg);
	}
}

/* Save project data, use GUI to indicate problem */

void
save_projects (void)
{
	GttErrCode errcode;
	char * xml_filepath;

	/* Try ... */
	xml_filepath = resolve_path (config_data_url);
	make_backup (xml_filepath);

	gtt_err_set_code (GTT_NO_ERR);
	gtt_xml_write_file (xml_filepath);

	/* Try to handle a bizzare missing-directory error 
	 * by creating the directory, and trying again. */
	errcode = gtt_err_get_code();
	if (GTT_CANT_OPEN_FILE == errcode)
	{
		create_data_dir (xml_filepath);
		gtt_err_set_code (GTT_NO_ERR);
		gtt_xml_write_file (xml_filepath);
	}
				  
	/* Catch */
	errcode = gtt_err_get_code();
	if (GTT_NO_ERR != errcode)
	{
		char *errmsg = gtt_err_to_string (errcode, xml_filepath);

		msgbox_ok(_("Warning"),
		     errmsg,
		     GTK_STOCK_OK,
		     NULL);
		g_free (errmsg);
	}

	g_free (xml_filepath);
}



#ifdef USE_SM

/*
 * session management
 */

static int
save_state(GnomeClient *client, gint phase, GnomeRestartStyle save_style,
	   gint shutdown, GnomeInteractStyle interact_styyle, gint fast,
	   gpointer data)
{
	char *errmsg;
	const char *sess_id;
	char *argv[5];
	int argc;
	int x, y, w, h;
	int rc;

	sess_id  = gnome_client_get_id(client);
	if (!app_window) return FALSE;

	gdk_window_get_origin (app_window->window, &x, &y);
	gdk_window_get_size (app_window->window, &w, &h);
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
	g_free (errmsg);

	return rc;
}


static void
session_die(GnomeClient *client)
{
	app_quit(NULL, NULL);
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

#if 0
	/* Clean things up on exit.  But this is pretty pointless */
	gtk_widget_destroy (app_window);
	project_list_destroy ();
#endif
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

	gnome_program_init(PACKAGE, VERSION, LIBGNOMEUI_MODULE, argc, argv, 
		                   GNOME_PARAM_POPT_TABLE, geo_options, 
		                   GNOME_PROGRAM_STANDARD_PROPERTIES, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-cromagnon.png");

	bindtextdomain(GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

#ifdef USE_SM
	client = gnome_master_client();
	g_signal_connect(G_OBJECT(client), "save_yourself",
			   G_CALLBACK(save_state), (gpointer) argv[0]);
	g_signal_connect(G_OBJECT(client), "die",
			   G_CALLBACK(session_die), NULL);
#endif /* USE_SM */

	glade_init();

	/* gconf init is needed by gtkhtml */
	gconf_init (argc, argv, NULL);

	signal (SIGCHLD, SIG_IGN);
	signal (SIGINT, got_signal);
	signal (SIGTERM, got_signal);
	lock_gtt();
	app_new(argc, argv, geometry_string);

	g_signal_connect(G_OBJECT(app_window), "delete_event",
			   G_CALLBACK(app_quit), NULL);

#if DEVEL_VERSION_WARNING
	msgbox_ok_cancel(_("Warning"),
		"WARNING !!! Achtung !!! Avertisment !!!\n"
		"\n"
		"This is a development version of GTT.  "
		"It probably crashes. Use at own risk!\n"
		"\n"
		"The last stable, working version can be obtained from\n"
		"the www.gnome.org gnome-utils cvs with\n"
		"cvs -z3 -d :pserver:anonymous@anoncvs.gnome.org:/cvs/gnome "
		"checkout -r gnome-utils-1-4 gnome-utils\n",
	     "Continue", "Exit", 
		G_CALLBACK(beta_run_or_abort));
#else
	read_config();
#endif

	gh_enter(argc, argv, guile_inner_main);
	return 0; /* not reached !? */
}

/* ======================= END OF FILE =================== */
