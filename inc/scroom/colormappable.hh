/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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

#include <stdint.h>

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/operators.hpp>

#include <scroom/presentationinterface.hh>
#include <scroom/observable.hh>

#define COLORMAPPABLE_PROPERTY_NAME "Colormappable"

namespace
{
  uint8_t byteFromDouble(double d) { return uint8_t(255*d); }
}

/**
 * Represent a (ARGB) color
 */
class Color : public boost::addable<Color>, public boost::multiplicative<Color,double>
{
public:
  double alpha;  /**< Alpha value */
  double red;    /**< R value */
  double green;  /**< G value */
  double blue;   /**< B value */

public:

  /** Default constructor. Create black */
  Color()
    : alpha(1.0), red(0.0), green(0.0), blue(0.0)
  {}

  /** Constructor. Use the given RGB values */
  Color(double red, double green, double blue, double alpha=1.0)
    : alpha(alpha), red(red), green(green), blue(blue)
  {}

  /** Constructor. Create the given gray value */
  Color(double gray, double alpha=1.0)
    : alpha(alpha), red(gray), green(gray), blue(gray)
  {}

  Color& operator+=(const Color& rhs)
  { alpha += rhs.alpha; red += rhs.red; green += rhs.green; blue+= rhs.blue; return *this; }

  Color& operator/=(double d)
  { alpha /= d; red /= d; green /= d; blue /= d; return *this; }

  Color& operator*=(double d)
  { alpha *= d; red *= d; green *= d; blue *= d; return *this; }

  uint32_t getRGB24()
  { return 0xFF000000 | byteFromDouble(red)<<16 | byteFromDouble(green)<<8 | byteFromDouble(blue) <<0; }

  uint32_t getARGB24()
  { return byteFromDouble(alpha)<<24 | byteFromDouble(red)<<16 | byteFromDouble(green)<<8 | byteFromDouble(blue) <<0; }

  Color& setAlpha(double alpha)
  { return *this *= alpha; }

  Color setAlpha(double alpha) const
  { return Color(*this).setAlpha(alpha); }
};

inline Color mix(const Color& a, const Color& b, double alpha)
{
  return a*alpha + b*(1.0-alpha);
}

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
    return Ptr(new Colormap());
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
      result->colors.push_back(Color(i/max));  // Min is black

    return result;
  }

  /** Constructor. Create a smart pointer to a colormap of @c n grays. */
  static Colormap::Ptr createDefaultInverted(int n)
  {
    Colormap::Ptr result=create();
    result->name="0 is white";
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
 *
 * Presentations should forward view creation and destruction events
 * (i.e. Viewable::open() and Viewable::close() events) to their
 * observers. As a result, new views will get a sidebar containing the
 * available colormaps, and when one is selected, setColormap() will
 * be called.
 */
class Colormappable: public virtual Scroom::Utils::Observable<Viewable>
{
public:
  typedef boost::shared_ptr<Colormappable> Ptr;
  typedef boost::weak_ptr<Colormappable> WeakPtr;
  
  /** Virtual destructor */
  virtual ~Colormappable() {}

  /** Request that the presentation use the given colormap */
  virtual void setColormap(Colormap::Ptr colormap)=0;

  // /**
  //  * Request that the presentation uses the given alpha value, in
  //  * addition to any alpha that may be present in the Colormap, for the given view only.
  //  */
  // virtual void setAlpha(ViewInterface::WeakPtr const& vi, double alpha)=0;

  /** Retrieve the images colormap (if any) */
  virtual Colormap::Ptr getOriginalColormap()=0;

  /** Retrieve the number of colors in use by the presentation */
  virtual int getNumberOfColors()=0;
};

class ColormapProvider
{
public:
  virtual Colormap::Ptr getColormap()=0;

  virtual ~ColormapProvider() {}
};

#endif
