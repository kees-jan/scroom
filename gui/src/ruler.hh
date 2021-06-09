#pragma once

#include <gtk/gtk.h>
#include <boost/smart_ptr.hpp>

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
     * @param lower The lower limit of the range.
     * @param upper The upper limit of the range.
     */
    void setRange(double lower, double upper);

private:

    GtkWidget *drawingArea{};

    static constexpr double DEFAULT_LOWER{0};
    static constexpr double DEFAULT_UPPER{10};

    /** The minimum space between major ticks. */
    static constexpr int MIN_SEGMENT_SIZE{80};

    /** The minimum space between sub-ticks. */
    static constexpr int MIN_SPACE_SUBTICKS{5};

    /** Valid intervals between major ticks. */
    constexpr static std::array<double, 5> VALID_INTERVALS{
            1, 2, 5, 10, 25
    };

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

    // The width and height of the drawing area widget.
    double width{};
    double height{};

    /** The chosen interval between major ticks. */
    double majorInterval{1};

    /** The space between major ticks when drawn. */
    int segmentScreenSize{};

    // ==== DRAWING PROPERTIES ====

    /**
     * Cairo integer coordinates map to points halfway between pixels.
     * Therefore, if we offset coordinates by 0.5 in the appropriate
     * direction, we can draw clear lines.
     */
    static constexpr double LINE_COORD_OFFSET{0.5};

    static constexpr double FONT_SIZE{11};

    static constexpr double LABEL_OFFSET{4};

    /** The length of a tick one "level" down, as a fraction of the line length of the ticks one level up. */
    static constexpr double LINE_MULTIPLIER{0.5};

    GdkRGBA lineColor{0, 0, 0, 1};

    static constexpr double LINE_WIDTH{1};

    /** Length of the major tick lines as a fraction of the width/height. */
    static constexpr double MAJOR_TICK_LENGTH{0.8};

    /**
     * Creates a ruler.
     */
    explicit Ruler(Orientation orientation);

    /**
     * Disconnects signal handlers from the current drawing area, if one is set.
     */
    void unregisterDrawingArea();

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
     * A callback to be connected to a GtkDrawingArea's "size-allocate" signal.
     * Updates the internal state of the ruler when the size of the ruler changes.
     * @param widget The widget that received the signal.
     * @param allocation The region which has been allocated to the widget.
     * @param data Pointer to a ruler instance.
     */
    static void sizeAllocateCallback(GtkWidget *widget, GdkRectangle *allocation, gpointer data);

    /**
     * Updates the internal state of the ruler, given the current range and dimensions.
     */
    void update();

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
     * @param lineOrigin The point at which to draw the line.
     * @param lineLength Length of the line in pixels.
     * @param drawLabel True if a label should be drawn to the right/top of the line.
     * @param label The label to draw if \p drawLabel is true.
     */
    void drawSingleTick(cairo_t *cr, double lineOrigin, double lineLength, bool drawLabel, const std::string &label);

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

    /**
     * Registers the required callbacks with the drawing area.
     * @param widget Drawing area to draw the ruler to.
     */
    void setDrawingArea(GtkWidget *widget);

    /**
     * Maps x from range a to range b.
     * @param x The input to map from range a to b.
     * @param a_lower The lower limit of range a.
     * @param a_upper The upper limit of range a.
     * @param b_lower The lower limit of range b.
     * @param b_upper The upper limit of range b.
     * @return The result of \p x mapped from range a to b.
     */
    static double mapRange(double x, double a_lower, double a_upper, double b_lower, double b_upper);
};