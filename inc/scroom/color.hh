/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cstdint>
#include <cmath>

#include <boost/operators.hpp>

#include <gdk/gdk.h>
#include <cairo.h>

inline uint8_t byteFromDouble(double d) { return uint8_t(255*d); }
inline double doubleFromByte(uint8_t b) { return b/255.0; }
inline uint16_t shortFromDouble(double d) { return uint16_t(0xFFFF*d); }

namespace
{
  // see http://stackoverflow.com/a/3943023
  double computeC(double c)
  {
    return c<=0.03928?c/12.92:pow((c+0.055)/1.055, 2.4);
  }
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
  Color(double red_, double green_, double blue_, double alpha_=1.0)
    : alpha(alpha_), red(red_), green(green_), blue(blue_)
  {}

  /** Constructor. Create the given gray value */
  explicit Color(double gray, double alpha_=1.0)
    : alpha(alpha_), red(gray), green(gray), blue(gray)
  {}

  Color& operator+=(const Color& rhs)
  { alpha += rhs.alpha; red += rhs.red; green += rhs.green; blue+= rhs.blue; return *this; }

  Color& operator/=(double d)
  { alpha /= d; red /= d; green /= d; blue /= d; return *this; }

  Color& operator*=(double d)
  { alpha *= d; red *= d; green *= d; blue *= d; return *this; }

  uint32_t getRGB24() const {
    uint32_t r = byteFromDouble(red);
    uint32_t g = byteFromDouble(green);
    uint32_t b = byteFromDouble(blue);
    return 0xFF000000 | r << 16 | g << 8 | b;
  }

  uint32_t getARGB32() const {
    uint32_t a = byteFromDouble(alpha);
    uint32_t r = byteFromDouble(red);
    uint32_t g = byteFromDouble(green);
    uint32_t b = byteFromDouble(blue);
    return a << 24 | r << 16 | g << 8 | b;
  }

  void setColor(cairo_t* cr) const
  { cairo_set_source_rgba(cr, red, green, blue, alpha); }

  GdkColor getGdkColor() const
  {
    return { 0, shortFromDouble(red), shortFromDouble(green), shortFromDouble(blue) };
  }

  Color getContrastingBlackOrWhite() const
  {
    // see http://stackoverflow.com/a/3943023
    double L = 0.2126 * computeC(red) + 0.7152 * computeC(green) + 0.0722 * computeC(blue);
    return Color(L>0.179?0:1);
  }

  Color& setAlpha(double alpha_)
  { return *this *= alpha_; }

  Color setAlpha(double alpha_) const
  { return Color(*this).setAlpha(alpha_); }
};

inline Color mix(const Color& a, const Color& b, double alpha)
{
  return a*alpha + b*(1.0-alpha);
}

