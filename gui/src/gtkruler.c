/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

/*
 * Ported to GTK3 + Cairo by Jelle Bootsma for the Scroom large image viewer
 * See SCROOM at https://github.com/kees-jan/scroom
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */
//#include "config.h"

#include <math.h>
#include <string.h>

#undef GDK_DISABLE_DEPRECATED /* We need gdk_drawable_get_size() */
#undef GTK_DISABLE_DEPRECATED

#include <gtk/gtk.h>
#include "gtkruler.h"
//#include "gtkprivate.h"
//#include "gtkintl.h"
//#include "gtkalias.h"


#define RULER_WIDTH           14
#define MINIMUM_INCR          5
#define MAXIMUM_SUBDIVIDE     5
#define MAXIMUM_SCALES        10

#define ROUND(x) ((int) ((x) + 0.5))

enum {
  PROP_0,
  PROP_ORIENTATION,
  PROP_LOWER,
  PROP_UPPER,
  PROP_POSITION,
  PROP_MAX_SIZE,
  PROP_METRIC
};

GType GTK_TYPE_METRIC_TYPE = 0;

typedef struct _GtkRulerPrivate GtkRulerPrivate;

struct _GtkRulerPrivate
{
  GtkOrientation orientation;
};

#define GTK_RULER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_TYPE_RULER, GtkRulerPrivate))


static void     gtk_ruler_set_property    (GObject        *object,
                                           guint            prop_id,
                                           const GValue   *value,
                                           GParamSpec     *pspec);
static void     gtk_ruler_get_property    (GObject        *object,
                                           guint           prop_id,
                                           GValue         *value,
                                           GParamSpec     *pspec);
static void     gtk_ruler_realize         (GtkWidget      *widget);
static void     gtk_ruler_unrealize       (GtkWidget      *widget);
static void     gtk_ruler_size_request    (GtkWidget      *widget,
                                           GtkRequisition *requisition);
static void     gtk_ruler_size_allocate   (GtkWidget      *widget,
                                           GtkAllocation  *allocation);
static gboolean gtk_ruler_motion_notify   (GtkWidget      *widget,
                                           GdkEventMotion *event);
static gboolean gtk_ruler_expose          (GtkWidget      *widget,
                                           GdkEventExpose *event);
static void     gtk_ruler_make_pixmap     (GtkRuler       *ruler);
static void     gtk_ruler_real_draw_ticks (GtkRuler       *ruler);
static void     gtk_ruler_real_draw_pos   (GtkRuler       *ruler);


