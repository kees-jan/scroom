#pragma once

#include <boost/shared_ptr.hpp>

#include <gtk/gtk.h>

#include <scroom/interface.hh>

/**
 * The Ruler class employs the Strategy pattern to draw the horizontal and vertical ruler.
 * Implementations of this interface contain the code to draw the parts of the ruler that
 * need to be oriented differently.
 */
class RulerDrawStrategyInterface : private Interface
{
public:
  using Ptr = boost::shared_ptr<RulerDrawStrategyInterface>;

  /**
   * Returns the length of the major tick lines, given the ruler dimensions.
   * @param width Allocated width for the ruler in pixels.
   * @param height Allocated height for the ruler in pixels.
   * @param percentage Percentage of the /p width or /p height to be the length of the major ticks.
   * @return The length of the major tick lines in pixels.
   */
  virtual double getMajorTickLength(double percentage) = 0;

  /**
   * Returns the length in pixels along the ruler.
   * For a horizontal ruler, this is its width.
   * For a vertical ruler, this is its height.
   * @param width Allocated width for the ruler in pixels.
   * @param height Allocated height for the ruler in pixels.
   * @return The length in pixels along the ruler in pixels.
   */
  virtual double getDrawAreaSize() = 0;


  /**
   * Draws the outline around the ruler.
   * @param cr Cairo context to draw to.
   * @param lineWidth Width of the outline in pixels.
   * @param width Allocated width for the ruler in pixels.
   * @param height Allocated height for the ruler in pixels.
   */
  virtual void drawOutline(cairo_t* cr, double lineWidth) = 0;

  /**
   * Draws the line for a ruler tick.
   * @param cr Cairo context to draw to.
   * @param linePosition Position of the line along the ruler in pixels.
   * @param lineWidth Width of the tick line in pixels.
   * @param lineLength Length of the tick line in pixels.
   * @param width Allocated width for the ruler in pixels.
   * @param height Allocated height for the ruler in pixels.
   */
  virtual void drawTickLine(cairo_t* cr, double linePosition, double lineWidth, double lineLength) = 0;

  /**
   * Draws the label to the right of a tick line.
   * @param cr Cairo context to draw to.
   * @param label Text to draw to right of tick line.
   * @param linePosition Position of the line along the ruler in pixels.
   * @param labelOffset Offset between tick line and label in pixels.
   * @param labelAlign Alignment of label along tick line as a fraction of line length.
   * @param lineLength Length of the tick line in pixels.
   * @param width Allocated width for the ruler in pixels.
   * @param height Allocated height for the ruler in pixels.
   */
  virtual void drawTickText(cairo_t*           cr,
                            const std::string& label,
                            double             linePosition,
                            double             labelOffset,
                            double             labelAlign,
                            double             lineLength) = 0;

  /**
   * Sets the allocated newWidth and height for the ruler.
   * @param newWidth Allocated newWidth for the ruler in pixels.
   * @param height Allocated height for the ruler in pixels.
   */
  void setAllocatedSize(int newWidth, int height);

protected:
  static constexpr double LINE_COORD_OFFSET{0.5};
  static constexpr double TEXT_ANCHOR{0.5};

  /** Allocated width for the ruler in pixels. */
  int width;
  /** Allocated height for the ruler in pixels. */
  int height;
};

/**
 * This class implements the drawing code for a horizontal ruler.
 */
class HorizontalDrawStrategy : public RulerDrawStrategyInterface
{
public:
  /**
   * Creates a new HorizontalDrawStrategy.
   * @return A new HorizontalDrawStrategy.
   */
  static RulerDrawStrategyInterface::Ptr create();

  double getMajorTickLength(double percentage) override;
  double getDrawAreaSize() override;

  void drawOutline(cairo_t* cr, double lineWidth) override;
  void drawTickLine(cairo_t* cr, double linePosition, double lineWidth, double lineLength) override;
  void drawTickText(cairo_t*           cr,
                    const std::string& label,
                    double             linePosition,
                    double             labelOffset,
                    double             labelAlign,
                    double             lineLength) override;
};

/**
 * This class implements the drawing code for a vertical ruler.
 */
class VerticalDrawStrategy : public RulerDrawStrategyInterface
{
public:
  /**
   * Creates a new VerticalDrawStrategy.
   * @return A new VerticalDrawStrategy.
   */
  static RulerDrawStrategyInterface::Ptr create();

  double getMajorTickLength(double percentage) override;
  double getDrawAreaSize() override;

  void drawOutline(cairo_t* cr, double lineWidth) override;
  void drawTickLine(cairo_t* cr, double linePosition, double lineWidth, double lineLength) override;
  void drawTickText(cairo_t*           cr,
                    const std::string& label,
                    double             linePosition,
                    double             labelOffset,
                    double             labelAlign,
                    double             lineLength) override;
};