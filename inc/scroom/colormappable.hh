/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <scroom/assertions.hh>
#include <scroom/color.hh>
#include <scroom/observable.hh>
#include <scroom/presentationinterface.hh>

const std::string COLORMAPPABLE_PROPERTY_NAME            = "Colormappable";
const std::string MONOCHROME_COLORMAPPABLE_PROPERTY_NAME = "Monochrome Colormappable";
const std::string TRANSPARENT_BACKGROUND_PROPERTY_NAME   = "Transparent Background";

/**
 * Represent a colormap
 */
class Colormap
{
public:
  using Ptr      = boost::shared_ptr<Colormap>;
  using ConstPtr = boost::shared_ptr<const Colormap>;
  using WeakPtr  = boost::weak_ptr<Colormap>;

public:
  std::string        name;   /**< Name of this colormap */
  std::vector<Color> colors; /**< Colors in this colormap */

private:
  /** Constructor. Create an empty colormap */
  Colormap()
    : name("Empty")
  {
  }

public:
  /** Constructor. Create a smart pointer to an empty colormap */
  static Colormap::Ptr create() { return Ptr(new Colormap()); }

  /** Constructor. Create a smart pointer to a colormap of @c n grays. */
  static Colormap::Ptr createDefault(int n)
  {
    Colormap::Ptr result = create();
    result->name         = "Default";
    result->colors.reserve(n);
    result->colors.clear();
    const double max = n - 1;
    for(int i = 0; i < n; i++)
    {
      result->colors.emplace_back(i / max); // Min is black
    }

    return result;
  }

  /** Constructor. Create a smart pointer to a colormap of @c n grays. */
  static Colormap::Ptr createDefaultInverted(int n)
  {
    Colormap::Ptr result = create();
    result->name         = "0 is white";
    result->colors.reserve(n);
    result->colors.clear();
    const double max = n - 1;
    for(int i = 0; i < n; i++)
    {
      result->colors.emplace_back((max - i) / max); // Min is white
    }

    return result;
  }

  [[nodiscard]] Ptr clone() const { return Ptr(new Colormap(*this)); }

  void setAlpha(double alpha)
  {
    for(Color& c: colors)
    {
      c.setAlpha(alpha);
    }
  }

  [[nodiscard]] Ptr setAlpha(double alpha) const
  {
    Ptr result = clone();
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
class Colormappable : private Interface
{
public:
  using Ptr     = boost::shared_ptr<Colormappable>;
  using WeakPtr = boost::weak_ptr<Colormappable>;

  /** Request that the presentation use the given colormap */
  virtual void setColormap(Colormap::Ptr colormap) = 0;

  /** Retrieve the images colormap (if any) */
  virtual Colormap::Ptr getOriginalColormap() = 0;

  /** Retrieve the number of colors in use by the presentation */
  virtual int getNumberOfColors() = 0;

  /**
   * @name For monochrome presentations: Set/Get the current color
   * @{
   */
  virtual Color getMonochromeColor()               = 0;
  virtual void  setMonochromeColor(const Color& c) = 0;
  /** @} */

  /**
   * @name Manipulate the "Transparent Background" setting of the presentation
   * @{
   */
  virtual void setTransparentBackground()     = 0;
  virtual void disableTransparentBackground() = 0;
  virtual bool getTransparentBackground()     = 0;
  /** @} */
};

class ColormapProvider : private Interface
{
public:
  using Ptr = boost::shared_ptr<ColormapProvider>;

public:
  virtual Colormap::Ptr getColormap() = 0;
};

class ColormapHelperBase
  : public ColormapProvider
  , public Colormappable
{
public:
  using Ptr = boost::shared_ptr<ColormapHelperBase>;

public:
  Colormap::Ptr colormap;
  Colormap::Ptr originalColormap;

public:
  explicit ColormapHelperBase(Colormap::Ptr const& colormap);
  virtual std::map<std::string, std::string> getProperties() = 0;

  ////////////////////////////////////////////////////////////////////////
  // Colormappable
  ////////////////////////////////////////////////////////////////////////
  void          setColormap(Colormap::Ptr colormap) override;
  Colormap::Ptr getOriginalColormap() override;
  int           getNumberOfColors() override;
  Color         getMonochromeColor() override;
  void          setMonochromeColor(const Color& c) override;
  void          setTransparentBackground() override;
  void          disableTransparentBackground() override;
  bool          getTransparentBackground() override;

  ////////////////////////////////////////////////////////////////////////
  // ColormapProvider
  ////////////////////////////////////////////////////////////////////////
  Colormap::Ptr getColormap() override;

  ////////////////////////////////////////////////////////////////////////
  // Helpers
  ////////////////////////////////////////////////////////////////////////
  virtual void setOriginalColormap(Colormap::Ptr colormap);

private:
  [[noreturn]] static void OperationNotSupported();
};

class ColormapHelper : public ColormapHelperBase
{
public:
  static Ptr create(int numberOfColors);
  static Ptr createInverted(int numberOfColors);
  static Ptr create(Colormap::Ptr const& colormap);

  std::map<std::string, std::string> getProperties() override;

private:
  explicit ColormapHelper(Colormap::Ptr const& colormap);
};

class MonochromeColormapHelper : public ColormapHelperBase
{
public:
  static Ptr create(int numberOfColors);
  static Ptr createInverted(int numberOfColors);

  std::map<std::string, std::string> getProperties() override;

  ////////////////////////////////////////////////////////////////////////
  // Colormappable
  ////////////////////////////////////////////////////////////////////////

  Color getMonochromeColor() override;
  void  setMonochromeColor(const Color& c) override;
  void  setTransparentBackground() override;
  void  disableTransparentBackground() override;
  bool  getTransparentBackground() override;

private:
  MonochromeColormapHelper(int numberOfColors, bool inverted);
  void                 regenerateColormap();
  static Colormap::Ptr generateInitialColormap(int numberOfColors, bool inverted);

private:
  int   numberOfColors;
  bool  inverted;
  Color blackish;
  Color whitish;
  bool  transparentBackground{false};
};
