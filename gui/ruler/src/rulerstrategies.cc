#include "rulerstrategies.hh"

#include <cmath>
#include <string>

void RulerDrawStrategyInterface::setAllocatedSize(int newWidth, int newHeight)
{
  this->width  = newWidth;
  this->height = newHeight;
}

RulerDrawStrategyInterface::Ptr HorizontalDrawStrategy::create()
{
  return RulerDrawStrategyInterface::Ptr(new HorizontalDrawStrategy());
}

RulerDrawStrategyInterface::Ptr VerticalDrawStrategy::create()
{
  return RulerDrawStrategyInterface::Ptr(new VerticalDrawStrategy());
}

double HorizontalDrawStrategy::getMajorTickLength(double percentage) { return percentage * height; }

double VerticalDrawStrategy::getMajorTickLength(double percentage) { return percentage * width; }

double HorizontalDrawStrategy::getDrawAreaSize() { return width; }

double VerticalDrawStrategy::getDrawAreaSize() { return height; }

void HorizontalDrawStrategy::drawOutline(cairo_t* cr, double lineWidth)
{
  cairo_set_line_width(cr, lineWidth);
  double drawOffset = lineWidth * LINE_COORD_OFFSET;

  // Draw line along left side of ruler
  cairo_move_to(cr, drawOffset, 0);
  cairo_line_to(cr, drawOffset, height);

  // Draw line along right side of ruler
  cairo_move_to(cr, width - drawOffset, 0);
  cairo_line_to(cr, width - drawOffset, height);
  // Render both lines
  cairo_stroke(cr);

  // Draw thicker border along bottom of ruler
  cairo_set_line_width(cr, 2 * lineWidth);
  drawOffset = 2 * lineWidth * LINE_COORD_OFFSET;
  cairo_move_to(cr, 0, height - drawOffset);
  cairo_line_to(cr, width, height - drawOffset);
  cairo_stroke(cr);
}

void VerticalDrawStrategy::drawOutline(cairo_t* cr, double lineWidth)
{
  cairo_set_line_width(cr, lineWidth);
  double drawOffset = lineWidth * LINE_COORD_OFFSET;

  // Draw line along top side of ruler
  cairo_move_to(cr, 0, drawOffset);
  cairo_line_to(cr, width, drawOffset);

  // Draw line along bottom side of ruler
  cairo_move_to(cr, 0, height - drawOffset);
  cairo_line_to(cr, width, height - drawOffset);
  // Render both lines
  cairo_stroke(cr);

  // Draw thicker border along right of ruler
  cairo_set_line_width(cr, 2 * lineWidth);
  drawOffset = 2 * lineWidth * LINE_COORD_OFFSET;
  cairo_move_to(cr, width - drawOffset, 0);
  cairo_line_to(cr, width - drawOffset, height);
  cairo_stroke(cr);
}

void HorizontalDrawStrategy::drawTickLine(cairo_t* cr, double linePosition, double lineWidth, double lineLength)
{
  cairo_set_line_width(cr, lineWidth);
  const double DRAW_OFFSET = lineWidth * LINE_COORD_OFFSET;

  // Draw vertical line
  cairo_move_to(cr, linePosition + DRAW_OFFSET, height);
  cairo_line_to(cr, linePosition + DRAW_OFFSET, height - round(lineLength));
  cairo_stroke(cr);
}

void VerticalDrawStrategy::drawTickLine(cairo_t* cr, double linePosition, double lineWidth, double lineLength)
{
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
  // Get the extents of the text if it were drawn
  cairo_text_extents_t textExtents;
  cairo_text_extents(cr, label.c_str(), &textExtents);
  // Center the text on the line
  cairo_move_to(cr, width - labelAlign * lineLength - TEXT_ANCHOR * textExtents.y_bearing, linePosition - labelOffset);
  // Draw text rotated 90 degrees anti-clockwise
  cairo_rotate(cr, -M_PI / 2);
  cairo_show_text(cr, label.c_str());
}
