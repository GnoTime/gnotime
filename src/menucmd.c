/*   Main menu callbacks for app main menubar for GTimeTracker 
 *   Copyright (C) 1997,98 Eckehard Berns
 *   Copyright (C) 2001,2002,2003 Linas Vepstas <linas@linas.org>
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

#include <config.h>
#include <gnome.h>
#include <string.h>

#include "app.h"
#include "ctree.h"
#include "ctree-gnome2.h"
#include "cur-proj.h"
#include "dialog.h"
#include "err-throw.h"
#include "file-io.h"
#include "gtt.h"
#include "menucmd.h"
#include "menus.h"
#include "prefs.h"
#include "proj.h"
#include "props-proj.h"
#include "timer.h"
#include "toolbar.h"
#include "xml-gtt.h"


void
about_box(GtkWidget *w, gpointer data)
{
	static GtkWidget *about = NULL;
	const gchar *authors[] = {
		  "Linas Vepstas <linas@linas.org>",
		  "Eckehard Berns <eb@berns.i-s-o.net>",
		  "George Lebl <jirka@5z.com>",
		  " ",
		  _("Bug-fixes from:"), 
		  "Eric Anderson <eric.anderson@cordata.net>",
		  "Derek Atkins <warlord@mit.edu>",
		  "Jonathan Blandford  <jrb@redhat.com>",
		  "Miguel de Icaza  <miguel@nuclecu.unam.mx>",
		  "John Fleck <jfleck@inkstain.net>",
		  "Nat Friedman  <nat@nat.org>",
		  "Mark Galassi  <rosalia@cygnus.com>",
		  "Jeff Garzik  <jgarzik@pobox.com>",
		  "Sven M. Hallberg <pesco@gmx.de>",
		  "Raja R Harinath  <harinath@cs.umn.edu>",
		  "Peter Hawkins <peterhawkins@ozemail.com.au>",
		  "Egil Kvaleberg <egil@kvaleberg.no>",
		  "Chris Lahey  <clahey@umich.edu>",
		  "Gregory McLean <gregm@comstar.net>",
		  "Kjartan Maraas  <kmaraas@gnome.org>",
		  "Federico Mena Quintero  <federico@nuclecu.unam.mx>",
		  "Tomas Ogren  <stric@ing.umu.se>",
		  "Gediminas Paulauskas <menesis@delfi.lt>",
		  "Havoc Pennington  <hp@pobox.com>",
		  "Ettore Perazzoli  <ettore@comm2000.it>",
		  "Changwoo Ryu  <cwryu@adam.kaist.ac.kr>",
		  "Pablo Saratxaga <srtxg@chanae.alphanet.ch>",
		  "Carsten Schaar  <nhadcasc@fs-maphy.uni-hannover.de>",
		  "Mark Stosberg <mark@summersault.com>",
		  "Tom Tromey  <tromey@cygnus.com>",
		  "Sebastian Wilhelmi  <wilhelmi@ira.uka.de>",
		  NULL
	};


	const gchar *documentors[] = {
		  "Eckehard Berns <eb@berns.i-s-o.net>",
		  "Linas Vepstas <linas@linas.org>",
		  NULL
	};

	const gchar *translators = 
		"Vasif Ismailoglu MD <azerb_linux@hotmail.com> - az\n"
		"Borislav Aleksandrov <B.Aleksandrov@cnsys.bg> - bg\n"
		"Ivan Vilata i Balaguer <al011097@alumail.uji.es> - ca\n"
		"Softcatala <tradgnome@softcatala.org> - ca\n"
		"David Sauer <davids@penguin.cz> - cz\n"
		"George Lebl <jirka@5z.com> - cz\n"
		"Kenneth Christiansen <kenneth@ripen.dk> - da\n"
		"Kim Schulz <kim@schulz.dk> - da\n"
		"Birger Langkjer <birger.langkjer@image.dk> - da\n"
		"Keld Simonsen <keld@dkuug.dk> - da\n"
		"Ole Laursen <olau@hardworking.dk> - da\n"
		"Matthias Warkus <mawa@iname.com> - de\n"
		"Keld Simonsen <keld@dkuug.dk> - da\n"
		"Karl Eichwalder <ke@suse.de> - de\n"
		"Benedikt Roth <Benedikt.Roth@gmx.net> - de\n"
		"Christian Meyer <chrisime@gnome.org> - de\n"
		"Spiros Papadimitriou <spapadim+@cs.cmu.edu> - el\n"
		"Simos Xenitellis <simos@hellug.gr> - el\n"
		"Robert Brady <rwb197@ecs.soton.ac.uk> - en_GB\n"
	   "Pablo Saratxaga <srtxg@chanae.alphanet.ch> - es\n"
		"Manuel de Vega Barreiro <mbarreiro@red.madritel.es> - es\n"
		"Lauris Kaplinski <lauris@ariman.ee> - et\n"
		"Josu Wali*o <josu@elhuyar.com> - eu\n"
		"Antti Ahvensalmi <aahven@mbnet.fi> - fi\n"
		"Tuomas Meril - fi\n"
		"Vincent Renardias <vincent@ldsol.com> - fr\n"
		"Thibaut Cousin <cousin@clermont.in2p3.fr> - fr\n"
		"Christophe Merlet (RedFox) <redfox@eikonex.org> - fr\n"
		"Bretin Didier <didier@bretin.net> - fr\n"
		"Christophe Fergeau <teuf@users.sourceforge.net> - fr\n"
		"Sean Ceallaigh <s_oceallaigh@yahoo.com> - ga\n"
		"Jesus Bravo Alvarez <jba@pobox.com> - gl\n"
		"Ruben Lopez Gomez <ryu@mundivia.es> - gl\n"
		"Szabolcs BAN <shooby@gnome.hu> - hu\n"
		"Gergely Nagy <greg@gnome.hu> - hu\n"
		"Andras Timar <timar@gnome.hu> - hu\n"
		"Giovanni Bortolozzo <borto@pluto.linux.it> - it\n"
		"Eiichiro ITANI <emu@ceres.dti.ne.jp> - jp\n"
		"Yuusuke Tahara <tahara@gnome.gr.jp> - jp\n"
		"Takayuki KUSANO <AE5T-KSN@asahi-net.or.jp> - jp\n"
		"Changwoo Ryu <cwryu@adam.kaist.ac.kr> - ko\n"
		"Young-Ho, Cha <ganadist@chollian.net> - ko\n"
		"Gediminas Paulauskas <menesis@delfi.lt> - lt\n"
		"Peteris Kriajlnis <peterisk@apollo.lv> - lv\n"
		"Artis Trops <hornet@navigators.lv> - lv\n"
		"Mohamad Afifi Omar (App) <mr_mohd_afifi@yahoo.com> - ms\n"
		"Hasbullah BIn Pit <sebol@ikhlas.com> - ms\n"
		"Mendel mobach <mendel@mobach.nl> - nl\n"
		"Almer S. Tigelaar <almer@gnome.org> - nl\n"
		"Marc Maurer <j.m.maurer@student.utwente.nl> - nl\n"
		"Dennis Smit <synap@area101.penguin.nl> - nl\n"
		"Kjartan Maraas  <kmaraas@gnome.org> - nn\n"
		"Roy-Magne Mo <rmo@sunnmore.net> - nn\n"
		"Kjartan Maraas <kmaraas@gnome.org> - no\n"
		"Egil Kvaleberg <egil@kvaleberg.no> - no\n"
		"GNOME PL Team <translators@gnome.pl> - pl\n"
		"Nuno Ferreira  <nmrf@rnl.ist.utl.pt> - pt\n"
		"Duarte Loreto <happyguy_pt@hotmail.com> - pt\n"
		"Alexandre Hautequest <hquest@fesppr.br> - pt_BR\n"
		"Ariel Bressan da Silva <ariel@conectiva.com.br> - pt_BR\n"
		"Francisco Petrecio Cavalcante Junior <fpcj@impa.br> - pt\n"
		"Iustin Pop <iusty@geocities.com> - ro\n"
		"Dan Damian <dand@dnttm.ro> - ro\n"
		"Valek Filppov <frob@df.ru> - ru\n"
		"Sergey Panov <sipan@mit.edu> - ru\n"
		"Stanislav Visnovsky <visnovsky@nenya.ms.mff.cuni.cz> - sk\n"
		"Andraz Tori <andraz.tori1@guest.arnes.si> - sl\n"
		"Martin Norbeck <d95mback@dtek.chalmers.se> - sv\n"
		"Andreas Hyden <a.hyden@cyberpoint.se> - sv\n"
		"Christian Rose <menthos@menthos.com> - sv\n"
		"Dinesh Nadarajah <n_dinesh@yahoo.com> - ta\n"
		"Fatih Demir <kabalak@gmx.net> - tr\n"
		"Gorkem etin <gorkem@gelecek.com.tr> - tr\n"
		"Yuri Syrota <rasta@renome.rovno.ua> - ua\n"
		"pclouds <pclouds@gmx.net> - vi\n"
		"Pablo Saratxaga <srtxg@chanae.alphanet.ch> - wa\n"
		"Wang Li <charles@linux.net.cn> - zh_CN\n"
		"Ming-Yen Hsu <myhsu@cyberdude.com> - zh_TW\n"
		"Abel Cheung  <maddog@linux.org.hk> - zh_TW.Big5\n"
	;


	if (about != NULL)
	{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return;
	}
	about = gnome_about_new(GTT_APP_TITLE,
				    VERSION,
				    "Copyright (C) 1997,98 Eckehard Berns\n"
				    "Copyright (C) 2001,2002,2003 Linas Vepstas",
#ifdef DEBUG
				    __DATE__ ", " __TIME__,
#else
 _("GnoTime is a combination stop-watch, diary, consultant billing "
   "system and project manager.  You can measure the amount of time you "
   "spend on a task, associate a memo with it, set a billing rate, print "
   "an invoice, as well as track the status of other projects."),

#endif
				    authors,
				    documentors,
				    translators,
				    NULL);
	g_signal_connect(G_OBJECT(about), "destroy",
		         G_CALLBACK(gtk_widget_destroyed), &about);
	gtk_widget_show(about);
}




static void
project_name_desc(GtkWidget *w, GtkEntry **entries)
{
	const char *name, *desc;
	GttProject *proj;
	GttProject *sib_prj;
	
	sib_prj = ctree_get_focus_project (global_ptw);

	if (!(name = gtk_entry_get_text(entries[0]))) return;
	if (!(desc = gtk_entry_get_text(entries[1]))) return;
	if (!name[0]) return;

	/* New project will have the same parent as the currently
	 * running project.  This seems like the sanest choice.
	 */
	proj = gtt_project_new_title_desc(name, desc);
	gtt_project_insert_after (proj, sib_prj);
	ctree_insert_after (global_ptw, proj, sib_prj);
}

