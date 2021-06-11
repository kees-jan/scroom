#pragma once

#include <gtk/gtk.h>
#include <boost/shared_ptr.hpp>

/**
 * This class draws a ruler to a GtkDrawingArea.
 * It is intended as a replacement for the old GTK2 ruler widget and is written
 * to mimic that widget's behavior as close as possible.
 */
class Ruler
{

public:
    using Ptr = boost::shared_ptr<Ruler>;

    enum Orientation
    {
        HORIZONTAL, VERTICAL
    };

    /**
     * Creates a ruler.
     * @param orientation The orientation of the ruler.
     * @param drawArea The GtkDrawingArea to draw the ruler to.
     * @return The newly created ruler.
     */
    static Ptr create(Orientation orientation, GtkWidget *drawArea);

    ~Ruler();
    Ruler(const Ruler&) = delete;
    Ruler(Ruler&&)      = delete;
    Ruler operator=(const Ruler&) = delete;
    Ruler operator=(Ruler&&) = delete;

    /**
     * Sets the range for the ruler to display.
     * @param lower Lower limit of the ruler range. Must be strictly less than \p upper.
     * @param upper Upper limit of the ruler range. Must be strictly greater than \p lower.
     */
    void setRange(double lower, double upper);

    /**
     * Returns the current lower limit of the ruler's range.
     * @return The current lower limit of the ruler's range.
     */
    [[nodiscard]] double getLowerLimit() const;

    /**
     * Returns the current upper limit of the ruler's range.
     * @return The current upper limit of the ruler's range.
     */
    [[nodiscard]] double getUpperLimit() const;

private:

    GtkWidget *drawingArea{};

    static constexpr double DEFAULT_LOWER{0};
    static constexpr double DEFAULT_UPPER{10};

    /**
     * Each space between major ticks is split into 5 smaller segments and
     * those segments are split into 2. (Assuming there's enough space.)
     */
    constexpr static std::array<int, 2> SUBTICK_SEGMENTS{
            5, 2
    };

    Orientation orientation;

    // The range to be displayed.
    double lowerLimit{DEFAULT_LOWER};
    double upperLimit{DEFAULT_UPPER};

    // The allocated width and height for the drawing area widget.
    int width{};
    int height{};

    /** The chosen interval between major ticks. */
    double majorInterval{1};

    /** The space between major ticks when drawn. */
    int majorTickSpacing{};

    // ==== DRAWING PROPERTIES ====

    /**
     * Cairo integer coordinates map to points halfway between pixels.
     * Therefore, if we offset coordinates by 0.5 in the appropriate
     * direction, we can draw clear lines.
     */
    static constexpr double LINE_COORD_OFFSET{0.5};

    /** The minimum space between sub-ticks. */
    static constexpr int MIN_SPACE_SUBTICKS{5};

    static constexpr double FONT_SIZE{11};

    /** Offset of tick label from the tick line in pixels. */
    static constexpr double LABEL_OFFSET{4};

    /** Alignment of tick label along the tick line as a fraction of line height. */
    static constexpr double LABEL_ALIGN{0.7};

    /** The length of a tick one "level" down, as a fraction of the line length of the ticks one level up. */
    static constexpr double LINE_MULTIPLIER{0.6};

    GdkRGBA lineColor{0, 0, 0, 1};

    static constexpr double LINE_WIDTH{1};

    /** Length of the major tick lines as a fraction of the width/height. */
    static constexpr double MAJOR_TICK_LENGTH{0.8};

    /**
     * Creates a Ruler.
     * @param orientation The orientation of the ruler.
     * @param drawingArea The GtkDrawingArea to draw the ruler to.
     */
    Ruler(Orientation orientation, GtkWidget* drawingArea);

    /**
     * A callback to be connected to a GtkDrawingArea's "draw" signal.
     * Draws the ruler to the drawing area.
     * @param widget The widget that received the signal.
     * @param cr Cairo context to draw to.
     * @param data Pointer to a ruler instance.
     * @returns TRUE to stop other handlers from being invoked for the event. FALSE to propagate the event further.
     */
    static gboolean drawCallback(GtkWidget *widget, cairo_t *cr, gpointer data);

