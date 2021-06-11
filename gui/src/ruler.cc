#include <cmath>
#include <iostream>
#include "ruler.hh"

////////////////////////////////////////////////////////////////////////
// Ruler

Ruler::Ptr Ruler::create(Ruler::Orientation orientation, GtkWidget *drawingArea)
{
    Ruler::Ptr ruler{new Ruler(orientation, drawingArea)};
    return ruler;
}

Ruler::Ruler(Ruler::Orientation orientation, GtkWidget* drawingAreaWidget)
        : orientation{orientation}
        , drawingArea{drawingAreaWidget}
        , width{gtk_widget_get_allocated_width(drawingAreaWidget)}
        , height{gtk_widget_get_allocated_height(drawingAreaWidget)}
{
    // [TODO] Scroom contains a require() macro.
    //  We'll need to add this when we move this code to Scroom.
    //require(drawingArea != nullptr);

    // Connect signal handlers
    g_signal_connect(drawingAreaWidget, "draw", G_CALLBACK(drawCallback), this); // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    g_signal_connect(drawingAreaWidget, "size-allocate", G_CALLBACK(sizeAllocateCallback), this); // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
    // Calculate tick intervals and spacing
    calculateTickIntervals();
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

    calculateTickIntervals();

    // We need to manually trigger the widget to redraw
    gtk_widget_queue_draw(drawingArea);
}

double Ruler::getLowerLimit() const
{
    return lowerLimit;
}

double Ruler::getUpperLimit() const
{
    return upperLimit;
}

void Ruler::sizeAllocateCallback(GtkWidget *widget, GdkRectangle * /*allocation*/, gpointer data)
{
    auto *ruler = static_cast<Ruler *>(data);

    ruler->width = gtk_widget_get_allocated_width(widget);
    ruler->height = gtk_widget_get_allocated_height(widget);

    ruler->calculateTickIntervals();
}

void Ruler::calculateTickIntervals()
{
    const double ALLOCATED_SIZE = (orientation == HORIZONTAL) ? width : height;
    // Calculate the interval between major ruler ticks
    majorInterval = RulerCalculations::calculateInterval(lowerLimit, upperLimit, ALLOCATED_SIZE);
    // Calculate the spacing in pixels between major ruler ticks
    majorTickSpacing = RulerCalculations::intervalPixelSpacing(majorInterval, lowerLimit, upperLimit, ALLOCATED_SIZE);
}

gboolean Ruler::drawCallback(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    auto *ruler = static_cast<Ruler *>(data);
    ruler->draw(widget, cr);

    return FALSE;
}

void Ruler::draw(GtkWidget *widget, cairo_t *cr)
{
    // Draw background using widget's style context
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_render_background(context, cr, 0, 0, width, height);

    // Draw outline along left and right sides and along the bottom
    gdk_cairo_set_source_rgba(cr, &lineColor);
    cairo_set_line_width(cr, LINE_WIDTH);
    // Cairo integer coordinates map to points halfway between pixels.
    // We need to offset the coordinates by 0.5 times the line width
    // to get clear lines
    double drawOffset = LINE_WIDTH * LINE_COORD_OFFSET;
    if (orientation == HORIZONTAL)
    {
        // Draw line along left side of ruler
        cairo_move_to(cr, drawOffset, 0);
        cairo_line_to(cr, drawOffset, height);

        // Draw line along right side of ruler
        cairo_move_to(cr, width - drawOffset, 0);
        cairo_line_to(cr, width - drawOffset, height);
        // Render both lines
        cairo_stroke(cr);

        // Draw thicker border along bottom of ruler
        cairo_set_line_width(cr, 2 * LINE_WIDTH);
        drawOffset = 2 * LINE_WIDTH * LINE_COORD_OFFSET;
        cairo_move_to(cr, 0, height - drawOffset);
        cairo_line_to(cr, width, height - drawOffset);
        cairo_stroke(cr);
    }
    else
    {
        // Draw line along top side of ruler
        cairo_move_to(cr, 0, drawOffset);
        cairo_line_to(cr, width, drawOffset);

        // Draw line along bottom side of ruler
        cairo_move_to(cr, 0, height - drawOffset);
        cairo_line_to(cr, width, height - drawOffset);
        // Render both lines
        cairo_stroke(cr);

        // Draw thicker border along right of ruler
        cairo_set_line_width(cr, 2 * LINE_WIDTH);
        drawOffset = 2 * LINE_WIDTH * LINE_COORD_OFFSET;
        cairo_move_to(cr, width - drawOffset, 0);
        cairo_line_to(cr, width - drawOffset, height);
        cairo_stroke(cr);
    }
    cairo_set_line_width(cr, Ruler::LINE_WIDTH);

    // The majorInterval is invalid, don't attempt to draw anything else
    if (majorInterval <= 0) { return; }

    // Calculate the line length for the major ticks given the size of the ruler
    double lineLength = (orientation == HORIZONTAL) ? MAJOR_TICK_LENGTH * height : MAJOR_TICK_LENGTH * width;

    // Draw the range [0, upperLimit]
    drawTicks(cr, 0.0, upperLimit, true, lineLength);

    // Draw the range [lowerLimit, 0]
    drawTicks(cr, lowerLimit, 0.0, false, lineLength);
}

