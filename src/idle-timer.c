/* 
 * idle-timer.c --- detecting when the user is idle, and other timer-related tasks.
 * Originally comes from xscreensaver:
 * xscreensaver, Copyright (c) 1991-1997, 1998 aJamie Zawinski <jwz@jwz.org>
 * hacked up for use with gtt, Copyright (c) 2001 Linas Vepstas <linas@linas.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * This version is a major restructuring of Zawiski's code, for two reasons:
 * 1) we want to poll how long system's been idle, (orig code blocked)
 * 2) the gdk event loop gobbles events, so we had to tap in.
 *
 * NB the /proc/interrupts code is dead, but is being saved for 
 * resurection on some rainy day.  Ditto the XIDLE extension code.
 */


/* methods of detecting idleness:

      explicitly informed by SGI SCREEN_SAVER server event;
      explicitly informed by MIT-SCREEN-SAVER server event;
      (but we can't use these here ... since we just want 
      arbitrary timeout, not a screen-saving event)
      poll server idle time with XIDLE extension;
      select events on all windows, and note absence of recent events;
      note that /proc/interrupts has not changed in a while;
      activated by clientmessage.

   methods of detecting non-idleness:

      read events on the xscreensaver window;
      explicitly informed by SGI SCREEN_SAVER server event;
      explicitly informed by MIT-SCREEN-SAVER server event;
      select events on all windows, and note events on any of them;
      note that /proc/interrupts has changed;
      deactivated by clientmessage.

   I trust that explains why this function is a big hairy mess.

 */

# include "config.h"

/* #define DEBUG_TIMERS */

#include <errno.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>

#ifdef HAVE_XMU
# ifndef VMS
#  include <X11/Xmu/Error.h>
# else /* VMS */
#  include <Xmu/Error.h>
# endif /* VMS */
# else /* !HAVE_XMU */
/* # include "xmu.h" */
#endif /* !HAVE_XMU */

#ifdef HAVE_XIDLE_EXTENSION
#include <X11/extensions/xidle.h>
#endif /* HAVE_XIDLE_EXTENSION */

#include "idle-timer.h"

typedef struct IdleTimeoutScreen_s IdleTimeoutScreen;

/* This structure holds all the data that applies to the program as a whole,
   or to the non-screen-specific parts of the display connection.
 */
struct IdleTimeout_s 
{
  /* pointer_timeout is how often we check for pinter 
   * movements (in seconds) */
  int pointer_timeout;

  /* notice_events_timeout is how long we wait before we 
   * walk the the window tree, selecting events on new windows */
  int notice_events_timeout;

  int nscreens;
  IdleTimeoutScreen *screens;
  IdleTimeoutScreen *default_screen;	/* ...on which dialogs will appear. */

  Display *dpy;

  Bool using_xidle_extension;	   /* which extension is being used.         */
  Bool using_proc_interrupts;

  Bool scanning_all_windows;
  Bool polling_mouse_position;

  guint check_pointer_timer_id;	/* `prefs.pointer_timeout' */

  time_t dispatch_time;		   /* Time of event dispatch. */
  time_t last_activity_time;	   /* Time of last user activity. */
  time_t last_wall_clock_time;     /* Used to detect laptop suspend. */
  IdleTimeoutScreen *last_activity_screen;

  Bool emergency_lock_p;        /* Set when the wall clock has jumped
                                   (presumably due to laptop suspend) and we
                                   need to lock down right away instead of
                                   waiting for the lock timer to go off. */

};


/* This structure holds all the data that applies to the screen-specific parts
   of the display connection; if the display has multiple screens, there will
   be one of these for each screen.
 */
struct IdleTimeoutScreen_s 
{
  IdleTimeout *global;

  Screen *screen;
  // Widget toplevel_shell;

  int poll_mouse_last_root_x;		/* Used only when no server exts. */
  int poll_mouse_last_root_y;
  Window poll_mouse_last_child;
  unsigned int poll_mouse_last_mask;
};


