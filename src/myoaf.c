
/* NOTE: hack alert XXX FIXME  this code is junk, some attempted
   fun and games to conect to the evolution calendear server.
   it didn't wrk the way its supposed to. It shuld probably be
   eliminated. 
*/




#include <gtk/gtk.h>
#if 0
#include <liboaf/liboaf.h>
#include <orb/orbit.h>
#endif

#include "myoaf.h"



void
edit_calendar(GtkWidget *w, gpointer data)
{

#if 0
	int i;
	OAF_ServerInfo *srv;
	OAF_ServerInfoList *sl;
	char * idl, *q;
	CORBA_Environment ev;
	CORBA_ORB   oaf_orb;
	CORBA_Object cal;

	int argc=0;
	char *argv[] = {"gtt_duude", NULL};

	/* hack alert move to main(), and not here */
	if (FALSE == oaf_is_initialized())
	{
		oaf_init (argc, argv);
	}

	
printf ("duude oaf is init=%d\n", oaf_is_initialized());

	oaf_orb = oaf_orb_get();


	idl = "IDL:GNOME/Calendar/Repository:1.0";

	/* this is the evolution calandar but it crashes */
	idl = "IDL:BonoboControl/calendar-control:1.0";

	/* the gnome-pim calendar, but I can't find a server here */ 
	/* Am I supposed to be using gnorba/goad for this? 
	 * I guess so...  */
	idl = "IDL:GNOME/Calendar/RepositoryLocator:1.0";

	/* This describes three of the evolution compnenents */
	idl = "IDL:Evolution/ShellComponent:1.0";

	q = g_strconcat ("repo_ids.has ('", idl, "')", NULL);
	CORBA_exception_init (&ev);
	sl = oaf_query (q, NULL, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) 
	{
		printf (
		 "Error: can't get list: %s\n", 
		 CORBA_exception_id (&ev));
		CORBA_exception_free (&ev);
	}

printf ("duude len=%d max=%d\n", sl->_length, sl->_maximum);

	for (i=0; i<sl->_length; i++)
	{
		srv = &sl->_buffer[i];
		printf ("duude %d %p\n", i, srv);
		printf ("duude type=%s loc=%s host=%s\n", srv->server_type,
srv->location_info, srv->hostname);
	}
	
	CORBA_exception_init (&ev);

	cal = oaf_activate (q, NULL, 0, NULL, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) 
	{
	printf (
		 "Error: can't activate: %s\n", 
		 CORBA_exception_id (&ev));
		CORBA_exception_free (&ev);
	}
	
printf ("duude query = %p item=%p\n", sl, cal);
#endif
}