void Ruler::drawTicks(cairo_t *cr, double lower, double upper, bool lowerToUpper, double lineLength)
{
    // Position in ruler range
    double pos = lowerToUpper ? lower : upper;

    const double DRAW_AREA_ORIGIN = 0;
    // We need to scale to either [0, width] or [0, height] depending
    // on the orientation of the ruler
    const double DRAW_AREA_SIZE = (orientation == HORIZONTAL) ? width : height;

    // Move pos across range
    while ((lowerToUpper && pos < upper) || (!lowerToUpper && lower < pos))
    {
        // Map pos from the ruler range to a drawing area position
        double s = RulerCalculations::scaleToRange(pos, lowerLimit, upperLimit, DRAW_AREA_ORIGIN, DRAW_AREA_ORIGIN + DRAW_AREA_SIZE);
        // Draw tick for this position
        drawSingleTick(cr, s, lineLength, true, std::to_string(static_cast<int>(floor(pos))));

        if (lowerToUpper)
        {
            drawSubTicks(cr, s, s + majorTickSpacing, 0, LINE_MULTIPLIER * lineLength, lowerToUpper);
            pos += majorInterval;
        }
        else
        {
            drawSubTicks(cr, s - majorTickSpacing, s, 0, LINE_MULTIPLIER * lineLength, lowerToUpper);
            pos -= majorInterval;
        }
    }
}

void Ruler::drawSingleTick(cairo_t *cr, double linePosition, double lineLength, bool drawLabel, const std::string &label)
{
    // Draw line
    cairo_set_line_width(cr, LINE_WIDTH);
    // Offset the line to get a clear line
    const double DRAW_OFFSET = LINE_WIDTH * LINE_COORD_OFFSET;
    if (orientation == HORIZONTAL)
    {
        // Draw vertical line
        cairo_move_to(cr, linePosition + DRAW_OFFSET, height);
        cairo_line_to(cr, linePosition + DRAW_OFFSET, height - round(lineLength));
    }
    else
    {
        // Draw horizontal line
        cairo_move_to(cr, width, linePosition + DRAW_OFFSET);
        cairo_line_to(cr, width - round(lineLength), linePosition + DRAW_OFFSET);
    }
    cairo_stroke(cr);

    // We'll be modifying the transformation matrix so
    // we save the current one to restore later
    cairo_save(cr);
    if (drawLabel) // Draw the tick label
    {
        // Set text font and size
        cairo_select_font_face(cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, FONT_SIZE);
        // Get the extents of the text if it were drawn
        cairo_text_extents_t textExtents;
        cairo_text_extents(cr, label.c_str(), &textExtents);
        // Draw the label if there's enough room
        if (textExtents.x_advance < majorTickSpacing)
        {
            if (orientation == HORIZONTAL)
            {
                // Center the text on the line
                cairo_move_to(cr, linePosition + LABEL_OFFSET, height - LABEL_ALIGN * lineLength - LINE_MULTIPLIER * textExtents.y_bearing);
                cairo_show_text(cr, label.c_str());
            }
            else
            {
                cairo_move_to(cr, width - LABEL_ALIGN * lineLength - LINE_MULTIPLIER * textExtents.y_bearing, linePosition - LABEL_OFFSET);
                cairo_rotate(cr, -M_PI / 2);
                cairo_show_text(cr, label.c_str());
            }
        }
    }
    cairo_restore(cr);
}