/* ===================================================================== */
#ifdef HAVE_PROC_INTERRUPTS
static Bool proc_interrupts_activity_p (IdleTimeout *si);
#endif /* HAVE_PROC_INTERRUPTS */

static void check_for_clock_skew (IdleTimeout *si);


/* ===================================================================== */
/* This routine will install event masks on the indicated window, 
 * and its children.  These masks are designed to trigger if there's 
 * keyboard/strcuture activity in that window.
 */

static void
notice_events (IdleTimeout *si, Window window, Bool top_p)
{
  XWindowAttributes attrs;
  unsigned long events;
  Window root, parent, *kids;
  unsigned int nkids;
  GdkWindow *gwin;

  gwin = gdk_window_lookup (window);
  if (gwin && (window != DefaultRootWindow (si->dpy)))
  {
    /* If it's one of ours, don't mess up its event mask. */
    return;
  }

  if (!XQueryTree (si->dpy, window, &root, &parent, &kids, &nkids))
  {
    return;
  }
  if (window == root) top_p = False;

  XGetWindowAttributes (si->dpy, window, &attrs);
  events = ((attrs.all_event_masks | attrs.do_not_propagate_mask)
	    & KeyPressMask );

  /* Select for SubstructureNotify on all windows.
     Select for KeyPress on all windows that already have it selected.

     Note that we can't select for ButtonPress, because of X braindamage:
     only one client at a time may select for ButtonPress on a given
     window, though any number can select for KeyPress.  Someone explain
     *that* to me.

     So, if the user spends a while clicking the mouse without ever moving
     the mouse or touching the keyboard, we won't know that they've been
     active, and the screensaver will come on.  That sucks, but I don't
     know how to get around it.
   */
  XSelectInput (si->dpy, window, SubstructureNotifyMask | events);

  if (top_p && (events & KeyPressMask))
    {
      /* Only mention one window per tree  */
#ifdef DEBUG
      /* hack alert -- why does this print statment almost never go ???? */
      /* key-press events certainly *do* seem to get delivered, so they
       * must have been selected for ??? */
       printf ("duude selected KeyPress on 0x%lX\n", (unsigned long) window);
#endif
      top_p = False;
    }

  if (kids)
    {
      while (nkids)
      {
	notice_events (si, kids [--nkids], top_p);
      }
      XFree ((char *) kids);
    }
}

/* ===================================================================== */
/* stuff realted to the notice-events timer */

static int
saver_ehandler (Display *dpy, XErrorEvent *error)
{

  fprintf (stderr, "\n"
           "#######################################"
           "#######################################\n\n"
           "X Error!  PLEASE REPORT THIS BUG.\n");

  // if (XmuPrintDefaultErrorMessage (dpy, error, stderr))
    {
      fprintf (stderr, "\n");
      exit (1);
    }
  // else
    fprintf (stderr, " (nonfatal.)\n");
  return 0;
}

static int
BadWindow_ehandler (Display *dpy, XErrorEvent *error)
{
  if (error->error_code == BadWindow ||
      error->error_code == BadMatch ||
      error->error_code == BadDrawable)
    return 0;
  else
    return saver_ehandler (dpy, error);
}

struct notice_events_timer_arg {
  IdleTimeout *si;
  Window w;
};

static gint
notice_events_timer (gpointer closure)
{
  struct notice_events_timer_arg *arg =
    (struct notice_events_timer_arg *) closure;
  XErrorHandler old_handler;
  IdleTimeout *si = arg->si;
  Window window = arg->w;

  g_free(arg);

  /* When we notice a window being created, we spawn a timer that waits
     30 seconds or so, and then selects events on that window.  This error
     handler is used so that we can cope with the fact that the window
     may have been destroyed <30 seconds after it was created.
   */
  old_handler = XSetErrorHandler (BadWindow_ehandler);

  notice_events (si, window, True);
  XSync (si->dpy, False);
  XSetErrorHandler (old_handler);

  /* we pop once, and we don't pop again */
  return 0;
}


