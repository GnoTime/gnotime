

#ifndef GTT_GCONF_IO_H_
#define GTT_GCONF_IO_H_

#include <glib.h>

void gtt_gconf_save (void);
void gtt_gconf_load (void);
gboolean gtt_gconf_exists (void);

#endif /* GTT_GCONF_IO_H_ */

#include <gconf/gconf-client.h>
#include <glib.h>

#include "app.h"
#include "prefs.h"


#define GTT_GCONF "/apps/GnoTimeDebug"

/* XXX Should use changesets */
/* XXX warnings should be graphical */
#define CHKERR(rc,err_ret,dir) {                                       \
   if (rc) {                                                           \
      printf ("GTT: GConf: Warning: set %s failed: ", dir);            \
      if (err_ret) printf ("%s", err_ret->message);                    \
      printf ("\n");                                                   \
   }                                                                   \
}

#define SETBOOL(dir,val) {                                             \
   gboolean rc;                                                        \
   GError *err_ret= NULL;                                              \
                                                                       \
   rc = gconf_client_set_bool (client, GTT_GCONF dir, val, &err_ret);  \
   CHKERR (rc,err_ret,dir);                                            \
}

#define SETINT(dir,val) {                                              \
   gboolean rc;                                                        \
   GError *err_ret= NULL;                                              \
                                                                       \
   rc = gconf_client_set_int (client, GTT_GCONF dir, val, &err_ret);   \
   CHKERR (rc,err_ret,dir);                                            \
}

#define SETSTR(dir,val) {                                              \
   gboolean rc;                                                        \
   GError *err_ret= NULL;                                              \
                                                                       \
   rc = gconf_client_set_str (client, GTT_GCONF dir, val, &err_ret);   \
   CHKERR (rc,err_ret,dir);                                            \
}

#define UNSET(dir) {                                                   \
   gboolean rc;                                                        \
   GError *err_ret= NULL;                                              \
                                                                       \
   rc = gconf_client_unset (client, GTT_GCONF dir, &err_ret);          \
   CHKERR (rc,err_ret,dir);                                            \
}

/* ======================================================= */
/* save only the GUI configuration info, not the actual data */

