/*   GConf2 input/output handling for GTimeTracker - a time tracker
 *   Copyright (C) 2003 Linas Vepstas <linas@linas.org>
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

/* This is a PRIVATE header file for GConf users only! Do not use! */

#ifndef GTT_GCONF_IO_P_H_
#define GTT_GCONF_IO_P_H_

#include <gconf/gconf-client.h>
#include <glib.h>

/* ======================================================= */
/* XXX Should use GConfChangesets */
/* XXX warnings should be graphical */
#define CHKERR(rc,err_ret,dir) {                                       \
   if (FALSE == rc) {                                                  \
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

#define F_SETINT(dir,val) {                                            \
   gboolean rc;                                                        \
   GError *err_ret= NULL;                                              \
                                                                       \
   rc = gconf_client_set_int (client, dir, val, &err_ret);             \
   CHKERR (rc,err_ret,dir);                                            \
}

#define SETINT(dir,val) F_SETINT (GTT_GCONF dir, val)

#define F_SETSTR(dir,val) {                                            \
   gboolean rc;                                                        \
   GError *err_ret= NULL;                                              \
                                                                       \
   const gchar *sval = val;                                            \
   if (!sval) sval = "";                                               \
   rc = gconf_client_set_string (client, dir, sval, &err_ret);         \
   CHKERR (rc,err_ret,dir);                                            \
}

#define SETSTR(dir,val) F_SETSTR (GTT_GCONF dir, val)

#define SETLIST(dir,tipe,val) {                                        \
   gboolean rc;                                                        \
   GError *err_ret= NULL;                                              \
                                                                       \
   rc = gconf_client_set_list (client, GTT_GCONF dir, tipe, val, &err_ret);  \
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

#define CHKGET(gcv,err_ret,dir,default_val)                            \
   if ((NULL == gcv) || (FALSE == GCONF_VALUE_TYPE_VALID(gcv->type))) {\
      retval = default_val;                                            \
      printf ("GTT: GConf: Warning: get %s failed: ", dir);            \
      if (err_ret) printf ("%s\n\t", err_ret->message);                \
      printf ("Using default value\n");                                \
   }

#define GETBOOL(dir,default_val) ({                                    \
   gboolean retval;                                                    \
   GError *err_ret= NULL;                                              \
   GConfValue *gcv;                                                    \
   gcv = gconf_client_get (client, GTT_GCONF dir, &err_ret);           \
   CHKGET (gcv,err_ret, dir, default_val)                              \
   else retval = gconf_value_get_bool (gcv);                           \
   retval;                                                             \
})

#define F_GETINT(dir,default_val) ({                                   \
   int retval;                                                         \
   GError *err_ret= NULL;                                              \
   GConfValue *gcv;                                                    \
   gcv = gconf_client_get (client, dir, &err_ret);                     \
   CHKGET (gcv,err_ret, dir, default_val)                              \
   else retval = gconf_value_get_int (gcv);                            \
   retval;                                                             \
})

#define GETINT(dir,default_val) F_GETINT (GTT_GCONF dir, default_val)

#define F_GETLIST(dir,default_val) ({                                  \
   GSList *retval;                                                     \
   GError *err_ret= NULL;                                              \
   GConfValue *gcv;                                                    \
   gcv = gconf_client_get (client, dir, &err_ret);                     \
   CHKGET (gcv,err_ret, dir, default_val)                              \
   else retval = gconf_value_get_list (gcv);                           \
   retval;                                                             \
})

#define GETLIST(dir,default_val) F_GETLIST (GTT_GCONF dir,  default_val)

#define F_GETSTR(dir,default_val) ({                                   \
   const char *retval;                                                 \
   GError *err_ret= NULL;                                              \
   GConfValue *gcv;                                                    \
   gcv = gconf_client_get (client, dir, &err_ret);                     \
   CHKGET (gcv,err_ret, dir, default_val)                              \
   else retval = gconf_value_get_string (gcv);                         \
   g_strdup (retval);                                                  \
})

#define GETSTR(dir,default_val) F_GETSTR (GTT_GCONF dir, default_val)

/* Convert list of GConfValue to list of the actual values */
#define GETINTLIST(dir) ({                                             \
   GSList *l,*n;                                                       \
   l = GETLIST(dir, NULL);                                             \
   for (n=l; n; n=n->next) {                                           \
      /* XXX mem leak?? do we need to free gconf value  ?? */          \
      n->data = (gpointer) (long) gconf_value_get_int (n->data);       \
   }                                                                   \
   l;                                                                  \
})

#endif /* GTT_GCONF_IO_P_H_ */