static void
start_notice_events_timer (IdleTimeout *si, Window w)
{
  struct notice_events_timer_arg *arg;

  /* we want to create a fresh one of these for every 
   * new window. */
  arg = g_new(struct notice_events_timer_arg, 1);
  arg->si = si;
  arg->w = w;
  gtk_timeout_add ( si->notice_events_timeout *1000, 
                   notice_events_timer, (gpointer) arg);
}

/* ===================================================================== */

/* When we aren't using a server extension, this timer is used to periodically
   wake up and poll the mouse position, which is possibly more reliable than
   selecting motion events on every window.
 */
static gint
check_pointer_timer (gpointer closure)
{
  int i;
  IdleTimeout *si = (IdleTimeout *) closure;
  Bool active_p = False;

  if (!si->using_proc_interrupts &&
      (si->using_xidle_extension))
    /* If an extension is in use, we should not be polling the mouse.
       Unless we're also checking /proc/interrupts, in which case, we should.
     */
    abort ();

  for (i = 0; i < si->nscreens; i++)
    {
      IdleTimeoutScreen *ssi = &si->screens[i];
      Window root, child;
      int root_x, root_y, x, y;
      unsigned int mask;

      XQueryPointer (si->dpy, RootWindowOfScreen (ssi->screen), &root, &child,
		     &root_x, &root_y, &x, &y, &mask);

      if (root_x == ssi->poll_mouse_last_root_x &&
	  root_y == ssi->poll_mouse_last_root_y &&
	  child  == ssi->poll_mouse_last_child &&
	  mask   == ssi->poll_mouse_last_mask)
	continue;

      active_p = True;

      si->last_activity_screen    = ssi;
      ssi->poll_mouse_last_root_x = root_x;
      ssi->poll_mouse_last_root_y = root_y;
      ssi->poll_mouse_last_child  = child;
      ssi->poll_mouse_last_mask   = mask;
    }

#ifdef HAVE_PROC_INTERRUPTS
  if (!active_p &&
      si->using_proc_interrupts &&
      proc_interrupts_activity_p (si))
    {
      active_p = True;
    }
#endif /* HAVE_PROC_INTERRUPTS */

  if (active_p) 
  {
    si->last_activity_time = time ((time_t *) 0);
  }

  check_for_clock_skew (si);
  return 1;
}


/* ===================================================================== */
/* An unfortunate situation is this: the
   user has been typing.  The machine is a laptop.  The user closes the lid
   and suspends it.  The CPU halts.  Some hours later, the user opens the
   lid.  At this point, timers will fire, and etc.

   So far so good -- well, not really, but it's the best that we can do,
   since the OS doesn't send us a signal *before* shutdown 

   We only do this when we'd be polling the mouse position anyway.
   This amounts to an assumption that machines with APM support also
   have /proc/interrupts.
 */
static void
check_for_clock_skew (IdleTimeout *si)
{
  time_t now = time ((time_t *) 0);

#ifdef DEBUG_TIMERS
  long shift = now - si->last_wall_clock_time;

  fprintf (stderr, "checking wall clock (%d).\n", 
             (si->last_wall_clock_time == 0 ? 0 : shift));

  if (si->last_wall_clock_time != 0 &&
      shift > (p->timeout / 1000))
    {
      fprintf (stderr, "wall clock has jumped by %ld:%02ld:%02ld!\n",
                 (shift / (60 * 60)), ((shift / 60) % 60), (shift % 60));
    }
#endif /* DEBUG_TIMERS */

  si->last_wall_clock_time = now;
}


/* ===================================================================== */

time_t
poll_last_activity (IdleTimeout *si)
{
  if (!si) return 0;

#ifdef HAVE_XIDLE_EXTENSION
    if (si->using_xidle_extension)
    {
      Time idle;
      /* The XIDLE extension uses ...
         ask the server how long the user has been idle. */
      if (! XGetIdleTime (si->dpy, &idle))
      {
         fprintf (stderr, "XGetIdleTime() failed.\n");
      }
/* hack alert fixme set the last activity time */
   }
#endif /* HAVE_XIDLE_EXTENSION */

  return si->last_activity_time;
}