static void
free_data(GtkWidget *dlg, gpointer data)
{
	g_free(data);
}


void
new_project(GtkWidget *widget, gpointer data)
{
	GtkWidget *dlg, *t, *title, *d, *desc;
	GtkBox *vbox;
	GtkWidget **entries = g_new0(GtkWidget *, 2);
	GtkWidget *table;

	title = gnome_entry_new("project_title");
	desc = gnome_entry_new("project_description");
	entries[0] = gnome_entry_gtk_entry(GNOME_ENTRY(title));
	entries[1] = gnome_entry_gtk_entry(GNOME_ENTRY(desc));

	new_dialog_ok_cancel(_("New Project..."), &dlg, &vbox,
			     GTK_STOCK_OK,
			     G_CALLBACK(project_name_desc),
				 entries,
			     GTK_STOCK_CANCEL, NULL, NULL);

	t = gtk_label_new(_("Project Title"));
	d = gtk_label_new(_("Description"));

	table = gtk_table_new(2,2, FALSE);
	gtk_table_attach(GTK_TABLE(table), t,     0,1, 0,1,
			    GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 2, 1);
	gtk_table_attach(GTK_TABLE(table), title, 1,2, 0,1,
			    GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 2, 1);
	gtk_table_attach(GTK_TABLE(table), d,     0,1, 1,2,
			    GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 2, 1);
	gtk_table_attach(GTK_TABLE(table), desc,  1,2, 1,2,
			    GTK_FILL|GTK_EXPAND, GTK_FILL|GTK_EXPAND, 2, 1);

	gtk_box_pack_start(vbox, table, FALSE, FALSE, 2);
	gtk_widget_show(t);
	gtk_widget_show(title);
	gtk_widget_show(d);
	gtk_widget_show(desc);
	gtk_widget_show(table);

	gtk_widget_grab_focus(entries[0]);

	/* enter in first entry goes to next */
	g_signal_connect_object (G_OBJECT (entries[0]), "activate",
				   G_CALLBACK (gtk_widget_grab_focus),
				   GTK_OBJECT (entries[1]), 0);
	gnome_dialog_editable_enters(GNOME_DIALOG(dlg),
				     GTK_EDITABLE(entries[1]));

	g_signal_connect(G_OBJECT(dlg), "destroy",
			   G_CALLBACK(free_data),
			   entries);
	
	gtk_widget_show(dlg);
}

