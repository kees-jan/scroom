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
#include <string>

#include <boost/shared_ptr.hpp>

#include <scroom/presentationinterface.hh>
#include <scroom/observable.hh>

#define COLORMAPPABLE_PROPERTY_NAME "Colormappable"

/**
 * Represent a (RGB) color
 */
class Color
{
public:
  double red;    /**< R value */
  double green;  /**< G value */
  double blue;   /**< B value */

public:

  /** Default constructor. Create black */
  Color()
    : red(0.0), green(0.0), blue(0.0)
  {}

  /** Constructor. Use the given RGB values */
  Color(double red, double green, double blue)
    : red(red), green(green), blue(blue)
  {}

  /** Constructor. Create the given gray value */
  Color(double gray)
    : red(gray), green(gray), blue(gray)
  {}
};

/**
 * Represent a colormap
 */
class Colormap
{
public:
  typedef boost::shared_ptr<Colormap> Ptr;
  typedef boost::shared_ptr<const Colormap> ConstPtr;
  typedef boost::weak_ptr<Colormap> WeakPtr;

public:
  std::string name;             /**< Name of this colormap */
  std::vector<Color> colors;    /**< Colors in this colormap */

private:
  /** Constructor. Create an empty colormap */
  Colormap()
    : name("Empty"), colors()
  {}

public:
  /** Constructor. Create a smart pointer to an empty colormap */
  static Colormap::Ptr create()
  {
    return Colormap::Ptr(new Colormap());
  }

  /** Constructor. Create a smart pointer to a colormap of @c n grays. */
  static Colormap::Ptr createDefault(int n)
  {
    Colormap::Ptr result=create();
    result->name="Default";
    result->colors.reserve(n);
    result->colors.clear();
    double max = n-1;
    for(int i=0; i<n; i++)
      result->colors.push_back(Color((max-i)/max));  // Min is white

    return result;
  }
};

/**
 * Interface for Colormappable presentations.
 *
 * In order to use the colormap plugin, presentations should implement
 * this interface, and define the @c COLORMAPPABLE_PROPERTY_NAME
 * property.
 */
 
class Colormappable: public Observable<Viewable>
{
public:
  /** Virtual destructor */
  virtual ~Colormappable() {}

  /** Request that the presentation use the given colormap */
  virtual void setColormap(Colormap::Ptr colormap)=0;

  /** Retrieve the currently used colormap */
  virtual Colormap::Ptr getColormap()=0;

  /** Retrieve the number of colors in use by the presentation */
  virtual int getNumberOfColors()=0;
};
#endif