/* ===================================================================== */
/* Some crap for dealing with /proc/interrupts.

   On Linux systems, it's possible to see the hardware interrupt count
   associated with the keyboard.  We can therefore use that as another method
   of detecting idleness.

   Why is it a good idea to do this?  Because it lets us detect keyboard
   activity that is not associated with X events.  For example, if the user
   has switched to another virtual console, it's good for xscreensaver to not
   be running graphics hacks on the (non-visible) X display.  The common
   complaint that checking /proc/interrupts addresses is that the user is
   playing Quake on a non-X console, and the GL hacks are perceptibly slowing
   the game...

   This is tricky for a number of reasons.

     * First, we must be sure to only do this when running on an X server that
       is on the local machine (because otherwise, we'd be reacting to the
       wrong keyboard.)  The way we do this is by noting that the $DISPLAY is
       pointing to display 0 on the local machine.  It *could* be that display
       1 is also on the local machine (e.g., two X servers, each on a different
       virtual-terminal) but it's also possible that screen 1 is an X terminal,
       using this machine as the host.  So we can't take that chance.

     * Second, one can only access these interrupt numbers in a completely
       and utterly brain-damaged way.  You would think that one would use an
       ioctl for this.  But no.  The ONLY way to get this information is to
       open the pseudo-file /proc/interrupts AS A FILE, and read the numbers
       out of it TEXTUALLY.  Because this is Unix, and all the world's a file,
       and the only real data type is the short-line sequence of ASCII bytes.

       Now it's all well and good that the /proc/interrupts pseudo-file
       exists; that's a clever idea, and a useful API for things that are
       already textually oriented, like shell scripts, and users doing
       interactive debugging sessions.  But to make a *C PROGRAM* open a file
       and parse the textual representation of integers out of it is just
       insane.

     * Third, you can't just hold the file open, and fseek() back to the
       beginning to get updated data!  If you do that, the data never changes.
       And I don't want to call open() every five seconds, because I don't want
       to risk going to disk for any inodes.  It turns out that if you dup()
       it early, then each copy gets fresh data, so we can get around that in
       this way (but for how many releases, one might wonder?)

     * Fourth, the format of the output of the /proc/interrupts file is
       undocumented, and has changed several times already!  In Linux 2.0.33,
       even on a multiprocessor machine, it looks like this:

          0:  309453991   timer
          1:    4771729   keyboard
   
       but on later kernels with MP machines, it looks like this:

                   CPU0       CPU1
          0:    1671450    1672618    IO-APIC-edge  timer
          1:      13037      13495    IO-APIC-edge  keyboard

       Joy!  So how are we expected to parse that?  Well, this code doesn't
       parse it: it saves the last line with the string "keyboard" in it, and
       does a string-comparison to note when it has changed.

   Thanks to Nat Friedman <nat@nat.org> for figuring out all of this crap.

   Note that this only checks for lines with "keyboard" or "PS/2 Mouse" in
   them.  If you have a serial mouse, it won't detect that, it will only detect
   keyboard activity.  That's because there's no way to tell the difference
   between a serial mouse and a general serial port, and it would be somewhat
   unfortunate to have the screensaver turn off when the modem on COM1 burped.
 */


#ifdef HAVE_PROC_INTERRUPTS

#define PROC_INTERRUPTS "/proc/interrupts"

Bool
query_proc_interrupts_available (IdleTimeout *si, const char **why)
{
  /* We can use /proc/interrupts if $DISPLAY points to :0, and if the
     "/proc/interrupts" file exists and is readable.
   */
  FILE *f;
  if (why) *why = 0;

  if (!display_is_on_console_p (si))
    {
      if (why) *why = "not on primary console";
      return False;
    }

  f = fopen (PROC_INTERRUPTS, "r");
  if (!f)
    return False;

  fclose (f);
  return True;
}