void Ruler::drawSubTicks(cairo_t *cr, double lower, double upper, int depth, double lineLength, bool lowerToUpper)
{
    // We don't need to divide the segment any further so return
    if (depth >= SUBTICK_SEGMENTS.size()) { return; }

    int numSegments = SUBTICK_SEGMENTS.at(depth);
    double interval = abs(upper - lower) / numSegments;

    if (interval < MIN_SPACE_SUBTICKS) { return; }

    // We draw from lower->upper / upper->lower, but in the process, we might be exceeding
    // the ruler area, so we also check that we're still inside the drawing area
    const double DRAW_AREA_SIZE = (orientation == HORIZONTAL) ? width : height;
    const double limit = lowerToUpper ? DRAW_AREA_SIZE : 0;

    // Position along the ruler to draw tick at
    double tick = 0;
    double pos = lowerToUpper ? lower : upper;

    // Draw at most (numSegments - 1) ticks, while not exceeding the limit
    while (tick < numSegments && ((lowerToUpper && pos < limit) || (!lowerToUpper && limit < pos)))
    {
        // We don't want to draw the tick for tick == 0, because
        // we would end up drawing over other ticks
        if (tick != 0)
        {
            drawSingleTick(cr, pos, lineLength, false, "");
        }
        tick++;
        if (lowerToUpper)
        {
            // Draw ticks at level below
            drawSubTicks(cr, pos, pos + interval, depth + 1, LINE_MULTIPLIER * lineLength, lowerToUpper);
            pos += interval;
        }
        else
        {
            drawSubTicks(cr, pos - interval, pos, depth + 1, LINE_MULTIPLIER * lineLength, lowerToUpper);
            pos -= interval;
        }
    }
}

double RulerCalculations::scaleToRange(double x, double src_lower, double src_upper, double dest_lower, double dest_upper)
{
    double src_size = src_upper - src_lower;
    double dest_size = dest_upper - dest_lower;
    double scale = dest_size / src_size;

    return dest_lower + round(scale * (x - src_lower));
}

int RulerCalculations::calculateInterval(double lower, double upper, double allocatedSize)
{
    // We need to calculate the distance between the largest ticks on the ruler
    // We will try each interval x * 10^n for x in VALID_INTERVALS and integer n >= 0
    // from smallest to largest until we find an interval which will produce a
    // spacing of a large enough width/height when drawn

    if (upper <= lower) { return -1; }

    // Index in the ruler's VALID_INTERVALS array
    int intervalIndex = 0;
    // Each interval is multiplied by 10 raised to a power n
    const int INTERVAL_BASE = 10;
    int intervalN = 0;

    // The interval to be returned
    int interval = 1;

    while (true)
    {
        interval = floor(VALID_INTERVALS.at(intervalIndex) * pow(INTERVAL_BASE, intervalN));

        // Calculate the drawn size for this interval by mapping from the ruler range
        // to the ruler size on the screen
        double spacing = intervalPixelSpacing(interval, lower, upper, allocatedSize);
        // If we've found a segment of appropriate size, we can stop
        if (spacing >= MIN_SPACE_MAJORTICKS) { break; }

        // Otherwise, try the next interval
        intervalIndex++;
        if (intervalIndex == VALID_INTERVALS.size())
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
    const double RANGE_SIZE = upper - lower;
    return static_cast<int>(round((allocatedSize / RANGE_SIZE) * interval));
}
