#include <cmath>
#include <iostream>
#include "ruler.hh"

////////////////////////////////////////////////////////////////////////
// Ruler

Ruler::Ptr Ruler::create(Ruler::Orientation orientation, GtkWidget *drawingArea) {
    Ruler::Ptr ruler{new Ruler(orientation)};
    if (drawingArea != nullptr)
    {
        ruler->setDrawingArea(drawingArea);
    }
    return ruler;
}

Ruler::Ruler(Ruler::Orientation orientation)
    : orientation{orientation}
{
}

void Ruler::setDrawingArea(GtkWidget *widget) {
    this->drawingArea = widget;

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    // Register callbacks
    g_signal_connect(static_cast<gpointer>(widget), "draw", G_CALLBACK(drawCallback), this);
    g_signal_connect(static_cast<gpointer>(widget), "size-allocate", G_CALLBACK(sizeAllocateCallback), this);
}

void Ruler::setRange(double lower, double upper) {
    lowerLimit = lower;
    upperLimit = upper;

    if (drawingArea == nullptr) return;

    update();

    // We need to manually trigger the widget to redraw
    gtk_widget_queue_draw(drawingArea);
}

void Ruler::update() {

    if (drawingArea == nullptr) return;

    // We need to calculate the distance between the largest ticks on the ruler
    // We will try each interval sequentially until we find an interval which
    // will produce segments of a large enough width/height when drawn

    // Once we have the distance between the major ticks (the size of the segments)
    // each segment is then divided into 5 parts assuming there's enough space
    // and each of those parts is again divided into 2, again, assuming there's enough space

    // Index in the ruler's VALID_INTERVALS array
    int intervalIndex = 0;
    // Each interval is multiplier by 10 raised to the power n
    int intervalN = 0;

    while (true)
    {
        if (VALID_INTERVALS[intervalIndex] != 1 || intervalN != 0)
        {
            majorInterval = VALID_INTERVALS[intervalIndex] * pow(10, intervalN);

            // We check versus the width or height depending on orientation
            double rulerSize;
            if (orientation == HORIZONTAL)
                rulerSize = width;
            else
                rulerSize = height;
            // Calculate the drawn size for this interval by mapping from the ruler range
            // to the ruler size on the screen
            segmentScreenSize = floor(mapRange(majorInterval, 0.0, upperLimit - lowerLimit, 0.0, rulerSize));

            // If we've found a segment of appropriate size, we can stop
            if (segmentScreenSize >= MIN_SEGMENT_SIZE) break;
        }

        // Try the next interval
        intervalIndex++;
        if (intervalIndex == VALID_INTERVALS.size())
        {
            // We tried all intervals for the current n
            intervalIndex = 0;
            intervalN++;
        }
    }
}

double Ruler::mapRange(double x, double a_lower, double a_upper, double b_lower, double b_upper) {
    double a_width = a_upper - a_lower;
    double b_width = b_upper - b_lower;
    double scale = b_width / a_width;

    return b_lower + scale * (x - a_lower);
}

gboolean Ruler::drawCallback(GtkWidget *widget, cairo_t *cr, gpointer data) {

    auto *ruler = static_cast<Ruler*>(data);

    if (ruler->drawingArea == nullptr) return FALSE;

    double width = ruler->width;
    double height = ruler->height;

    // Draw background
    GtkStyleContext *context = gtk_widget_get_style_context (widget);
    gtk_render_background(context, cr, 0, 0, ruler->width, ruler->height);

    // Draw outline along left and right sides and along the bottom
    gdk_cairo_set_source_rgba(cr, &(ruler->lineColor));

    cairo_set_line_width(cr, Ruler::LINE_WIDTH);
    if (ruler->orientation == HORIZONTAL)
    {
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, 0, height);

        cairo_move_to(cr, width, 0);
        cairo_line_to(cr, width, height);
        cairo_stroke(cr);

        cairo_set_line_width(cr, 2 * Ruler::LINE_WIDTH);
        cairo_move_to(cr, 0, height);
        cairo_line_to(cr, width, height);
        cairo_stroke(cr);
    }
    else if (ruler->orientation == VERTICAL)
    {
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, width, 0);

        cairo_move_to(cr, 0, height);
        cairo_line_to(cr, width, height);
        cairo_stroke(cr);

        cairo_set_line_width(cr, 2 * Ruler::LINE_WIDTH);
        cairo_move_to(cr, width, 0);
        cairo_line_to(cr, width, height);
        cairo_stroke(cr);
    }
    cairo_set_line_width(cr, Ruler::LINE_WIDTH);

    // Calculate the line length for the major ticks given the size of the ruler
    double lineLength;
    if (ruler->orientation == HORIZONTAL)
        lineLength = Ruler::MAJOR_TICK_LENGTH * height;
    else
        lineLength = Ruler::MAJOR_TICK_LENGTH * width;

    // Draw positive side of the ruler from lower to upper
    ruler->drawTicks(cr, 0, ruler->upperLimit, true, lineLength);

    // Draw negative side of the ruler from upper to lower
    ruler->drawTicks(cr, ruler->lowerLimit, 0, false, lineLength);

    return FALSE;
}