/* ======================================================= */


GttProject *cutted_project = NULL;

void
cut_project(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = ctree_get_focus_project (global_ptw);

	if (!prj) return;
	if (cutted_project)
	{
		/* Wipe out whatever was previously in our cut buffer. */
		gtt_project_destroy(cutted_project);
	}
	
	cutted_project = prj;

	/* Clear out relevent GUI elements. */
	prop_dialog_set_project(NULL);

	if (cutted_project == cur_proj) ctree_stop_timer (cur_proj);
	gtt_project_remove(cutted_project);
	ctree_remove(global_ptw, cutted_project);

	/* Update various subsystems */
	/* Set the notes are to whatever the new focus project is. */
	prj = ctree_get_focus_project (global_ptw);
	notes_area_set_project (global_na, prj);
						 
	menu_set_states();      /* to enable paste menu item */
	toolbar_set_states();
}



void
paste_project(GtkWidget *w, gpointer data)
{
	GttProject *sib_prj;
	GttProject *p, *focus_prj;
	
	sib_prj = ctree_get_focus_project (global_ptw);

	if (!cutted_project) return;
	p = cutted_project;

	/* if we paste a second time, we better paste a copy ... */
	cutted_project = gtt_project_dup(cutted_project);

	/* insert before the focus proj */
	gtt_project_insert_before (p, sib_prj);

	if (!sib_prj) 
	{
		/* top-level insert */
		ctree_add(global_ptw, p, NULL);
		return;
	}
	ctree_insert_before(global_ptw, p, sib_prj);

	/* Set the notes are to whatever the new focus project is. 
	 * (which should be 'p', bbut we play it safe to avoid
	 * weird inconsistent state.) */
	focus_prj = ctree_get_focus_project (global_ptw);
	notes_area_set_project (global_na, focus_prj);
}



