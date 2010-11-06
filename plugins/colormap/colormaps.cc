/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

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

Colormaps::Colormaps()
{
  const char *homedir = g_getenv ("HOME");
  if (!homedir)
     homedir = g_get_home_dir ();

  char* colormapDirPath = g_build_filename(homedir, SCROOMDIR, COLORMAPDIR, NULL);

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
          Colormap::Ptr c = load(name);
          if(c)
          {
            colormaps.push_back(c);
          }          
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
}

Colormaps::~Colormaps()
{
}

Colormaps& Colormaps::getInstance()
{
  static Colormaps instance;
  return instance;
}

std::list<Colormap::ConstPtr> Colormaps::getColormaps()
{
  return colormaps;
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
      colormap->name = name;
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