/* ===================================================================== */
static Bool
proc_interrupts_activity_p (IdleTimeout *si)
{
  static FILE *f0 = 0;
  FILE *f1 = 0;
  int fd;
  static char last_kbd_line[255] = { 0, };
  static char last_ptr_line[255] = { 0, };
  char new_line[sizeof(last_kbd_line)];
  Bool got_kbd = False, kbd_diff = False;
  Bool got_ptr = False, ptr_diff = False;

  if (!f0)
    {
      /* First time -- open the file. */
      f0 = fopen (PROC_INTERRUPTS, "r");
      if (!f0)
        {
          char buf[255];
          sprintf(buf, "%s: error opening %s", blurb(), PROC_INTERRUPTS);
          perror (buf);
          goto FAIL;
        }
    }

  if (f0 == (FILE *) -1)	    /* means we got an error initializing. */
    return False;

  fd = dup (fileno (f0));
  if (fd < 0)
    {
      char buf[255];
      sprintf(buf, "%s: could not dup() the %s fd", blurb(), PROC_INTERRUPTS);
      perror (buf);
      goto FAIL;
    }

  f1 = fdopen (fd, "r");
  if (!f1)
    {
      char buf[255];
      sprintf(buf, "%s: could not fdopen() the %s fd", blurb(),
              PROC_INTERRUPTS);
      perror (buf);
      goto FAIL;
    }

  /* Actually, I'm unclear on why this fseek() is necessary, given the timing
     of the dup() above, but it is. */
  if (fseek (f1, 0, SEEK_SET) != 0)
    {
      char buf[255];
      sprintf(buf, "%s: error rewinding %s", blurb(), PROC_INTERRUPTS);
      perror (buf);
      goto FAIL;
    }

  /* Now read through the pseudo-file until we find the "keyboard" line. */

  while (fgets (new_line, sizeof(new_line)-1, f1))
    {
      if (!got_kbd && strstr (new_line, "keyboard"))
        {
          kbd_diff = (*last_kbd_line && !!strcmp (new_line, last_kbd_line));
          strcpy (last_kbd_line, new_line);
          got_kbd = True;
        }
      else if (!got_ptr && strstr (new_line, "PS/2 Mouse"))
        {
          ptr_diff = (*last_ptr_line && !!strcmp (new_line, last_ptr_line));
          strcpy (last_ptr_line, new_line);
          got_ptr = True;
        }

      if (got_kbd && got_ptr)
        break;
    }

  if (got_kbd || got_ptr)
    {
      fclose (f1);
      return (kbd_diff || ptr_diff);
    }


  /* If we got here, we didn't find either a "keyboard" or a "PS/2 Mouse"
     line in the file at all. */
  fprintf (stderr, "%s: no keyboard or mouse data in %s?\n",
           blurb(), PROC_INTERRUPTS);

 FAIL:
  if (f1)
    fclose (f1);

  if (f0 && f0 != (FILE *) -1)
    fclose (f0);

  f0 = (FILE *) -1;
  return False;
}

#endif /* HAVE_PROC_INTERRUPTS */

/* ===================================================================== */
/* This routine gets called whenever there's an event sitting on the 
 * X input queue.  In principle, we should remove any event we asked
 * for that gdk wasn't expecting.  In practice, this is hard, cpu-consuming
 * and dangerous. So we let gdk weed hem out itself.
 */

static Bool
if_event_predicate (Display *dpy, XEvent *ev, XPointer arg)
{
  IdleTimeout *si = (IdleTimeout *) arg;
#ifdef DEBUG
  printf ("duude event type %d\n", ev->xany.type);
#endif
  switch (ev->xany.type) 
  {
    case KeyPress:
    case KeyRelease:
    case ButtonPress:
    case ButtonRelease:
    case MotionNotify:
    case EnterNotify:
    case LeaveNotify:
      si->last_activity_time = si->dispatch_time;
      break;

    case CreateNotify:
      /* A window has been created on the screen somewhere.  If we're
         supposed to scan all windows for events, prepare this window.
         Don't reset the timer on this event. The xscreensaver typically
         generates one of these every 10 minutes ('phosphor' mode), so
         we shouldn't mistake this for user activity.
       */
      if (si->scanning_all_windows)
      {
        Window w = ev->xcreatewindow.window;
        start_notice_events_timer (si, w);
      }
      break;

    default:
      /* We assume all other messages might occur 'spontaneously', without
       * the activity of a user, and therefore, we ignore them. */
      break;
  }

  return False;
}

