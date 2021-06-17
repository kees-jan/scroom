#include "rulerstrategies.hh"

#include <cmath>
#include <string>

void RulerDrawStrategy::setAllocatedSize(int newWidth, int newHeight)
{
  this->width  = newWidth;
  this->height = newHeight;
}

int RulerDrawStrategy::getWidth() const
{
  return width;
}

int RulerDrawStrategy::getHeight() const
{
  return height;
}

RulerDrawStrategy::Ptr HorizontalDrawStrategy::create()
{
  return RulerDrawStrategy::Ptr(new HorizontalDrawStrategy());
}

RulerDrawStrategy::Ptr VerticalDrawStrategy::create()
{
  return RulerDrawStrategy::Ptr(new VerticalDrawStrategy());
}

double HorizontalDrawStrategy::getMajorTickLength(double percentage) { return percentage * getHeight(); }

double VerticalDrawStrategy::getMajorTickLength(double percentage) { return percentage * getWidth(); }

double HorizontalDrawStrategy::getDrawAreaSize() { return getWidth(); }

double VerticalDrawStrategy::getDrawAreaSize() { return getHeight(); }

void HorizontalDrawStrategy::drawOutline(cairo_t* cr, double lineWidth)
{
  int width = getWidth();
  int height = getHeight();

  cairo_set_line_width(cr, lineWidth);
  const double DRAW_OFFSET = lineWidth * LINE_COORD_OFFSET;

  // Draw line along left side of ruler
  cairo_move_to(cr, DRAW_OFFSET, 0);
  cairo_line_to(cr, DRAW_OFFSET, height);

  // Draw line along right side of ruler
  cairo_move_to(cr, width - DRAW_OFFSET, 0);
  cairo_line_to(cr, width - DRAW_OFFSET, height);

  // Draw thicker border along bottom of ruler
  cairo_move_to(cr, 0, height - DRAW_OFFSET);
  cairo_line_to(cr, width, height - DRAW_OFFSET);

  // Render all lines
  cairo_stroke(cr);
}

void VerticalDrawStrategy::drawOutline(cairo_t* cr, double lineWidth)
{
  int width = getWidth();
  int height = getHeight();

  cairo_set_line_width(cr, lineWidth);
  const double DRAW_OFFSET = lineWidth * LINE_COORD_OFFSET;

  // Draw line along top side of ruler
  cairo_move_to(cr, 0, DRAW_OFFSET);
  cairo_line_to(cr, width, DRAW_OFFSET);

  // Draw line along bottom side of ruler
  cairo_move_to(cr, 0, height - DRAW_OFFSET);
  cairo_line_to(cr, width, height - DRAW_OFFSET);

  // Draw thicker border along right of ruler
  cairo_move_to(cr, width - DRAW_OFFSET, 0);
  cairo_line_to(cr, width - DRAW_OFFSET, height);

  // Render all lines
  cairo_stroke(cr);
}

void HorizontalDrawStrategy::drawTickLine(cairo_t* cr, double linePosition, double lineWidth, double lineLength)
{
  int height = getHeight();

  cairo_set_line_width(cr, lineWidth);
  const double DRAW_OFFSET = lineWidth * LINE_COORD_OFFSET;

  // Draw vertical line
  cairo_move_to(cr, linePosition + DRAW_OFFSET, height);
  cairo_line_to(cr, linePosition + DRAW_OFFSET, height - round(lineLength));
  cairo_stroke(cr);
}

void VerticalDrawStrategy::drawTickLine(cairo_t* cr, double linePosition, double lineWidth, double lineLength)
{
  int width = getWidth();

  cairo_set_line_width(cr, lineWidth);
  const double DRAW_OFFSET = lineWidth * LINE_COORD_OFFSET;

  // Draw horizontal line
  cairo_move_to(cr, width, linePosition + DRAW_OFFSET);
  cairo_line_to(cr, width - round(lineLength), linePosition + DRAW_OFFSET);
  cairo_stroke(cr);
}

void HorizontalDrawStrategy::drawTickText(cairo_t*           cr,
                                          const std::string& label,
                                          double             linePosition,
                                          double             labelOffset,
                                          double             labelAlign,
                                          double             lineLength)
{
  int height = getHeight();

  // Get the extents of the text if it were drawn
  cairo_text_extents_t textExtents;
  cairo_text_extents(cr, label.c_str(), &textExtents);
  // Center the text on the line
  cairo_move_to(cr, linePosition + labelOffset, height - labelAlign * lineLength - TEXT_ANCHOR * textExtents.y_bearing);
  cairo_show_text(cr, label.c_str());
}

void VerticalDrawStrategy::drawTickText(cairo_t*           cr,
                                        const std::string& label,
                                        double             linePosition,
                                        double             labelOffset,
                                        double             labelAlign,
                                        double             lineLength)
{
  int width = getWidth();

  // Get the extents of the text if it were drawn
  cairo_text_extents_t textExtents;
  cairo_text_extents(cr, label.c_str(), &textExtents);
  // Center the text on the line
  cairo_move_to(cr, width - labelAlign * lineLength - TEXT_ANCHOR * textExtents.y_bearing, linePosition - labelOffset);
  // Draw text rotated 90 degrees anti-clockwise
  cairo_rotate(cr, -M_PI / 2);
  cairo_show_text(cr, label.c_str());
}
