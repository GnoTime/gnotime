
/*
 * stub routines for maybe some future gnotime vfs module

To Build:
     cc vfs-module.c -c -fPIC  \
   `pkg-config --cflags glib-2.0 gnome-vfs-module-2.0`

   cc vfs-module.o -shared -Wl,-soname,-libgtt.so -o libgtt.so \
      `pkg-config --libs glib-2.0 gnome-vfs-module-2.0`
                                                                                                                  
	install into /usr/lib/gnome-vfs-2.0/modules/libgtt.so
And also: install conf file in /etc/gnome-vfs-2.0/modules

	To Test:
	/usr/bin/gnomevfs-ls gtt:/
	/usr/bin/gnomevfs-cat gtt:/file-duude-5


 */

//  #include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-module.h>

#define MYSTR "ho ho ho and a bottle of rum\n"

/* ------------------------------------------------------------- */
/* dir ops */

static GnomeVFSResult
gtt_opendir (GnomeVFSMethod           *method,
             GnomeVFSMethodHandle    **method_handle,
             GnomeVFSURI              *uri,
             GnomeVFSFileInfoOptions   options,
             GnomeVFSContext          *context)
{
	static int x=0;


	*method_handle  = (GnomeVFSMethodHandle *) &x;

	//return GNOME_VFS_ERROR_NOT_FOUND;
	return GNOME_VFS_OK;
}

static GnomeVFSResult
gtt_readdir (GnomeVFSMethod       *method,
             GnomeVFSMethodHandle *method_handle,
             GnomeVFSFileInfo     *file_info,
             GnomeVFSContext      *context)
{
	int *x = (int *) method_handle;
	
	printf ("readdir duude x=%d\n", *x);
	(*x)++;
	if (10 < *x) 
	{
		*x = 0;
		return GNOME_VFS_ERROR_EOF;
	}

	 if (0) {
      file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
      file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
      file_info->mime_type = g_strdup ("x-directory/normal");
      file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

   } else {
      file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
      file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
      file_info->mime_type = g_strdup ("text/plain");
      file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
      file_info->size = strlen (MYSTR);
      file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_SIZE;

		file_info->name = g_strdup_printf ("file-duude-%d", *x);
   }
	
	return GNOME_VFS_OK;
}

static GnomeVFSResult
gtt_closedir (GnomeVFSMethod       *method,
              GnomeVFSMethodHandle *method_handle,
              GnomeVFSContext      *context)
{
	int *x = (int *) method_handle;
	*x = 0;

   return GNOME_VFS_OK;
}

static gboolean
gtt_is_local (GnomeVFSMethod    *method,
              const GnomeVFSURI *uri)
{
   return TRUE;
}

/* ------------------------------------------------------------- */
/* file ops */

static GnomeVFSResult
gtt_file_info (GnomeVFSMethod          *method,
               GnomeVFSURI             *uri,
               GnomeVFSFileInfo        *file_info,
               GnomeVFSFileInfoOptions  options,
               GnomeVFSContext         *context)
{
   /* Root directory. */
   file_info->name = g_strdup ("GnoTime");
#if 0
   } else {
      file_info->name = g_strdup (name);
   }
    
   if (->directory) {
      file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
      file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
      file_info->mime_type = g_strdup ("x-directory/normal");
      file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
    } else {
      file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
      file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
      file_info->mime_type = g_strdup ("text/plain");
      file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
      file_info->size = strlen (MYSTR);
      file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_SIZE;
   }
#endif
    
   return GNOME_VFS_OK;
}

static GnomeVFSResult
gtt_open (GnomeVFSMethod        *method,
          GnomeVFSMethodHandle **method_handle,
          GnomeVFSURI           *uri,
          GnomeVFSOpenMode       mode,
          GnomeVFSContext       *context)
{
	static int l=0;
	*method_handle = (GnomeVFSMethodHandle *) &l;

   return GNOME_VFS_OK;
}

static GnomeVFSResult
gtt_read (GnomeVFSMethod       *method,
          GnomeVFSMethodHandle *method_handle,
          gpointer              buffer,
          GnomeVFSFileSize      bytes,
          GnomeVFSFileSize     *bytes_read,
          GnomeVFSContext      *context)
{
	int *l = (int *) method_handle;

	if (1 == *l) { *l=0; *bytes_read=0; return GNOME_VFS_ERROR_EOF; }
   *bytes_read = MIN (bytes, strlen (MYSTR));

   memcpy (buffer, MYSTR, *bytes_read);
	*l = 1;

   return GNOME_VFS_OK;
}

static GnomeVFSResult
gtt_close (GnomeVFSMethod       *method,
           GnomeVFSMethodHandle *method_handle,
           GnomeVFSContext      *context)
{
   return GNOME_VFS_OK;
}

/* ------------------------------------------------------------- */
/* module setup */

static GnomeVFSMethod gtt_method = 
{
	sizeof (GnomeVFSMethod),
	open_directory:  gtt_opendir,
	close_directory:  gtt_closedir,
	read_directory:  gtt_readdir,
	
	is_local:   gtt_is_local,
	
	open:  gtt_open,
	read:  gtt_read,
	close:  gtt_close,
};


GnomeVFSMethod *
vfs_module_init (const char * method_name, 
                 const char * args)
{
	printf ("duude called with name=%s args=%s\n", method_name, args);

	if (0 == strcmp (method_name, "gtt"))
	{
		return &gtt_method;
	} 
	return NULL;
}


void 
vfs_module_shutdown (GnomeVFSMethod* method)
{
}