/* ===================================================================== */
/* We need to take over the main loop, in order to be able to capture
 * X Events before gdk eats them.   So we select on the X socket to 
 * make sure we get them first.  We dispatch gtk at least once a second,
 * so that other timers & etc. get properly handled.
 */

static int
idle_timeout_main_loop (gpointer data)
{
  IdleTimeout *si = data;
  int quit_now = 0;
  fd_set rfds;
  struct timeval tv;
  int fd;
  int retval;

  FD_ZERO (&rfds);
  fd = ConnectionNumber (si->dpy);

  while (!quit_now)
  {
     FD_SET (fd, &rfds);

     /* block and wait one sec at most */
     tv.tv_sec = 1;
     tv.tv_usec = 0;
     retval = select(fd+1, &rfds, NULL, NULL, &tv);
     if ((0 > retval) && (EAGAIN != errno) && (EINTR != errno))
     {
        printf ("GTT: fatal error in select %d, %s\n", errno, strerror(errno));
        gtk_main_quit();
        return 0;
     }

     /* Monitor X input queue */
     {
        XEvent event;
        si->dispatch_time = time (0);
        XCheckIfEvent (si->dpy, &event, if_event_predicate, (XPointer) si);
     }

     /* Clear out pending gtk events before we got back to sleep */
     while (gtk_events_pending())
     {
        quit_now = gtk_main_iteration_do (FALSE);
        if (quit_now) return 0;
     }
  }
  return 0;
}


/* ===================================================================== */

IdleTimeout *
idle_timeout_new (void)
{
  IdleTimeout *si;

  si = g_new0 (IdleTimeout, 1);
  si->dpy = GDK_DISPLAY();

  /* hack alert xxx we should grep all screens */
  si->nscreens = 1;
  si->screens = g_new0 (IdleTimeoutScreen, 1);
  si->screens->global = si;
  si->screens->screen = DefaultScreenOfDisplay (si->dpy);

  /* how often we observe mouse  */
  si->pointer_timeout = 5;

  /* how soon we select the window tree after a new windows created */
  si->notice_events_timeout = 30;

  /* ------------------------------------------------------------- */
  /* We need to select events on all windows if we're not using any extensions.
     Otherwise, we don't need to. */
  si->scanning_all_windows = !(si->using_xidle_extension);

  /* Whether we need to periodically wake up and check to see if the mouse has
     moved.  We only need to do this when not using any extensions.  The reason
     this isn't the same as `polling_for_idleness' is that the "idleness" poll
     can happen (for example) 5 minutes from now, whereas the mouse-position
     poll should happen with low periodicity.  We don't need to poll the mouse
     position with the XIDLE extension, but we do need to periodically wake up
     and query the server with that extension.  For our purposes, polling
     /proc/interrupts is just like polling the mouse position.  It has to
     happen on the same kind of schedule. */
  si->polling_mouse_position = (si->using_proc_interrupts ||
                              !(si->using_xidle_extension));

  if (si->polling_mouse_position)
  {
    /* Check to see if the mouse has moved, and set up a repeating timer
       to do so periodically (typically, every 5 seconds.) */
    si->check_pointer_timer_id = gtk_timeout_add (si->pointer_timeout *1000, 
                  check_pointer_timer, (gpointer) si);

    /* run it once, to initialze stuff */
    check_pointer_timer ((gpointer) si);
  }

  /* ------------------------------------------------------------- */

  if (si->scanning_all_windows)
  { 
    /* get events from every window */
    notice_events (si, DefaultRootWindow(si->dpy), True);
  
    /* hijack the main loop; this is the only way to get events */
    gtk_timeout_add (900, idle_timeout_main_loop, (gpointer)si);
  }

  return si;
}

/* =================== END OF FILE ===================================== */
