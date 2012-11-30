
/* This file contains sample/experimental hacking code for a gnomevfs client.
 * it allows playing with the API.  It shows how to use the GnomeVFS API
 * in a client such as GnoTime.
 *
 * This is not a part of the standard GnoTime distribution, its just for hacking around.
 */
/* ------------------------------------------------------------------ */
#include <sys/types.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs.h>

/* See http://developer.gnome.org/doc/API/2.0/gnome-vfs-2.0/
 * for a good set of API documentation for GnomeVFS clients.
 *
 * See http://www-106.ibm.com/developerworks/linux/library/l-gnvfs/?ca=dnt-435
 * for an example on how to write a gnomevfs server aka 'module'
 *
 * http://www.stafford.uklinux.net/libesmtp/
 */

void
gtt_save_buffer (const gchar *uri, const char * buf)
{
	GnomeVFSHandle   *handle;
	GnomeVFSResult    result;

	GnomeVFSURI *parsed_uri;
	parsed_uri = gnome_vfs_uri_new (uri);

	gboolean exists = gnome_vfs_uri_exists (parsed_uri);
	if (exists)
	{
		// file exists are you sure you want to over-write?
	}

	// result = gnome_vfs_open (&handle, uri, GNOME_VFS_OPEN_WRITE);
	result = gnome_vfs_create (&handle, uri, GNOME_VFS_OPEN_WRITE,
	                 FALSE, 0644);

	if (GNOME_VFS_OK != result)
	{
		const gchar * errmsg = gnome_vfs_result_to_string (result);
printf ("duuude pen error=%s\n", errmsg);
		return;
	}

	GnomeVFSFileSize buflen = strlen (buf);
	GnomeVFSFileSize bytes_written = 0;
	size_t off = 0;
	while (GNOME_VFS_OK == result)
	{
		result = gnome_vfs_write (handle, &buf[off],
		                         buflen, &bytes_written);
		off += bytes_written;
		buflen -= bytes_written;

		printf ("duude wrote %lld bytes left=%d\n", bytes_written, buflen);
		if (0>= buflen) break;
	}
	if (GNOME_VFS_OK != result)
	{
		const gchar * errmsg = gnome_vfs_result_to_string (result);
printf ("duuude write error=%s\n", errmsg);
		return;
	}
	gnome_vfs_close (handle);
}

#define BUF_SIZE 8192

void
load_gorp (const gchar *uri)
{
	GnomeVFSHandle   *handle;
	GnomeVFSResult    result;
	gchar             buffer[BUF_SIZE];
	GnomeVFSFileSize  bytes_read;  /* actually a long long */

	result = gnome_vfs_open (&handle, uri, GNOME_VFS_OPEN_READ);

printf ("duude attempt to read %s\n", uri);
	while (GNOME_VFS_OK == result)
	{
		result = gnome_vfs_read (handle, buffer,
		                         BUF_SIZE, &bytes_read);

		printf ("duude got %lld %s\n", bytes_read, buffer);
	}
	gnome_vfs_close (handle);
}

int
main (int argc, char **argv)
{
	if (argc < 2) {
	         g_print ("Run with %s <uri>\n", argv[0]);
	          exit (1);
	}

	 gnome_vfs_init ();

	 // load_gorp (argv[1]);
	 char *msg="qewrtyuiop duuuude";
	 gtt_save_buffer (argv[1], msg);

	  return 0;
}
/* ------------------------------------------------------------------ */


/*
 To compile the example:

# gcc `pkg-config --libs --cflags gtk+-2.0 gnome-vfs-2.0` j.c

*/