void
gtt_gconf_save (void)
{
	GList *node;
	char s[120];
	int i;
	int x, y, w, h;
	const char *xpn;
	
	GConfClient *client;

	client = gconf_client_get_default ();

	/* ------------- */
	/* save the window location and size */
	gdk_window_get_origin(app_window->window, &x, &y);
	gdk_window_get_size(app_window->window, &w, &h);
	SETINT ("/Geometry/Width", w);
	SETINT ("/Geometry/Height", h);
	SETINT ("/Geometry/X", x);
	SETINT ("/Geometry/Y", y);

	{
		int vp, hp;
		notes_area_get_pane_sizes (global_na, &vp, &hp);
		SETINT ("/Geometry/VPaned", vp);
		SETINT ("/Geometry/HPaned", hp);
	}
	/* ------------- */
	/* save the configure dialog values */
	SETBOOL ("/Display/ShowSecs", config_show_secs);
	SETBOOL ("/Display/ShowStatusbar", config_show_statusbar);
	SETBOOL ("/Display/ShowSubProjects", config_show_subprojects);
	SETBOOL ("/Display/ShowTableHeader", config_show_clist_titles);
	SETBOOL ("/Display/ShowTimeCurrent", config_show_title_current);
	SETBOOL ("/Display/ShowTimeDay", config_show_title_day);
	SETBOOL ("/Display/ShowTimeYesterday", config_show_title_yesterday);
	SETBOOL ("/Display/ShowTimeWeek", config_show_title_week);
	SETBOOL ("/Display/ShowTimeLastWeek", config_show_title_lastweek);
	SETBOOL ("/Display/ShowTimeMonth", config_show_title_month);
	SETBOOL ("/Display/ShowTimeYear", config_show_title_year);
	SETBOOL ("/Display/ShowTimeEver", config_show_title_ever);
	SETBOOL ("/Display/ShowDesc", config_show_title_desc);
	SETBOOL ("/Display/ShowTask", config_show_title_task);
	SETBOOL ("/Display/ShowEstimatedStart", config_show_title_estimated_start);
	SETBOOL ("/Display/ShowEstimatedEnd", config_show_title_estimated_end);
	SETBOOL ("/Display/ShowDueDate", config_show_title_due_date);
	SETBOOL ("/Display/ShowSizing", config_show_title_sizing);
	SETBOOL ("/Display/ShowPercentComplete", config_show_title_percent_complete);
	SETBOOL ("/Display/ShowUrgency", config_show_title_urgency);
	SETBOOL ("/Display/ShowImportance", config_show_title_importance);
	SETBOOL ("/Display/ShowStatus", config_show_title_status);

	xpn = ctree_get_expander_state (global_ptw);
	SETSTR ("/Display/ExpanderState", xpn);

	/* ------------- */
	SETBOOL ("/Toolbar/ShowIcons", config_show_tb_icons);
	SETBOOL ("/Toolbar/ShowTexts", config_show_tb_texts);
	SETBOOL ("/Toolbar/ShowTips", config_show_tb_tips);
	SETBOOL ("/Toolbar/ShowNew", config_show_tb_new);
	SETBOOL ("/Toolbar/ShowCCP", config_show_tb_ccp);
	SETBOOL ("/Toolbar/ShowJournal", config_show_tb_journal);
	SETBOOL ("/Toolbar/ShowProp", config_show_tb_prop);
	SETBOOL ("/Toolbar/ShowTimer", config_show_tb_timer);
	SETBOOL ("/Toolbar/ShowPref", config_show_tb_pref);
	SETBOOL ("/Toolbar/ShowHelp", config_show_tb_help);
	SETBOOL ("/Toolbar/ShowExit", config_show_tb_exit);

	/* ------------- */
	if (config_shell_start) {
		SETSTR ("/Actions/StartCommand", config_shell_start);
	} else {
		UNSET ("/Actions/StartCommand");
	}

	if (config_shell_stop) {
		SETSTR ("/Actions/StopCommand", config_shell_stop);
	} else {
		UNSET ("/Actions/StopCommand");
	}

	/* ------------- */
	SETBOOL ("/LogFile/Use", config_logfile_use);
	if (config_logfile_name) {
		SETSTR ("/LogFile/Filename", config_logfile_name);
	} else {
		UNSET ("/LogFile/Filename");
	}
		
	if (config_logfile_start) {
		SETSTR ("/LogFile/Entry", config_logfile_start);
	} else {
		SETSTR ("/LogFile/Entry", "");
	}
		
	if (config_logfile_stop) {
		SETSTR ("/LogFile/EntryStop", config_logfile_stop);
	} else {
		SETSTR ("/LogFile/EntryStop", "");
	}

	SETINT ("/LogFile/MinSecs", config_logfile_min_secs);

	/* ------------- */
	SETSTR ("/Data/URL", config_data_url);
	SETINT ("/Data/SaveCount", save_count);

	/* ------------- */
	w = 0;
	for (i = 0; -1< w; i++) 
	{
	   gboolean rc;
		GError *err_ret= NULL;

		g_snprintf(s, sizeof (s), GTT_GCONF"/CList/ColumnWidth%d", i);
		w = ctree_get_col_width (global_ptw, i);
		if (0 > w) break;
		rc = gconf_client_set_int(client, s, w, &err_ret);
		CHKERR(rc,err_ret);
	}

	/* ------------- */
	g_snprintf(s, sizeof (s), "%ld", time(0));
	SETSTR ("/Misc/LastTimer", s);
	SETINT ("/Misc/IdleTimeout", config_idle_timeout);
	SETINT ("/Misc/AutosavePeriod", config_autosave_period);
	SETINT ("/Misc/TimerRunning", timer_is_running());
	SETINT ("/Misc/CurrProject", gtt_project_get_id (cur_proj));
	SETINT ("/Misc/NumProjects", -1);

	/* write out the customer report info */
	i = 0;
	for (node = gtt_plugin_get_list(); node; node=node->next)
	{
	   gboolean rc;
		GError *err_ret= NULL;

		GttPlugin *plg = node->data;
	   g_snprintf(s, sizeof (s), GTT_CONF"/Report%d/Name", i);
		rc = gconfig_client_set_string(client, s, plg->name, &err_ret);
		CHKERR (rc,err_ret,s);

	   g_snprintf(s, sizeof (s), GTT_CONF"/Report%d/Path", i);
		rc = gconfig_client_set_string(client, s, plg->path, &err_ret);
		CHKERR (rc,err_ret,s);

	   g_snprintf(s, sizeof (s), GTT_CONF"/Report%d/Tooltip", i);
		rc = gconfig_client_set_string(client, s, plg->tooltip, &err_ret);
		CHKERR (rc,err_ret,s);

		i++;
	}
	SETINT ("/Misc/NumReports", i);

   /* Sync to file.
	 * XXX if this fails, the error is serious, and tehre should be a 
	 * graphical popup.
	 */
   {
		GError *err_ret= NULL;
		gconf_client_suggest_sync (client, &err_ret);
		if (NULL != err_ret)
		{
			printf ("GTT: GConf: Sync Failed\n");
		}
	}
}

/* ======================================================= */

gboolean
gtt_gconf_exists (void)
{
	gboolean rc;
	GConfClient *client;
	GError *err_ret= NULL;

	client = gconf_client_get_default ();

	rc = gconf_client_dir_exists (client, GTT_GCONF, &err_ret);
	if (err_ret) printf ("duude err %s\n", err_ret->message);

	return rc;
}
	
void
gtt_gconf_load (void)
{
	gboolean rc;
	GConfClient *client;
	GError *err_ret= NULL;

	client = gconf_client_get_default ();

	rc = gconf_client_dir_exists (client, GTT_GCONF, &err_ret);
	if (err_ret) printf ("duude err %s\n", err_ret->message);

}

/* =========================== END OF FILE ========================= */
