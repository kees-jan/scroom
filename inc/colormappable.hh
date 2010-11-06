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

#ifndef COLORMAPPABLE_HH
#define COLORMAPPABLE_HH

#include <vector>

#include <boost/shared_ptr.hpp>

#include <presentationinterface.hh>
#include <observable.hh>

#define COLORMAPPABLE_PROPERTY_NAME "Colormappable"

class Color
{
public:
  double red;
  double green;
  double blue;

public:
  Color()
    : red(0.0), green(0.0), blue(0.0)
  {}
  
  Color(double red, double green, double blue)
    : red(red), green(green), blue(blue)
  {}
  
  Color(double gray)
    : red(gray), green(gray), blue(gray)
  {}
};

class Colormap
{
public:
  typedef boost::shared_ptr<Colormap> Ptr;
  typedef boost::weak_ptr<Colormap> WeakPtr;

public:
  std::vector<Color> colors;

public:
  static Colormap::Ptr create()
  {
    return Colormap::Ptr(new Colormap());
  }

  static Colormap::Ptr createDefault(int n)
  {
    Colormap::Ptr result=create();
    result->colors.reserve(n);
    result->colors.clear();
    double max = n-1;
    for(int i=0; i<n; i++)
      result->colors.push_back(Color((max-i)/max));  // Min is white

    return result;
  }
};

class Colormappable: public Observable<Viewable>
{
public:
  ~Colormappable() {}
  
  virtual void setColormap(Colormap::Ptr colormap)=0;
};

#endif
