#include "ruler.hh"

#include <cmath>
#include <iostream>
#include <utility>

#include <scroom/assertions.hh>

////////////////////////////////////////////////////////////////////////
// Ruler

Ruler::Ptr Ruler::create(Ruler::Orientation orientation, GtkWidget* drawingArea)
{
  Ruler::Ptr ruler;
  // We pass a different drawing strategy to the ruler depending on orientation
  if(orientation == HORIZONTAL)
  {
    ruler = Ruler::Ptr(new Ruler(orientation, HorizontalDrawStrategy::create(), drawingArea));
  }
  else
  {
    ruler = Ruler::Ptr(new Ruler(orientation, VerticalDrawStrategy::create(), drawingArea));
  }
  return ruler;
}

Ruler::Ruler(Ruler::Orientation rulerOrientation, RulerDrawStrategyInterface::Ptr strategy, GtkWidget* drawingAreaWidget)
  : drawingArea{drawingAreaWidget}
  , orientation{rulerOrientation}
  , width{gtk_widget_get_allocated_width(drawingAreaWidget)}
  , height{gtk_widget_get_allocated_height(drawingAreaWidget)}
  , drawStrategy{std::move(strategy)}
{
  require(drawingArea != nullptr);

  // Set size for strategy
  this->drawStrategy->setAllocatedSize(width, height);

  // Connect signal handlers
  g_signal_connect(drawingAreaWidget, "draw", G_CALLBACK(drawCallback), this);
  g_signal_connect(drawingAreaWidget, "size-allocate", G_CALLBACK(sizeAllocateCallback), this);
  // Calculate tick intervals and spacing
  updateMajorTickInterval();
}

Ruler::~Ruler()
{
  // Disconnect all signal handlers for this object from the drawing area
  g_signal_handlers_disconnect_by_data(drawingArea, this);
}

void Ruler::setRange(double lower, double upper)
{
  lowerLimit = lower;
  upperLimit = upper;

  updateMajorTickInterval();

  // We need to manually trigger the widget to redraw
  gtk_widget_queue_draw(drawingArea);
}

double Ruler::getLowerLimit() const { return lowerLimit; }

double Ruler::getUpperLimit() const { return upperLimit; }

int Ruler::getWidth() const { return width; }

int Ruler::getHeight() const { return height; }

void Ruler::updateAllocatedSize(int newWidth, int newHeight)
{
  this->width  = newWidth;
  this->height = newHeight;
  drawStrategy->setAllocatedSize(newWidth, newHeight);

  updateMajorTickInterval();
}

void Ruler::sizeAllocateCallback(GtkWidget* widget, GdkRectangle* /*allocation*/, gpointer data)
{
  auto* ruler = static_cast<Ruler*>(data);

  int width  = gtk_widget_get_allocated_width(widget);
  int height = gtk_widget_get_allocated_height(widget);

  ruler->updateAllocatedSize(width, height);
}

void Ruler::updateMajorTickInterval()
{
  const double ALLOCATED_SIZE = drawStrategy->getDrawAreaSize();
  // Calculate the interval between major ruler ticks
  majorInterval = RulerCalculations::calculateInterval(lowerLimit, upperLimit, ALLOCATED_SIZE);
  // Calculate the spacing in pixels between major ruler ticks
  majorTickSpacing = RulerCalculations::intervalPixelSpacing(majorInterval, lowerLimit, upperLimit, ALLOCATED_SIZE);
}

gboolean Ruler::drawCallback(GtkWidget* widget, cairo_t* cr, gpointer data)
{
  auto* ruler = static_cast<Ruler*>(data);
  ruler->draw(widget, cr);

  return FALSE;
}

void Ruler::draw(GtkWidget* widget, cairo_t* cr)
{
  // Initialize cairo
  cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);

  // Draw background using widget's style context
  GtkStyleContext* context = gtk_widget_get_style_context(widget);
  gtk_render_background(context, cr, 0, 0, width, height);

  // Draw outline
  gdk_cairo_set_source_rgba(cr, &lineColor);
  drawStrategy->drawOutline(cr, LINE_WIDTH);
  cairo_set_line_width(cr, Ruler::LINE_WIDTH);

  // The majorInterval is invalid, don't attempt to draw anything else
  if(majorInterval <= 0)
  {
    return;
  }

  // Calculate the line length for the major ticks given the size of the ruler
  double lineLength = drawStrategy->getMajorTickLength(MAJOR_TICK_LENGTH);

  int firstTick = RulerCalculations::firstTick(lowerLimit, majorInterval);

  // Draw the range [0, upperLimit]
  drawTicks(cr, firstTick, upperLimit, lineLength);
}