void Ruler::drawTicks(cairo_t *cr, double lower, double upper, bool lowerToUpper, double lineLength) {
    double t; // Position in ruler range
    if (lowerToUpper)
        t = lower;
    else
        t = upper;

    double origin;
    double size;
    if (orientation == HORIZONTAL)
    {
        origin = 0;
        size = width;
    }
    else if (orientation == VERTICAL)
    {
        origin = 0;
        size = height;
    }

    // Move t across range
    while ((lowerToUpper && t < upper) || (!lowerToUpper && lower < t))
    {
        // Map t from the ruler range to a drawing area position
        double s = mapRange(t, lowerLimit, upperLimit, origin, origin + size);
        // Draw tick for this position
        drawSingleTick(cr, s, lineLength, true, std::to_string(static_cast<int>(floor(t))));

        if (lowerToUpper)
        {
            drawSubTicks(cr, s, s + segmentScreenSize, 0, 0.5 * lineLength, lowerToUpper);
            t += majorInterval;
        }
        else
        {
            drawSubTicks(cr, s - segmentScreenSize, s, 0, 0.5 * lineLength, lowerToUpper);
            t -= majorInterval;
        }
    }
}

void Ruler::sizeAllocateCallback(GtkWidget *widget, GdkRectangle *allocation, gpointer data) {
    auto *ruler = static_cast<Ruler*>(data);

    if (ruler->drawingArea == nullptr) return;

    ruler->width = gtk_widget_get_allocated_width(widget);
    ruler->height = gtk_widget_get_allocated_height(widget);

    ruler->update();
}

void Ruler::drawSingleTick(cairo_t *cr, double lineOrigin, double lineLength, bool drawLabel, const std::string& label) {
    if (orientation == HORIZONTAL)
    {
        // [BUG] Setting the line width to LINE_WIDTH here still draws the lines rather thick.
        // Probably some sub-pixel rounding errors or transform matrix shenanigens going on
        // so for now I just half LINE_WIDTH to achieve the right end-result.
        // (Yes, that's ugly and might break somewhere later, but for now this works.)
        cairo_set_line_width(cr, 0.5 * LINE_WIDTH);
        cairo_move_to(cr, lineOrigin, height);
        cairo_line_to(cr, lineOrigin, height - lineLength);
        cairo_stroke(cr);
    }
    else if (orientation == VERTICAL)
    {
        cairo_set_line_width(cr, 0.5 * LINE_WIDTH);
        cairo_move_to(cr, width, lineOrigin);
        cairo_line_to(cr, width - lineLength, lineOrigin);
        cairo_stroke(cr);
    }

    cairo_save(cr);
    if (drawLabel)
    {
        // Get the extents of the text if it were drawn
        // [NOTE] Text sizing and stuff is probably still inconsistent between Windows and Linux.
        // Selecting an explicit font type might help.
        cairo_set_font_size(cr, FONT_SIZE);
        cairo_text_extents_t textExtents;
        cairo_text_extents(cr, label.c_str(), &textExtents);
        // Draw the label if there's enough room
        if (textExtents.x_advance < segmentScreenSize)
        {
            if (orientation == HORIZONTAL)
            {
                // Center the text on the line
                cairo_move_to(cr, lineOrigin + LABEL_OFFSET, height - 0.5 * lineLength - 0.5 * textExtents.y_bearing);
                cairo_show_text(cr, label.c_str());
            }
            else if (orientation == VERTICAL)
            {
                cairo_move_to(cr, width - 0.5 * lineLength - 0.5 * textExtents.y_bearing, lineOrigin - LABEL_OFFSET);
                cairo_rotate(cr, -M_PI / 2);
                cairo_show_text(cr, label.c_str());
            }
        }
    }
    cairo_restore(cr);
}

void Ruler::drawSubTicks(cairo_t *cr, double lower, double upper, int depth, double lineLength, bool lowerToUpper) {

    // We don't need to divide the segment any further so return
    if (depth >= SUBTICK_SEGMENTS.size()) return;

    int numSegments = SUBTICK_SEGMENTS[depth];
    double interval = abs(upper - lower) / numSegments;

    if (interval < MIN_SPACE_SUBTICKS) return;

    // We draw from lower->upper / upper->lower, but in the process, we might be exceeding
    // the ruler area, so we also check that we're still inside the drawing area
    double limit;
    if (orientation == HORIZONTAL)
    {
        if (lowerToUpper)
            limit = width;
        else
            limit = 0;
    }
    else if (orientation == VERTICAL)
    {
        if (lowerToUpper)
            limit = height;
        else
            limit = 0;
    }

    double s; // Position to draw tick at
    if (lowerToUpper)
        s = lower;
    else
        s = upper;

    while ((lowerToUpper && s < upper && s < limit) || (!lowerToUpper && lower < s && limit < s))
    {
        drawSingleTick(cr, s, lineLength, false, "");
        if (lowerToUpper)
        {
            // Draw ticks at level below
            drawSubTicks(cr, s, s + interval, depth + 1, 0.5 * lineLength, lowerToUpper);
            s += interval;
        }
        else
        {
            drawSubTicks(cr, s - interval, s, depth + 1, 0.5 * lineLength, lowerToUpper);
            s -= interval;
        }
    }
}
