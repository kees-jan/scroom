#include "colormaps.hh"

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <gio/gio.h>

#include <colormappable.hh>

#define SCROOMDIR   ".scroom"
#define COLORMAPDIR "colormaps"
#define COLORMAPEXT ".pal"

#define PAL_HEADER  "JASC-PAL"
#define PAL_VERSION "0100"
#define BUFFERSIZE  256

////////////////////////////////////////////////////////////////////////
// Colormaps


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

  char* colormapDirPath = g_build_filename(homedir, SCROOMDIR, COLORMAPDIR, NULL);

  filenames = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_BOOLEAN);

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
                             COLUMN_ENABLED, true,
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
  g_free(colormapDirPath);

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

void Colormaps::select(GtkTreeIter iter, Colormappable* colormappable)
{
  if(gtk_list_store_iter_is_valid(filenames, &iter))
  {
    gchar* name = NULL;
    gpointer* pointer = NULL;
    gtk_tree_model_get(GTK_TREE_MODEL(filenames), &iter, COLUMN_NAME, &name, COLUMN_POINTER, &pointer, -1);
    Colormap::WeakPtr* w = (Colormap::WeakPtr*)pointer;
    Colormap::Ptr c = w->lock();
    if(!c)
    {
      c = load(name);
      if(c)
      {
        *w = c;
      }
      else
      {
        gtk_list_store_set(filenames, &iter, COLUMN_ENABLED, false, -1);
      }
    }
    if(c)
      colormappable->setColormap(c);

    g_free(name);
  }
}

Colormap::Ptr Colormaps::load(const char* name)
{
  Colormap::Ptr colormap;

  const char *homedir = g_getenv ("HOME");
  if (!homedir)
     homedir = g_get_home_dir ();

  char* fullName = g_build_filename(homedir, SCROOMDIR, COLORMAPDIR, name, NULL); 
  FILE* f = fopen(fullName, "r");
  if(f)
  {
    try
    {
      char buffer[BUFFERSIZE];

      // Read header
      char* result = fgets(buffer, BUFFERSIZE, f);
      if(!result)
        throw std::exception();
      if(strncmp(buffer, PAL_HEADER, strlen(PAL_HEADER)))
        throw std::exception();
        
      // Read version
      result = fgets(buffer, BUFFERSIZE, f);
      if(!result)
        throw std::exception();
      if(strncmp(buffer, PAL_VERSION, strlen(PAL_VERSION)))
        throw std::exception();

      // Read ColorCount
      result = fgets(buffer, BUFFERSIZE, f);
      if(!result)
        throw std::exception();
      unsigned int count = atoi(result);
      if(count==0)
        throw std::exception();

      colormap = Colormap::create();
      std::vector<Color>& colors = colormap->colors;
      int red=0;
      int green=0;
      int blue=0;
      while(colors.size()<count &&
            fgets(buffer, BUFFERSIZE, f) &&
            3 == sscanf(buffer, "%d %d %d", &red, &green, &blue))
      {
        colors.push_back(Color(red/255.0, green/255.0, blue/255.0));
      }
      if(colors.size()!=count)
        throw std::exception();
    }
    catch(std::exception& e)
    {
      printf("ERROR: Couldn't parse file\n");
      colormap.reset();
    }
    fclose(f);
  }
  g_free(fullName);
  return colormap;
}
