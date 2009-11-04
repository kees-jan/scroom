#ifndef _VIEW_HH
#define _VIEW_HH

#include <map>

#include <glade/glade.h>
#include <cairo.h>

#include <scroominterface.hh>
#include <viewinterface.hh>
#include <presentationinterface.hh>

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
  int zoom;
  int x;
  int y;
  ViewIdentifier* vid;
  
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

  ////////////////////////////////////////////////////////////////////////
  // Presentation events

  virtual void invalidate();
};

#endif