static const GtkRulerMetric ruler_metrics[] =
{
  { "Pixel", "Pi", 1.0, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  { "Inches", "In", 72.0, { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 }, { 1, 2, 4, 8, 16 }},
  { "Centimeters", "Cn", 28.35, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
};


G_DEFINE_TYPE_WITH_CODE (GtkRuler, gtk_ruler, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE,
                                                NULL))
/* main method only used for buildabilty checking
 * and is temporary until a proper build pipeline is set up 
 */
int main() {}


static void
gtk_ruler_class_init (GtkRulerClass *class)
{
  GObjectClass   *gobject_class = G_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class  = GTK_WIDGET_CLASS (class);

  gobject_class->set_property = gtk_ruler_set_property;
  gobject_class->get_property = gtk_ruler_get_property;

  widget_class->realize = gtk_ruler_realize;
  widget_class->unrealize = gtk_ruler_unrealize;
  widget_class->size_allocate = gtk_ruler_size_allocate;
  widget_class->motion_notify_event = gtk_ruler_motion_notify;
  //widget_class->expose_event = gtk_ruler_expose;

  class->draw_ticks = gtk_ruler_real_draw_ticks;
  class->draw_pos = gtk_ruler_real_draw_pos;

  g_object_class_override_property (gobject_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_install_property (gobject_class,
                                   PROP_LOWER,
                                   g_param_spec_double ("lower",
							"Lower",
							"Lower limit of ruler",
							-G_MAXDOUBLE,
							G_MAXDOUBLE,
							0.0,
							G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_UPPER,
                                   g_param_spec_double ("upper",
							"Upper",
							"Upper limit of ruler",
							-G_MAXDOUBLE,
							G_MAXDOUBLE,
							0.0,
							G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_POSITION,
                                   g_param_spec_double ("position",
							"Position",
							"Position of mark on the ruler",
							-G_MAXDOUBLE,
							G_MAXDOUBLE,
							0.0,
							G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_MAX_SIZE,
                                   g_param_spec_double ("max-size",
							"Max Size",
							"Maximum size of the ruler",
							-G_MAXDOUBLE,
							G_MAXDOUBLE,
							0.0,
                            G_PARAM_READWRITE));
  /**
   * GtkRuler:metric:
   *
   * The metric used for the ruler.
   *
   * Since: 2.8
   */
  g_object_class_install_property (gobject_class,
                                   PROP_METRIC,
                                   g_param_spec_enum ("metric",
						      "Metric",
						      "The metric used for the ruler",
						      GTK_TYPE_METRIC_TYPE,
						      GTK_PIXELS,
						      G_PARAM_READWRITE));

  g_type_class_add_private (gobject_class, sizeof (GtkRulerPrivate));
}

static void
gtk_ruler_init (GtkRuler *ruler)
{
  GtkWidget *widget = GTK_WIDGET (ruler);
  GtkRulerPrivate *private = GTK_RULER_GET_PRIVATE (ruler);

  private->orientation = GTK_ORIENTATION_HORIZONTAL;

  gint requestedWidth = gtk_widget_get_style(widget)->xthickness * 2 + 1;
  gint requestedHeight = gtk_widget_get_style(widget)->ythickness * 2 + RULER_WIDTH;
  gtk_widget_set_size_request(widget, requestedWidth, requestedHeight);

  ruler->backing_store = NULL;
  ruler->xsrc = 0;
  ruler->ysrc = 0;
  ruler->slider_size = 0;
  ruler->lower = 0;
  ruler->upper = 0;
  ruler->position = 0;
  ruler->max_size = 0;

  gtk_ruler_set_metric (ruler, GTK_PIXELS);
}

static void
gtk_ruler_set_property (GObject      *object,
 			guint         prop_id,
			const GValue *value,
			GParamSpec   *pspec)
{
  GtkRuler *ruler = GTK_RULER (object);
  GtkRulerPrivate *private = GTK_RULER_GET_PRIVATE (ruler);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      private->orientation = g_value_get_enum (value);
      gtk_widget_queue_resize (GTK_WIDGET (ruler));
      break;
    case PROP_LOWER:
      gtk_ruler_set_range (ruler, g_value_get_double (value), ruler->upper,
			   ruler->position, ruler->max_size);
      break;
    case PROP_UPPER:
      gtk_ruler_set_range (ruler, ruler->lower, g_value_get_double (value),
			   ruler->position, ruler->max_size);
      break;
    case PROP_POSITION:
      gtk_ruler_set_range (ruler, ruler->lower, ruler->upper,
			   g_value_get_double (value), ruler->max_size);
      break;
    case PROP_MAX_SIZE:
      gtk_ruler_set_range (ruler, ruler->lower, ruler->upper,
			   ruler->position,  g_value_get_double (value));
      break;
    case PROP_METRIC:
      gtk_ruler_set_metric (ruler, g_value_get_enum (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gtk_ruler_get_property (GObject      *object,
			guint         prop_id,
			GValue       *value,
			GParamSpec   *pspec)
{
  GtkRuler *ruler = GTK_RULER (object);
  GtkRulerPrivate *private = GTK_RULER_GET_PRIVATE (ruler);

  switch (prop_id)
    {
    case PROP_ORIENTATION:
      g_value_set_enum (value, private->orientation);
      break;
    case PROP_LOWER:
      g_value_set_double (value, ruler->lower);
      break;
    case PROP_UPPER:
      g_value_set_double (value, ruler->upper);
      break;
    case PROP_POSITION:
      g_value_set_double (value, ruler->position);
      break;
    case PROP_MAX_SIZE:
      g_value_set_double (value, ruler->max_size);
      break;
    case PROP_METRIC:
      g_value_set_enum (value, gtk_ruler_get_metric(ruler));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

void
gtk_ruler_set_metric (GtkRuler      *ruler,
		       GtkMetricType  metric)
{
  g_return_if_fail (GTK_IS_RULER (ruler));

  ruler->metric = (GtkRulerMetric *) &ruler_metrics[metric];

  if (gtk_widget_is_drawable (GTK_WIDGET (ruler)))
    gtk_widget_queue_draw (GTK_WIDGET (ruler));

  g_object_notify (G_OBJECT (ruler), "metric");
}

/**
 * gtk_ruler_get_metric:
 * @ruler: a #GtkRuler
 *
 * Gets the units used for a #GtkRuler. See gtk_ruler_set_metric().
 *
 * Return value: the units currently used for @ruler
 *
 * @Deprecated: 2.24: #GtkRuler has been removed from GTK 3 for being
 *              unmaintained and too specialized. There is no replacement.
 **/
GtkMetricType gtk_ruler_get_metric (GtkRuler *ruler)
{
  gint i;

  g_return_val_if_fail (GTK_IS_RULER (ruler), 0);

  for (i = 0; i < G_N_ELEMENTS (ruler_metrics); i++)
    if (ruler->metric == &ruler_metrics[i])
      return i;

  g_assert_not_reached ();

  return 0;
}

/**
 * gtk_ruler_set_range:
 * @ruler: the gtkruler
 * @lower: the lower limit of the ruler
 * @upper: the upper limit of the ruler
 * @position: the mark on the ruler
 * @max_size: the maximum size of the ruler used when calculating the space to
 * leave for the text
 *
 * This sets the range of the ruler. 
 *
 * @Deprecated: 2.24: #GtkRuler has been removed from GTK 3 for being
 *              unmaintained and too specialized. There is no replacement.
 */
void
gtk_ruler_set_range (GtkRuler *ruler,
		     gdouble   lower,
		     gdouble   upper,
		     gdouble   position,
		     gdouble   max_size)
{
  g_return_if_fail (GTK_IS_RULER (ruler));

  g_object_freeze_notify (G_OBJECT (ruler));
  if (ruler->lower != lower)
    {
      ruler->lower = lower;
      g_object_notify (G_OBJECT (ruler), "lower");
    }
  if (ruler->upper != upper)
    {
      ruler->upper = upper;
      g_object_notify (G_OBJECT (ruler), "upper");
    }
  if (ruler->position != position)
    {
      ruler->position = position;
      g_object_notify (G_OBJECT (ruler), "position");
    }
  if (ruler->max_size != max_size)
    {
      ruler->max_size = max_size;
      g_object_notify (G_OBJECT (ruler), "max-size");
    }
  g_object_thaw_notify (G_OBJECT (ruler));

  if (gtk_widget_is_drawable (GTK_WIDGET (ruler)))
    gtk_widget_queue_draw (GTK_WIDGET (ruler));
}

/**
 * gtk_ruler_get_range:
 * @ruler: a #GtkRuler
 * @lower: (allow-none): location to store lower limit of the ruler, or %NULL
 * @upper: (allow-none): location to store upper limit of the ruler, or %NULL
 * @position: (allow-none): location to store the current position of the mark on the ruler, or %NULL
 * @max_size: location to store the maximum size of the ruler used when calculating
 *            the space to leave for the text, or %NULL.
 *
 * Retrieves values indicating the range and current position of a #GtkRuler.
 * See gtk_ruler_set_range().
 *
 * @Deprecated: 2.24: #GtkRuler has been removed from GTK 3 for being
 *              unmaintained and too specialized. There is no replacement.
 **/
void
gtk_ruler_get_range (GtkRuler *ruler,
		     gdouble  *lower,
		     gdouble  *upper,
		     gdouble  *position,
		     gdouble  *max_size)
{
  g_return_if_fail (GTK_IS_RULER (ruler));

  if (lower)
    *lower = ruler->lower;
  if (upper)
    *upper = ruler->upper;
  if (position)
    *position = ruler->position;
  if (max_size)
    *max_size = ruler->max_size;
}

void
gtk_ruler_draw_ticks (GtkRuler *ruler)
{
  g_return_if_fail (GTK_IS_RULER (ruler));

  if (GTK_RULER_GET_CLASS (ruler)->draw_ticks)
    GTK_RULER_GET_CLASS (ruler)->draw_ticks (ruler);
}

void
gtk_ruler_draw_pos (GtkRuler *ruler)
{
  g_return_if_fail (GTK_IS_RULER (ruler));

  if (GTK_RULER_GET_CLASS (ruler)->draw_pos)
     GTK_RULER_GET_CLASS (ruler)->draw_pos (ruler);
}


static void
gtk_ruler_realize (GtkWidget *widget)
{
  GtkRuler *ruler;
  GdkWindowAttr attributes;
  gint attributes_mask;

  ruler = GTK_RULER (widget);

  gtk_widget_set_realized (widget, TRUE);
  struct _cairo_rectangle_int* allocation;
  attributes.window_type = GDK_WINDOW_CHILD;
  gtk_widget_get_allocation(widget, allocation);
  attributes.x =  allocation->x;
  attributes.y = allocation->y;
  attributes.width = allocation->width;
  attributes.height = allocation->height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);

  attributes.event_mask = gtk_widget_get_events (widget);
  attributes.event_mask |= (GDK_EXPOSURE_MASK |
			    GDK_POINTER_MOTION_MASK |
			    GDK_POINTER_MOTION_HINT_MASK);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

  gtk_widget_set_window(widget, gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask)) ;
  gdk_window_set_user_data (gtk_widget_get_window(widget), ruler);

  gtk_widget_set_style(widget, gtk_style_attach (gtk_widget_get_style(widget), gtk_widget_get_window(widget)));
  gtk_style_set_background (gtk_widget_get_style(widget), gtk_widget_get_window(widget), GTK_STATE_ACTIVE);

  gtk_ruler_make_pixmap (ruler);
}

static void
gtk_ruler_unrealize (GtkWidget *widget)
{
  GtkRuler *ruler = GTK_RULER (widget);

  if (ruler->backing_store)
    {
      g_object_unref (ruler->backing_store);
      ruler->backing_store = NULL;
    }

  GTK_WIDGET_CLASS (gtk_ruler_parent_class)->unrealize (widget);
}

static void
gtk_ruler_size_request (GtkWidget      *widget,
                        GtkRequisition *requisition)
{
  GtkRulerPrivate *private = GTK_RULER_GET_PRIVATE (widget);

  if (private->orientation == GTK_ORIENTATION_HORIZONTAL)
    {

      requisition->width  = gtk_widget_get_style(widget)->xthickness * 2 + 1;
      requisition->height = gtk_widget_get_style(widget)->ythickness* 2 + RULER_WIDTH;
    }
  else
    {
      requisition->width  = gtk_widget_get_style(widget)->xthickness * 2 + RULER_WIDTH;
      requisition->height = gtk_widget_get_style(widget)->ythickness * 2 + 1;
    }
}

static void
gtk_ruler_size_allocate (GtkWidget     *widget,
			 struct _cairo_rectangle_int *allocation)
{
  GtkRuler *ruler = GTK_RULER (widget);

    gtk_widget_set_allocation(widget, allocation);

  if (gtk_widget_get_realized (widget))
    {
      gdk_window_move_resize (gtk_widget_get_window(widget),
			      allocation->x, allocation->y,
			      allocation->width, allocation->height);

      gtk_ruler_make_pixmap (ruler);
    }
}

static gboolean
gtk_ruler_motion_notify (GtkWidget      *widget,
                         GdkEventMotion *event)
{
  GtkRuler *ruler = GTK_RULER (widget);
  GtkRulerPrivate *private = GTK_RULER_GET_PRIVATE (widget);
  gint x;
  gint y;

  gdk_event_request_motions (event);
  x = event->x;
  y = event->y;

  if (private->orientation == GTK_ORIENTATION_HORIZONTAL)
    ruler->position = ruler->lower + ((ruler->upper - ruler->lower) * x) / (float)gtk_widget_get_allocated_width(widget);
  else
    ruler->position = ruler->lower + ((ruler->upper - ruler->lower) * y) / (float)gtk_widget_get_allocated_height(widget);

  g_object_notify (G_OBJECT (ruler), "position");

  /*  Make sure the ruler has been allocated already  */
  if (ruler->backing_store != NULL)
    gtk_ruler_draw_pos (ruler);

  return FALSE;
}

static gboolean
gtk_ruler_expose (GtkWidget      *widget,
		  GdkEventExpose *event)
{
  if (gtk_widget_is_drawable (widget))
    {
      GtkRuler *ruler = GTK_RULER (widget);
      cairo_t *cr;

      gtk_ruler_draw_ticks (ruler);
      
      cr = gdk_cairo_create (gtk_widget_get_window(widget));
      cairo_set_source_surface(cr, ruler->backing_store, 0, 0);
      gdk_cairo_rectangle (cr, &event->area);
      cairo_fill (cr);
      cairo_destroy (cr);
      
      gtk_ruler_draw_pos (ruler);
    }

  return FALSE;
}

static void
gtk_ruler_make_pixmap (GtkRuler *ruler)
{
  GtkWidget *widget;
  gint width;
  gint height;

  widget = GTK_WIDGET (ruler);

  if (ruler->backing_store)
    {
      width = cairo_image_surface_get_width(ruler->backing_store);
      height = cairo_image_surface_get_height(ruler->backing_store);

      if ((width == gtk_widget_get_allocated_width(widget)) &&
	  (height == gtk_widget_get_allocated_height(widget)))
	return;

      g_object_unref (ruler->backing_store);
    }
  ruler->xsrc = 0;
  ruler->ysrc = 0;
  ruler->backing_store = cairo_surface_create_for_rectangle(gtk_widget_get_window(widget),
                                                            ruler->xsrc,
                                                            ruler->ysrc,
                                                            gtk_widget_get_allocated_width(widget),
                                                            gtk_widget_get_allocated_height(widget));


}

static void
gtk_ruler_real_draw_ticks (GtkRuler *ruler)
{
  GtkWidget *widget = GTK_WIDGET (ruler);
  GtkRulerPrivate *private = GTK_RULER_GET_PRIVATE (ruler);
  cairo_t *cr;
  gint i, j;
  gint width, height;
  gint xthickness;
  gint ythickness;
  gint length, ideal_length;
  gdouble lower, upper;		/* Upper and lower limits, in ruler units */
  gdouble increment;		/* Number of pixels per unit */
  gint scale;			/* Number of units per major unit */
  gdouble subd_incr;
  gdouble start, end, cur;
  gchar unit_str[32];
  gint digit_height;
  gint digit_offset;
  gint text_width;
  gint text_height;
  gint pos;
  PangoLayout *layout;
  PangoRectangle logical_rect, ink_rect;

  if (!gtk_widget_is_drawable (widget))
    return;

  xthickness = gtk_widget_get_style(widget)->xthickness;
  ythickness = gtk_widget_get_style(widget)->ythickness;

  layout = gtk_widget_create_pango_layout (widget, "012456789");
  pango_layout_get_extents (layout, &ink_rect, &logical_rect);

  digit_height = PANGO_PIXELS (ink_rect.height) + 2;
  digit_offset = ink_rect.y;

  if (private->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      width = gtk_widget_get_allocated_width(widget);
      height = gtk_widget_get_allocated_height(widget) - ythickness * 2;
    }
  else
    {
      width = gtk_widget_get_allocated_height(widget);
      height = gtk_widget_get_allocated_width(widget) - ythickness * 2;
    }

#define DETAILE(private) (private->orientation == GTK_ORIENTATION_HORIZONTAL ? "hruler" : "vruler");
  cr = gdk_cairo_create (ruler->backing_store);
  gtk_paint_box (gtk_widget_get_style(widget),
                 cr,
                 GTK_STATE_NORMAL,
                 GTK_SHADOW_OUT,
                 widget,
                 private->orientation == GTK_ORIENTATION_HORIZONTAL ? "hruler" : "vruler",
                 0,
                 0,
                 gtk_widget_get_allocated_width(widget),
                 gtk_widget_get_allocated_height(widget));


  gdk_cairo_set_source_color (cr,
                              &gtk_widget_get_style(widget)->fg[gtk_widget_get_state(widget)]);

  if (private->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      cairo_rectangle (cr,
                       xthickness,
                       height + ythickness,
                       gtk_widget_get_allocated_width(widget) - 2 * xthickness,
                       1);
    }
  else
    {
      cairo_rectangle (cr,
                       height + xthickness,
                       ythickness,
                       1,
                       gtk_widget_get_allocated_height(widget) - 2 * ythickness);
    }

  upper = ruler->upper / ruler->metric->pixels_per_unit;
  lower = ruler->lower / ruler->metric->pixels_per_unit;

  if ((upper - lower) == 0)
    goto out;

  increment = (gdouble) width / (upper - lower);

  /* determine the scale H
   *  We calculate the text size as for the vruler instead of using
   *  text_width = gdk_string_width(font, unit_str), so that the result
   *  for the scale looks consistent with an accompanying vruler
   */
  /* determine the scale V
   *   use the maximum extents of the ruler to determine the largest
   *   possible number to be displayed.  Calculate the height in pixels
   *   of this displayed text. Use this height to find a scale which
   *   leaves sufficient room for drawing the ruler.
   */
  scale = ceil (ruler->max_size / ruler->metric->pixels_per_unit);
  g_snprintf (unit_str, sizeof (unit_str), "%d", scale);

  if (private->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      text_width = strlen (unit_str) * digit_height + 1;

      for (scale = 0; scale < MAXIMUM_SCALES; scale++)
        if (ruler->metric->ruler_scale[scale] * fabs(increment) > 2 * text_width)
          break;
    }
  else
    {
      text_height = strlen (unit_str) * digit_height + 1;

      for (scale = 0; scale < MAXIMUM_SCALES; scale++)
        if (ruler->metric->ruler_scale[scale] * fabs(increment) > 2 * text_height)
          break;
    }

  if (scale == MAXIMUM_SCALES)
    scale = MAXIMUM_SCALES - 1;

  /* drawing starts here */
  length = 0;
  for (i = MAXIMUM_SUBDIVIDE - 1; i >= 0; i--)
    {
      subd_incr = (gdouble) ruler->metric->ruler_scale[scale] /
	          (gdouble) ruler->metric->subdivide[i];
      if (subd_incr * fabs(increment) <= MINIMUM_INCR)
	continue;

      /* Calculate the length of the tickmarks. Make sure that
       * this length increases for each set of ticks
       */
      ideal_length = height / (i + 1) - 1;
      if (ideal_length > ++length)
	length = ideal_length;

      if (lower < upper)
	{
	  start = floor (lower / subd_incr) * subd_incr;
	  end   = ceil  (upper / subd_incr) * subd_incr;
	}
      else
	{
	  start = floor (upper / subd_incr) * subd_incr;
	  end   = ceil  (lower / subd_incr) * subd_incr;
	}

      for (cur = start; cur <= end; cur += subd_incr)
	{
	  pos = ROUND ((cur - lower) * increment);

          if (private->orientation == GTK_ORIENTATION_HORIZONTAL)
            {
              cairo_rectangle (cr,
                               pos, height + ythickness - length,
                               1,   length);
            }
          else
            {
              cairo_rectangle (cr,
                               height + xthickness - length, pos,
                               length,                       1);
            }

	  /* draw label */
	  if (i == 0)
	    {
	      g_snprintf (unit_str, sizeof (unit_str), "%d", (int) cur);

              if (private->orientation == GTK_ORIENTATION_HORIZONTAL)
                {
                  pango_layout_set_text (layout, unit_str, -1);
                  pango_layout_get_extents (layout, &logical_rect, NULL);

                  gtk_paint_layout (gtk_widget_get_style(widget),
                                    cr,
                                    gtk_widget_get_state (widget),
                                    FALSE,
                                    widget,
                                    "hruler",
                                    pos + 2, ythickness + PANGO_PIXELS (logical_rect.y - digit_offset),
                                    layout);
                }
              else
                {
                  for (j = 0; j < (int) strlen (unit_str); j++)
                    {
                      pango_layout_set_text (layout, unit_str + j, 1);
                      pango_layout_get_extents (layout, NULL, &logical_rect);

                      gtk_paint_layout (gtk_widget_get_style(widget),
                                        cr,
                                        gtk_widget_get_state (widget),
                                        FALSE,
                                        widget,
                                        "vruler",
                                        xthickness + 1,
                                        pos + digit_height * j + 2 + PANGO_PIXELS (logical_rect.y - digit_offset),
                                        layout);
                    }
                }
	    }
	}
    }

  cairo_fill (cr);
out:
  cairo_destroy (cr);

  g_object_unref (layout);
}

static void
gtk_ruler_real_draw_pos (GtkRuler *ruler)
{
  GtkWidget *widget = GTK_WIDGET (ruler);
  GtkRulerPrivate *private = GTK_RULER_GET_PRIVATE (ruler);
  gint x, y;
  gint width, height;
  gint bs_width, bs_height;
  gint xthickness;
  gint ythickness;
  gdouble increment;

  if (gtk_widget_is_drawable (widget))
    {
      xthickness = gtk_widget_get_style(widget)->xthickness;
      ythickness = gtk_widget_get_style(widget)->ythickness;
      width = gtk_widget_get_allocated_width(widget);
      height = gtk_widget_get_allocated_height(widget);

      if (private->orientation == GTK_ORIENTATION_HORIZONTAL)
        {
          height -= ythickness * 2;

          bs_width = height / 2 + 2;
          bs_width |= 1;  /* make sure it's odd */
          bs_height = bs_width / 2 + 1;
        }
      else
        {
          width -= xthickness * 2;

          bs_height = width / 2 + 2;
          bs_height |= 1;  /* make sure it's odd */
          bs_width = bs_height / 2 + 1;
        }

      if ((bs_width > 0) && (bs_height > 0))
	{
	  cairo_t *cr = gdk_cairo_create (gtk_widget_get_window(widget));

	  /*  If a backing store exists, restore the ruler  */
	  if (ruler->backing_store)
            {
              cairo_t *cr = gdk_cairo_create (gtk_widget_get_window(widget));

              cairo_set_source_surface(cr, ruler->backing_store, 0, 0);
              cairo_rectangle (cr, ruler->xsrc, ruler->ysrc, bs_width, bs_height);
              cairo_fill (cr);

              cairo_destroy (cr);
            }

          if (private->orientation == GTK_ORIENTATION_HORIZONTAL)
            {
              increment = (gdouble) width / (ruler->upper - ruler->lower);

              x = ROUND ((ruler->position - ruler->lower) * increment) + (xthickness - bs_width) / 2 - 1;
              y = (height + bs_height) / 2 + ythickness;
            }
          else
            {
              increment = (gdouble) height / (ruler->upper - ruler->lower);

              x = (width + bs_width) / 2 + xthickness;
              y = ROUND ((ruler->position - ruler->lower) * increment) + (ythickness - bs_height) / 2 - 1;
            }

	  gdk_cairo_set_source_color (cr, &gtk_widget_get_style(widget)->fg[gtk_widget_get_state(widget)]);

	  cairo_move_to (cr, x, y);

          if (private->orientation == GTK_ORIENTATION_HORIZONTAL)
            {
              cairo_line_to (cr, x + bs_width / 2.0, y + bs_height);
              cairo_line_to (cr, x + bs_width,       y);
            }
          else
            {
              cairo_line_to (cr, x + bs_width, y + bs_height / 2.0);
              cairo_line_to (cr, x,            y + bs_height);
            }

	  cairo_fill (cr);

	  cairo_destroy (cr);

	  ruler->xsrc = x;
	  ruler->ysrc = y;
	}
    }
}

#define __GTK_RULER_C__
//#include "gtkaliasdef.c"
