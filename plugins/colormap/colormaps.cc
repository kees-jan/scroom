#include "colormaps.hh"

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <gio/gio.h>

#define SCROOMDIR   ".scroom"
#define COLORMAPDIR "colormaps"
#define COLORMAPEXT ".pal"

// void on_dir_changed (GFileMonitor *monitor, GFile *file, GFile *other_file, GFileMonitorEvent event_type, gpointer user_data)
// {
//   GFileInfo* fileInfo = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
//                                           G_FILE_QUERY_INFO_NONE, NULL, NULL);
//   GFileInfo* otherFileInfo = g_file_query_info(file, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
//                                                G_FILE_QUERY_INFO_NONE, NULL, NULL);
//   const char* fileName = g_file_info_get_attribute_string(fileInfo, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
//   const char* otherFileName = g_file_info_get_attribute_string(otherFileInfo, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
//   
//   printf("--> Dir changed: %s - %s, %d\n",fileName, otherFileName,event_type);
// }

Colormaps::Colormaps()
{
  const char *homedir = g_getenv ("HOME");
  if (!homedir)
     homedir = g_get_home_dir ();

  const char* colormapDirPath = g_build_filename(homedir, SCROOMDIR, COLORMAPDIR, NULL);

  filenames = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);

  DIR* colormapDir = opendir(colormapDirPath);
  if(colormapDir)
  {
    for(struct dirent* d=readdir(colormapDir); d; d=readdir(colormapDir))
    {
      if(d->d_type==DT_REG)
      {
        char* name = d->d_name;
        int len = strlen(name);
        if(!strcmp(name+len-strlen(COLORMAPEXT), COLORMAPEXT))
        {
          Colormap::WeakPtr* c = new Colormap::WeakPtr(); 

          GtkTreeIter iter;
          gtk_list_store_append(filenames, &iter);
          gtk_list_store_set(filenames, &iter,
                             COLUMN_NAME, name,
                             COLUMN_POINTER, c,
                             -1);
        }
      }
    }
    closedir(colormapDir);
  }
  else
  {
    printf("Failed to open dir %s: (%d, %s)\n", colormapDirPath, errno, strerror(errno));
  }

  // For now, read the directory once. Later, we can use gio and watch
  // for updates and stuff.

  //   printf("--> Attempting to monitor dir %s\n", colormapDirPath);
  //   
  //   // GFileMonitor
  //   GError* err = NULL;
  //   GFile* colormapDir = g_file_new_for_path(colormapDirPath);
  //   GFileMonitor* colormapDirMonitor = g_file_monitor_directory(colormapDir, G_FILE_MONITOR_NONE, NULL, &err);
  //   if(err!=NULL)
  //   {
  //     printf("PANIC: Can't monitor directory: %s\n", err->message);
  //     g_error_free(err);
  //     err=NULL;
  //   }
  //   printf("--> Successfully created the monitor!\n");
  //   g_signal_connect ((gpointer) colormapDirMonitor, "changed", G_CALLBACK (on_dir_changed), NULL);
  // 
  //   // Maak een GHashTable die GFile's mapt op Colormap::Ptr's
}

Colormaps::~Colormaps()
{
}

Colormaps& Colormaps::getInstance()
{
  static Colormaps instance;
  return instance;
}

GtkListStore* Colormaps::getFileNames()
{
  return filenames;
}

void Colormaps::select(GtkTreeIter iter, PresentationInterface::Ptr p)
{
  printf("-->Attempting to change the selection!\n");
}