void
copy_project(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = ctree_get_focus_project (global_ptw);

	if (!prj) return;

	if (cutted_project) 
	{
		gtt_project_destroy(cutted_project);
	}
	cutted_project = gtt_project_dup(prj);
	
	/* Update various subsystems */
	menu_set_states();      /* to enable paste menu item */
	toolbar_set_states();
}




/*
 * timer related menu functions
 */

void
menu_start_timer(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = ctree_get_focus_project (global_ptw);
	ctree_start_timer (prj);
}



void
menu_stop_timer(GtkWidget *w, gpointer data)
{
	ctree_stop_timer (cur_proj);
}


void
menu_toggle_timer(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = ctree_get_focus_project (global_ptw);

	if (timer_is_running()) {
		ctree_stop_timer (cur_proj);
	} else {
		ctree_start_timer (prj);
	}
}


void
menu_options(GtkWidget *w, gpointer data)
{
	prefs_dialog_show();
}



void
menu_properties(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = ctree_get_focus_project (global_ptw);

	if (prj) {
		prop_dialog_show(prj);
	}
}



void
menu_clear_daily_counter(GtkWidget *w, gpointer data)
{
	GttProject *prj;
	prj = ctree_get_focus_project (global_ptw);

	gtt_clear_daily_counter (prj);
	ctree_update_label(global_ptw, prj);
}

/* ============================ END OF FILE ======================= */
