#ifndef _VIEW_HH
#define _VIEW_HH

#include <map>

#include <glade/glade.h>
#include <gdk/gdk.h>
#include <cairo.h>

#include <scroominterface.hh>
#include <viewinterface.hh>
#include <presentationinterface.hh>

struct Measurement
{
public:
  GdkPoint start;
  GdkPoint end;

public:
  Measurement(int x, int y) { start.x=x; start.y=y; end=start; }
  Measurement(GdkPoint start) : start(start), end(start) {}

  bool endsAt(GdkPoint p) { return end.x==p.x && end.y==p.y; }
};

class View : public ViewInterface
{
private:
  GladeXML* scroomXml;
  PresentationInterface* presentation;
  GtkWidget* drawingArea;
  int drawingAreaWidth;
  int drawingAreaHeight;
  GdkRectangle presentationRect;
  GtkVScrollbar* vscrollbar;
  GtkHScrollbar* hscrollbar;
  GtkAdjustment* vscrollbaradjustment;
  GtkAdjustment* hscrollbaradjustment;
  GtkRuler* hruler;
  GtkRuler* vruler;
  GtkComboBox* zoomBox;
  GtkListStore* zoomItems;
  GtkProgressBar* progressBar;
  int zoom;
  int x;
  int y;
  ViewIdentifier* vid;
  Measurement* measurement;

  gint modifiermove;
  GdkPoint cachedPoint;
  
public:

  View(GladeXML* scroomXml, PresentationInterface* presentation);
  virtual ~View();

  void redraw(cairo_t* cr);
  bool hasPresentation();
  void setPresentation(PresentationInterface* presentation);

  void updateScrollbar(GtkAdjustment* adj, int zoom, int value,
                       int presentationStart, int presentationSize, int windowSize);
  void updateScrollbars();
  void updateZoom();
  void updateRulers();

  ////////////////////////////////////////////////////////////////////////
  // Scroom events
  
  void on_newInterfaces_update(const std::map<NewInterface*, std::string>& newInterfaces);
  void on_configure();
  void on_window_size_changed(int newWidth, int newHeight);
  void on_zoombox_changed();
  void on_zoombox_changed(int newZoom, int mousex, int mousey);
  void on_scrollbar_value_changed(GtkAdjustment* adjustment);
  void on_scrollwheel(GdkEventScroll* event);
  void on_buttonPress(GdkEventButton* event);
  void on_buttonRelease(GdkEventButton* event);
  void on_motion_notify(GdkEventMotion* event);

  ////////////////////////////////////////////////////////////////////////
  // Presentation events

  virtual void invalidate();
  virtual GtkProgressBar* getProgressBar();

  ////////////////////////////////////////////////////////////////////////
  // Helpers

private:
  GdkPoint windowPointToPresentationPoint(GdkPoint wp);
  GdkPoint presentationPointToWindowPoint(GdkPoint pp);
  GdkPoint eventToPoint(GdkEventButton* event);
  GdkPoint eventToPoint(GdkEventMotion* event);
  void drawCross(cairo_t* cr, GdkPoint p);
};

#endif