void Ruler::drawTicks(cairo_t* cr, double lower, double upper, double lineLength)
{
  // Position in ruler range
  double pos = lower;

  const double DRAW_AREA_ORIGIN = 0;
  // We need to scale to either [0, width] or [0, height] depending
  // on the orientation of the ruler
  const double DRAW_AREA_SIZE = drawStrategy->getDrawAreaSize();

  // Move pos across range
  while(pos < upper)
  {
    // Map pos from the ruler range to a drawing area position
    double s = RulerCalculations::scaleToRange(pos, lowerLimit, upperLimit, DRAW_AREA_ORIGIN, DRAW_AREA_ORIGIN + DRAW_AREA_SIZE);
    // Draw tick for this position
    drawSingleTick(cr, s, lineLength, true, std::to_string(static_cast<int>(floor(pos))));

    drawSubTicks(cr, s, s + majorTickSpacing, 0, LINE_MULTIPLIER * lineLength);
    pos += majorInterval;
  }
}

void Ruler::drawSingleTick(cairo_t* cr, double linePosition, double lineLength, bool drawLabel, const std::string& label)
{
  const double DRAW_AREA_SIZE = drawStrategy->getDrawAreaSize();
  // Draw the line if is within the drawing area
  if(0 < linePosition && linePosition < DRAW_AREA_SIZE)
  {
    // Draw line
    drawStrategy->drawTickLine(cr, linePosition, LINE_WIDTH, lineLength);
  }

  // We'll be modifying the transformation matrix so
  // we save the current one to restore later
  cairo_save(cr);
  if(drawLabel) // Draw the tick label
  {
    // Set text font and size
    cairo_select_font_face(cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, FONT_SIZE);
    // drawTickText(cairo_t *cr, std::string label, double linePosition, double labelOffset, double labelAlign, double lineLength,
    // int width, int height) = 0;
    drawStrategy->drawTickText(cr, label, linePosition, LABEL_OFFSET, LABEL_ALIGN, lineLength);
  }
  cairo_restore(cr);
}

void Ruler::drawSubTicks(cairo_t* cr, double lower, double upper, int depth, double lineLength)
{
  // We don't need to divide the segment any further so return
  if(static_cast<unsigned int>(depth) >= SUBTICK_SEGMENTS.size())
  {
    return;
  }

  int    numSegments = SUBTICK_SEGMENTS.at(depth);
  double interval    = abs(upper - lower) / numSegments;

  if(interval < MIN_SPACE_SUBTICKS)
  {
    return;
  }

  // We draw from lower->upper / upper->lower, but in the process, we might be exceeding
  // the ruler area, so we also check that we're still inside the drawing area
  const double DRAW_AREA_SIZE = drawStrategy->getDrawAreaSize();
  const double limit          = DRAW_AREA_SIZE;

  // Position along the ruler to draw tick at
  double tick = 0;
  double pos  = lower;

  // Draw at most (numSegments - 1) ticks, while not exceeding the limit
  while(tick < numSegments && pos < limit)
  {
    // We don't want to draw the tick for tick == 0, because
    // we would end up drawing over other ticks
    if(tick != 0)
    {
      drawSingleTick(cr, pos, lineLength, false, "");
    }
    tick++;
    // Draw ticks at level below
    drawSubTicks(cr, pos, pos + interval, depth + 1, LINE_MULTIPLIER * lineLength);

    pos += interval;
  }
}

double RulerCalculations::scaleToRange(double x, double src_lower, double src_upper, double dest_lower, double dest_upper)
{
  double src_size  = src_upper - src_lower;
  double dest_size = dest_upper - dest_lower;
  double scale     = dest_size / src_size;

  return dest_lower + round(scale * (x - src_lower));
}

int RulerCalculations::calculateInterval(double lower, double upper, double allocatedSize)
{
  // We need to calculate the distance between the largest ticks on the ruler
  // We will try each interval x * 10^n for x in VALID_INTERVALS and integer n >= 0
  // from smallest to largest until we find an interval which will produce a
  // spacing of a large enough width/height when drawn

  if(upper <= lower)
  {
    return -1;
  }

  // Index in the ruler's VALID_INTERVALS array
  int intervalIndex = 0;
  // Each interval is multiplied by 10 raised to a power n
  const int INTERVAL_BASE = 10;
  int       intervalN     = 0;

  // The interval to be returned
  int interval = 1;

  while(true)
  {
    interval = floor(VALID_INTERVALS.at(intervalIndex) * pow(INTERVAL_BASE, intervalN));

    // Calculate the drawn size for this interval by mapping from the ruler range
    // to the ruler size on the screen
    double spacing = intervalPixelSpacing(interval, lower, upper, allocatedSize);
    // If we've found a segment of appropriate size, we can stop
    if(spacing >= MIN_SPACE_MAJORTICKS)
    {
      break;
    }

    // Otherwise, try the next interval
    intervalIndex++;
    if(static_cast<unsigned int>(intervalIndex) == VALID_INTERVALS.size())
    {
      // We tried all intervals for the current n, increment n
      intervalIndex = 0;
      intervalN++;
    }
  }

  return interval;
}

int RulerCalculations::intervalPixelSpacing(double interval, double lower, double upper, double allocatedSize)
{
  if(upper <= lower)
  {
    return -1;
  }

  const double RANGE_SIZE = upper - lower;
  return static_cast<int>(round((allocatedSize / RANGE_SIZE) * interval));
}

int RulerCalculations::firstTick(double lower, int interval) { return static_cast<int>(floor(lower / interval)) * interval; }
