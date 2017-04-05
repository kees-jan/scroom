/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
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

const std::string COLORMAPPABLE_PROPERTY_NAME="Colormappable";
const std::string MONOCHROME_COLORMAPPABLE_PROPERTY_NAME="Monochrome Colormappable";
const std::string TRANSPARENT_BACKGROUND_PROPERTY_NAME="Transparent Background";

inline uint8_t byteFromDouble(double d) { return uint8_t(255*d); }
inline double doubleFromByte(uint8_t b) { return b/255.0; }

#define GCC_VERSION (__GNUC__ * 10000           \
                     + __GNUC_MINOR__ * 100     \
                     + __GNUC_PATCHLEVEL__)

#if GCC_VERSION < 40800  // 4.8.0
#define NORETURN __attribute__((noreturn))  // precise, gcc 4.6.3
#else
#define NORETURN [[noreturn]] // others // gcc 4.8.4
#endif

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
  explicit Color(double gray, double alpha=1.0)
    : alpha(alpha), red(gray), green(gray), blue(gray)
  {}

  Color& operator+=(const Color& rhs)
  { alpha += rhs.alpha; red += rhs.red; green += rhs.green; blue+= rhs.blue; return *this; }

  Color& operator/=(double d)
  { alpha /= d; red /= d; green /= d; blue /= d; return *this; }

  Color& operator*=(double d)
  { alpha *= d; red *= d; green *= d; blue *= d; return *this; }

  uint32_t getRGB24() const
  { return 0xFF000000 | byteFromDouble(red)<<16 | byteFromDouble(green)<<8 | byteFromDouble(blue) <<0; }

  uint32_t getARGB24() const
  { return byteFromDouble(alpha)<<24 | byteFromDouble(red)<<16 | byteFromDouble(green)<<8 | byteFromDouble(blue) <<0; }

  void setColor(cairo_t* cr) const
  { cairo_set_source_rgba(cr, red, green, blue, alpha); }
  
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
    : name("Empty")
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

  Ptr clone() const
  {
    return Ptr(new Colormap(*this));
  }

  void setAlpha(double alpha)
  {
    BOOST_FOREACH(Color& c, colors)
      c.setAlpha(alpha);
  }

  Ptr setAlpha(double alpha) const
  {
    Ptr result=clone();
    result->setAlpha(alpha);
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
class Colormappable
{
public:
  typedef boost::shared_ptr<Colormappable> Ptr;
  typedef boost::weak_ptr<Colormappable> WeakPtr;
  
  /** Virtual destructor */
  virtual ~Colormappable() {}

  /** Request that the presentation use the given colormap */
  virtual void setColormap(Colormap::Ptr colormap)=0;

  /** Retrieve the images colormap (if any) */
  virtual Colormap::Ptr getOriginalColormap()=0;

  /** Retrieve the number of colors in use by the presentation */
  virtual int getNumberOfColors()=0;

  /**
   * @name For monochrome presentations: Set/Get the current color
   * @{
   */
  virtual Color getMonochromeColor()=0;
  virtual void setMonochromeColor(const Color& c)=0;
  /** @} */

  /**
   * @name Manipulate the "Transparent Background" setting of the presentation
   * @{
   */
  virtual void setTransparentBackground()=0;
  virtual void disableTransparentBackground()=0;
  virtual bool getTransparentBackground()=0;
  /** @} */
};

class ColormapProvider
{
public:
  typedef boost::shared_ptr<ColormapProvider> Ptr;
  
public:
  virtual Colormap::Ptr getColormap()=0;

  virtual ~ColormapProvider() {}
};

class ColormapHelperBase : public ColormapProvider, public Colormappable
{
public:
  typedef boost::shared_ptr<ColormapHelperBase> Ptr;
  
public:
  Colormap::Ptr colormap;
  Colormap::Ptr originalColormap;
  
public:
  ColormapHelperBase(Colormap::Ptr const& colormap);
  
  ////////////////////////////////////////////////////////////////////////
  // Colormappable
  ////////////////////////////////////////////////////////////////////////
  virtual void setColormap(Colormap::Ptr colormap);
  virtual Colormap::Ptr getOriginalColormap();
  virtual int getNumberOfColors();
  virtual Color getMonochromeColor();
  virtual void setMonochromeColor(const Color& c);
  virtual void setTransparentBackground();
  virtual void disableTransparentBackground();
  virtual bool getTransparentBackground();

  ////////////////////////////////////////////////////////////////////////
  // ColormapProvider
  ////////////////////////////////////////////////////////////////////////
  virtual Colormap::Ptr getColormap();

  ////////////////////////////////////////////////////////////////////////
  // Helpers
  ////////////////////////////////////////////////////////////////////////
  virtual void setOriginalColormap(Colormap::Ptr colormap);

private:
  NORETURN void OperationNotSupported();
};

class ColormapHelper : public ColormapHelperBase
{
public:
  static Ptr create(int numberOfColors);
  static Ptr createInverted(int numberOfColors);
  static Ptr create(Colormap::Ptr const& colormap);

private:
  ColormapHelper(Colormap::Ptr const& colormap);
};

class MonochromeColormapHelper : public ColormapHelperBase
{
public:
  static Ptr create(int numberOfColors);
  static Ptr createInverted(int numberOfColors);

  ////////////////////////////////////////////////////////////////////////
  // Colormappable
  ////////////////////////////////////////////////////////////////////////

  virtual Color getMonochromeColor();
  virtual void setMonochromeColor(const Color& c);
  // virtual void setTransparentBackground();
  // virtual void disableTransparentBackground();
  // virtual bool getTransparentBackground();

private:
  MonochromeColormapHelper(int numberOfColors, bool inverted);
  void regenerateColormap();
  static Colormap::Ptr generateInitialColormap(int numberOfColors, bool inverted);

private:
  int numberOfColors;
  bool inverted;
  Color blackish;
  Color whitish;
};

#endif
