/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "colormaps.hh"

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <dirent.h>
#include <spdlog/spdlog.h>
#include <sys/types.h>

#include <gio/gio.h>

#ifdef _WIN32
#  include <Windows.h>
#endif

#include <scroom/colormappable.hh>

#define SCROOMDIR ".scroom"
#define COLORMAPDIR "colormaps"
#define COLORMAPEXT ".pal"

#define PAL_HEADER "JASC-PAL"
#define PAL_VERSION "0100"
#define BUFFERSIZE 256

namespace Scroom::ColormapImpl
{
  ////////////////////////////////////////////////////////////////////////
  // Colormaps

  Colormaps::Colormaps()
  {
    char* colormapDirPath = getColormapDirPath();
    DIR*  colormapDir     = opendir(colormapDirPath);
    if(colormapDir)
    {
      for(struct dirent* d = readdir(colormapDir); d; d = readdir(colormapDir))
      {
        // At this point, we're not sure if d is a regular file or a
        // directory or whatever. However, we can safely assume that in
        // all bad cases, the load() will fail :-)

        char*     name = d->d_name;
        const int len  = strlen(name);
        if(!strcmp(name + len - strlen(COLORMAPEXT), COLORMAPEXT))
        {
          Colormap::Ptr const c = load(name);
          if(c)
          {
            colormaps.emplace_back(c);
          }
        }
      }
      closedir(colormapDir);
    }
    else
    {
      spdlog::info("Failed to open dir {}: ({}, {})", colormapDirPath, errno, strerror(errno));
    }
    g_free(colormapDirPath);
  }

  Colormaps& Colormaps::getInstance()
  {
    static Colormaps instance;
    return instance;
  }

  char* Colormaps::getColormapDirPath()
  {
#ifdef _WIN32
    // We want to keep everything portable on windows so we look for the colormaps folder in the same directory as the .exe
    char buffer[2048];
    GetModuleFileName(NULL, buffer, 2047);
    std::string modulePath(buffer);
    auto        pos = modulePath.rfind("\\");
    return g_build_filename(modulePath.substr(0, pos).c_str(), "\\", COLORMAPDIR, NULL);
#else
    const char* homedir = g_getenv("HOME");
    if(!homedir)
    {
      homedir = g_get_home_dir();
    }

    return g_build_filename(homedir, SCROOMDIR, COLORMAPDIR, NULL);
#endif
  }

  std::list<Colormap::ConstPtr> Colormaps::getColormaps() { return colormaps; }

  Colormap::Ptr Colormaps::load(const char* name)
  {
    Colormap::Ptr colormap;

    char* fullName = g_build_filename(getColormapDirPath(), name, NULL);
    FILE* f        = fopen(fullName, "re");
    if(f)
    {
      try
      {
        char buffer[BUFFERSIZE];

        // Read header
        char* result = fgets(buffer, BUFFERSIZE, f);
        if(!result)
        {
          throw std::exception();
        }
        if(0 != strncmp(buffer, PAL_HEADER, strlen(PAL_HEADER)))
        {
          throw std::exception();
        }

        // Read version
        result = fgets(buffer, BUFFERSIZE, f);
        if(!result)
        {
          throw std::exception();
        }
        if(0 != strncmp(buffer, PAL_VERSION, strlen(PAL_VERSION)))
        {
          throw std::exception();
        }

        // Read ColorCount
        result = fgets(buffer, BUFFERSIZE, f);
        if(!result)
        {
          throw std::exception();
        }
        unsigned const int count = atoi(result);
        if(count == 0)
        {
          throw std::exception();
        }

        colormap                   = Colormap::create();
        colormap->name             = name;
        std::vector<Color>& colors = colormap->colors;
        int                 red    = 0;
        int                 green  = 0;
        int                 blue   = 0;
        while(colors.size() < count && fgets(buffer, BUFFERSIZE, f) && 3 == sscanf(buffer, "%d %d %d", &red, &green, &blue))
        {
          colors.emplace_back(red / 255.0, green / 255.0, blue / 255.0);
        }
        if(colors.size() != count)
        {
          throw std::exception();
        }
      }
      catch(std::exception& e)
      {
        spdlog::error("Couldn't parse file");
        colormap.reset();
      }
      fclose(f);
    }
    g_free(fullName);
    return colormap;
  }
} // namespace Scroom::ColormapImpl