    /**
     * Draws the ruler to the given Cairo context.
     * @param widget The widget that received the draw signal.
     * @param cr Cairo context to draw to.
     */
    void draw(GtkWidget *widget, cairo_t *cr);

    /**
     * A callback to be connected to a GtkDrawingArea's "size-allocate" signal.
     * Updates the internal state of the ruler when the size of the ruler changes.
     * @param widget The widget that received the signal.
     * @param allocation The region which has been allocated to the widget.
     * @param data Pointer to a ruler instance.
     */
    static void sizeAllocateCallback(GtkWidget *widget, GdkRectangle *allocation, gpointer data);

    /**
     * Calculates an appropriate interval between major ticks, given the current range and dimensions.
     */
    void calculateTickIntervals();

    /**
     * Draws the tick marks of the ruler for a given subset of the range.
     * @param cr Cairo context to draw to.
     * @param lower The lower limit of the range to draw.
     * @param upper The upper limit of the range to draw.
     * @param lowerToUpper True if the ticks should be drawn from lower to upper. False if from upper to lower.
     * @param lineLength Length of the lines in pixels.
     */
    void drawTicks(cairo_t *cr, double lower, double upper, bool lowerToUpper, double lineLength);

    /**
     * Draws a single tick, taking into account the ruler's orientation.
     * @param cr Cairo context to draw to.
     * @param linePosition The position of the line along the ruler.
     * @param lineLength Length of the line in pixels.
     * @param drawLabel True if a label should be drawn to the right/top of the line.
     * @param label The label to draw if \p drawLabel is true.
     */
    void drawSingleTick(cairo_t *cr, double linePosition, double lineLength, bool drawLabel, const std::string &label);

    /**
     * Draws the smaller ticks in between the major ticks.
     * @param cr Cairo context to draw to.
     * @param lower The lower limit of the range in draw space.
     * @param upper The upper limit of the range in draw space.
     * @param depth The depth of this recursive function. Functions as an index into the ruler's SUBTICK_SEGMENTS array.
     * @param lineLength Length of the lines in pixels.
     * @param lowerToUpper True if the ticks should be drawn from lower to upper. False if from upper to lower.
     */
    void drawSubTicks(cairo_t *cr, double lower, double upper, int depth, double lineLength, bool lowerToUpper);
};

/**
 * This class contains the functions a Ruler uses to calculate the interval between major ticks.
 */
class RulerCalculations
{
private:
    /** The minimum space between major ticks. */
    static constexpr int MIN_SPACE_MAJORTICKS{80};

    /** Valid intervals between major ticks. */
    constexpr static std::array<int, 4> VALID_INTERVALS{
            1,  5, 10, 25
    };

public:
    /**
     * Calculates an appropriate interval between major ticks on a ruler.
     * @param lower Lower limit of the ruler range. Must be strictly less than \p upper.
     * @param upper Upper limit of the ruler range. Must be strictly greater than \p lower.
     * @param allocatedSize The allocated width/height in pixels for the ruler.
     * @return The interval between ticks, or -1 if the given range is invalid.
     */
    static int calculateInterval(double lower, double upper, double allocatedSize);

    /**
     * Calculates the spacing in pixels between tick marks for a given interval.
     * @param interval The interval to calculate the spacing for.
     * @param lower Lower limit of the ruler range. Must be strictly less than \p upper.
     * @param upper Upper limit of the ruler range. Must be strictly greater than \p lower.
     * @param allocatedSize The allocated width/height in pixels for the ruler.
     * @return The spacing in pixels between tick marks for a given interval.
     */
    static int intervalPixelSpacing(double interval, double lower, double upper, double allocatedSize);

    /**
     * Scales a number \p x in the range [\p src_lower, \p src_upper] to the range [\p dest_lower, \p dest_upper].
     * Used to scale from the ruler range to the drawing space.
     * @param x The number to scale.
     * @param src_lower The lower limit of the source range. Inclusive.
     * @param src_upper The upper limit of the source range. Inclusive.
     * @param dest_lower The lower limit of the destination range. Inclusive.
     * @param dest_upper The upper limit of the destination range. Inclusive.
     * @return The result of \p x scaled from range source to dest.
     */
    static double scaleToRange(double x, double src_lower, double src_upper, double dest_lower, double dest_upper);
};