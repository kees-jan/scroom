/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "view.hh"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <array>
#include <cmath>
#include <sstream>

#include <spdlog/spdlog.h>

#include <boost/lexical_cast.hpp>

#include <glib-object.h>

#include <scroom/assertions.hh>
#include <scroom/cairo-helpers.hh>
#include <scroom/rounding.hh>

#include "callbacks.hh"
#include "pluginmanager.hh"

#ifndef G_VALUE_INIT
#  define G_VALUE_INIT \
    {                  \
      0,               \
      {                \
        {              \
          0            \
        }              \
      }                \
    }
#endif

static const std::array<const char*, 36> zoomfactor = {
  "32:1",          "16:1",          "8:1",           "4:1",         "2:1",         "1:1",          "1:2",          "1:4",
  "1:8",           "1:16",          "1:32",          "1:64",        "1:128",       "1:250",        "1:500",        "1:1000",
  "1:2000",        "1:4000",        "1:8000",        "1:16000",     "1:32000",     "1:64000",      "1:128000",     "1:250000",
  "1:500000",      "1:1 million",   "1:2 million",   "1:4 million", "1:8 million", "1:16 million", "1:32 million", "1:64 million",
  "1:128 million", "1:250 million", "1:500 million", "1:1 billion",
};
static const int MaxZoom = 5;

enum
{
  COLUMN_TEXT,
  COLUMN_ZOOM,
  N_COLUMNS
};

////////////////////////////////////////////////////////////////////////
/// Tweakers

class TweakPresentationPosition
{
public:
  using Ptr   = boost::shared_ptr<TweakPresentationPosition>;
  using Point = Scroom::Utils::Point<double>;

  static Ptr create(Point aspectRatio_) { return Ptr(new TweakPresentationPosition(aspectRatio_)); }

  Point tweakPosition(Point currentPosition, Scroom::Utils::Point<int> /*drawingAreaSize*/, int zoom)
  {
    return round_to_multiple_of(currentPosition, Point(50 / pixelSizeFromZoom(zoom)));
  }

  void setAspectRatio(Point aspectRatio_) { aspectRatio = aspectRatio_; }

private:
  explicit TweakPresentationPosition(Point aspectRatio_)
    : aspectRatio(aspectRatio_)
  {
  }

private:
  Point aspectRatio;
  //  const int                          tileSize = 4096;
};

class TweakRulers
{
public:
  using Ptr   = boost::shared_ptr<TweakRulers>;
  using Point = Scroom::Utils::Point<double>;

  static Ptr create(Point aspectRatio_) { return Ptr(new TweakRulers(aspectRatio_)); }

  Scroom::Utils::Rectangle<double> tweakRulers(Point currentPosition, Scroom::Utils::Point<int> drawingAreaSize, int zoom)
  {
    return Scroom::Utils::make_rect(currentPosition, drawingAreaSize.to<double>() / pixelSizeFromZoom(zoom)) / aspectRatio;
  }

  void setAspectRatio(Point aspectRatio_) { aspectRatio = aspectRatio_; }

private:
  explicit TweakRulers(Point aspectRatio_)
    : aspectRatio(aspectRatio_)
  {
  }

private:
  Point aspectRatio;
};

class TweakPositionTextBox
{
public:
  using Ptr   = boost::shared_ptr<TweakPositionTextBox>;
  using Point = Scroom::Utils::Point<double>;

  static Ptr create(Point aspectRatio_) { return Ptr(new TweakPositionTextBox(aspectRatio_)); }

  Point parse(std::string_view x, std::string_view y, Scroom::Utils::Point<int> drawingAreaSize, int zoom)
  {
    Point entered_position(boost::lexical_cast<double>(x), boost::lexical_cast<double>(y));

    return entered_position * aspectRatio - drawingAreaSize.to<double>() / pixelSizeFromZoom(zoom) / 2;
  }

  std::pair<std::string, std::string> display(Point position, Scroom::Utils::Point<int> drawingAreaSize, int zoom)
  {
    const Point center = (position + drawingAreaSize.to<double>() / pixelSizeFromZoom(zoom) / 2) / aspectRatio;

    return std::make_pair(fmt::format("{:.0f}", center.x), fmt::format("{:.0f}", center.y));
  }

  void setAspectRatio(Point aspectRatio_) { aspectRatio = aspectRatio_; }

private:
  explicit TweakPositionTextBox(Point aspectRatio_)
    : aspectRatio(aspectRatio_)
  {
  }

private:
  Point aspectRatio;
};

////////////////////////////////////////////////////////////////////////
/// Helpers

static Scroom::Utils::Point<double> eventToPoint(GdkEventButton* event) { return {event->x, event->y}; }

static Scroom::Utils::Point<double> eventToPoint(GdkEventMotion* event) { return {event->x, event->y}; }

// This one has too much View-internal knowledge to hide in callbacks.cc
static void on_newWindow_activate(GtkMenuItem*, gpointer user_data)
{
  PresentationInterface::WeakPtr& wp = *static_cast<PresentationInterface::WeakPtr*>(user_data); // Yuk!
  PresentationInterface::Ptr      p  = wp.lock();
  if(p)
  {
    find_or_create_scroom(p);
  }
}

////////////////////////////////////////////////////////////////////////

View::View(GtkBuilder* scroomXml_)
  : scroomXml(scroomXml_)
  , zoom(0)
  , aspectRatio(Scroom::Utils::make_point(1.0, 1.0))
  , tweakPresentationPosition(TweakPresentationPosition::create(aspectRatio))
  , tweakPositionTextBox(TweakPositionTextBox::create(aspectRatio))
  , tweakRulers(TweakRulers::create(aspectRatio))
  , modifiermove(0)
{
  PluginManager::Ptr pluginManager = PluginManager::getInstance();
  window                           = GTK_WINDOW(GTK_WIDGET(gtk_builder_get_object(scroomXml_, "scroom")));
  drawingArea                      = GTK_WIDGET(gtk_builder_get_object(scroomXml_, "drawingarea"));
  vscrollbar                       = GTK_SCROLLBAR(GTK_WIDGET(gtk_builder_get_object(scroomXml_, "vscrollbar")));
  hscrollbar                       = GTK_SCROLLBAR(GTK_WIDGET(gtk_builder_get_object(scroomXml_, "hscrollbar")));
  vscrollbaradjustment             = gtk_range_get_adjustment(GTK_RANGE(vscrollbar));
  hscrollbaradjustment             = gtk_range_get_adjustment(GTK_RANGE(hscrollbar));
  vruler_area                      = GTK_DRAWING_AREA(GTK_WIDGET(gtk_builder_get_object(scroomXml_, "vruler_area")));
  hruler_area                      = GTK_DRAWING_AREA(GTK_WIDGET(gtk_builder_get_object(scroomXml_, "hruler_area")));
  xTextBox                         = GTK_ENTRY(GTK_WIDGET(gtk_builder_get_object(scroomXml_, "x_textbox")));
  yTextBox                         = GTK_ENTRY(GTK_WIDGET(gtk_builder_get_object(scroomXml_, "y_textbox")));

  // Create rulers and attach ruler areas to them
  vruler = Ruler::create(Ruler::VERTICAL, GTK_WIDGET(vruler_area));
  hruler = Ruler::create(Ruler::HORIZONTAL, GTK_WIDGET(hruler_area));

  menubar     = GTK_WIDGET(gtk_builder_get_object(scroomXml_, "menubar"));
  statusArea  = GTK_WIDGET(gtk_builder_get_object(scroomXml_, "status_area"));
  toolbarArea = GTK_WIDGET(gtk_builder_get_object(scroomXml_, "toolbar_area"));

  zoomBox   = GTK_COMBO_BOX(GTK_WIDGET(gtk_builder_get_object(scroomXml_, "zoomboxcombo")));
  zoomItems = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_INT);

  gtk_combo_box_set_model(zoomBox, GTK_TREE_MODEL(zoomItems));
  GtkCellRenderer* txt = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
  gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(zoomBox), txt, true);
  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(zoomBox), txt, "text", COLUMN_TEXT, NULL);

  progressBar        = GTK_PROGRESS_BAR(GTK_WIDGET(gtk_builder_get_object(scroomXml_, "progressbar")));
  progressBarManager = ProgressBarManager::create(progressBar);
  statusBar          = GTK_STATUSBAR(GTK_WIDGET(gtk_builder_get_object(scroomXml_, "statusbar")));
  statusBarContextId = gtk_statusbar_get_context_id(statusBar, "View");

  GtkWidget* panelWindow = GTK_WIDGET(gtk_builder_get_object(scroomXml_, "panelWindow"));
  GtkBox*    panel       = GTK_BOX(GTK_WIDGET(gtk_builder_get_object(scroomXml_, "panel")));
  sidebarManager.setWidgets(panelWindow, panel);
  toolBar          = GTK_TOOLBAR(GTK_WIDGET(gtk_builder_get_object(scroomXml_, "toolbar")));
  toolBarSeparator = nullptr;
  toolBarCount     = 0;

  on_newPresentationInterfaces_update(pluginManager->getNewPresentationInterfaces());
  updateNewWindowMenu();
  on_configure();
}

View::Ptr View::create(GtkBuilder* scroomXml, PresentationInterface::Ptr presentation)
{
  Ptr view(new View(scroomXml));
  spdlog::debug("Creating a new view");

  if(presentation)
  {
    view->setPresentation(presentation);
  }

  return view;
}

View::~View()
{
  spdlog::debug("Destroying view...");
  gtk_widget_destroy(GTK_WIDGET(window));
}

void View::redraw(cairo_t* cr)
{
  if(presentation)
  {
    double                       pixelSize         = pixelSizeFromZoom(zoom);
    Scroom::Utils::Point<double> visibleRegionSize = drawingAreaSize.to<double>() / pixelSize;

    auto rect = Scroom::Utils::make_rect(tweakedPosition(), visibleRegionSize);

    presentation->redraw(shared_from_this<View>(), cr, rect, zoom);
    for(const auto& renderer: postRenderers)
    {
      renderer->render(shared_from_this<View>(), cr, rect, zoom);
    }
  }
  else
  {
    // A logo here would be nice...

    // char buffer[] = "View says \"Hi\"";
    //
    // cairo_move_to(cr, 50, 50);
    // cairo_show_text(cr, buffer);
  }
}

void View::hide() { gtk_widget_hide(GTK_WIDGET(window)); }

bool View::hasPresentation() { return presentation != nullptr; }

void View::clearPresentation()
{
  setPresentation(PresentationInterface::Ptr()); // null
}

void View::setPresentation(PresentationInterface::Ptr presentation_)
{
  View::Ptr me = shared_from_this<View>();

  if(presentation)
  {
    presentation->close(me);
    presentation.reset();
  }

  presentation = std::move(presentation_);

  if(presentation)
  {
    presentation->open(me);
    presentationRect = presentation->getRect();
    aspectRatio      = presentation->getAspectRatio();
    std::string s    = presentation->getTitle();

    tweakPresentationPosition->setAspectRatio(aspectRatio);
    tweakPositionTextBox->setAspectRatio(aspectRatio);
    tweakRulers->setAspectRatio(aspectRatio);

    if(s.length())
    {
      s = "Scroom - " + s;
    }
    else
    {
      s = "Scroom";
    }
    gtk_window_set_title(window, s.c_str());
  }

  zoom                   = 0;
  const double pixelSize = pixelSizeFromZoom(zoom);
  position               = -drawingAreaSize.to<double>() / pixelSize / 2;

  updateZoom();
  updateScrollbars();
  updateTextbox();
  invalidate();
}

void View::updateScrollbar(GtkAdjustment* adj,
                           int            zoom_,
                           double         value,
                           double         presentationStart,
                           double         presentationSize,
                           double         windowSize)
{
  const double pixelSize = pixelSizeFromZoom(zoom_);

  presentationStart -= windowSize / pixelSize / 2;
  presentationSize += windowSize / pixelSize;

  gtk_adjustment_configure(adj,
                           value,
                           presentationStart,
                           presentationStart + presentationSize,
                           std::max(1.0, 1 / pixelSize),
                           3 * windowSize / pixelSize / 4,
                           windowSize / pixelSize);
}

void View::updateScrollbars()
{
  if(presentation)
  {
    gtk_widget_set_sensitive(GTK_WIDGET(vscrollbar), true);
    gtk_widget_set_sensitive(GTK_WIDGET(hscrollbar), true);

    updateScrollbar(hscrollbaradjustment, zoom, position.x, presentationRect.x(), presentationRect.width(), drawingAreaSize.x);
    updateScrollbar(vscrollbaradjustment, zoom, position.y, presentationRect.y(), presentationRect.height(), drawingAreaSize.y);
    updateRulers();
  }
  else
  {
    gtk_widget_set_sensitive(GTK_WIDGET(vscrollbar), false);
    gtk_widget_set_sensitive(GTK_WIDGET(hscrollbar), false);
  }
}

void View::updateTextbox()
{
  if(presentation)
  {
    gtk_widget_set_sensitive(GTK_WIDGET(xTextBox), true);
    gtk_widget_set_sensitive(GTK_WIDGET(yTextBox), true);

    const auto [xs, ys] = tweakPositionTextBox->display(position, drawingAreaSize, zoom);

    gtk_entry_set_text(xTextBox, xs.c_str());
    gtk_entry_set_text(yTextBox, ys.c_str());
  }
  else
  {
    gtk_widget_set_sensitive(GTK_WIDGET(xTextBox), false);
    gtk_widget_set_sensitive(GTK_WIDGET(yTextBox), false);
  }
}

void View::updateZoom()
{
  if(presentation)
  {
    int presentationHeight = presentationRect.height();
    int presentationWidth  = presentationRect.width();
    int minZoom            = 0;

    while(presentationHeight > drawingAreaSize.y / 2 || presentationWidth > drawingAreaSize.x / 2)
    {
      presentationHeight >>= 1;
      presentationWidth >>= 1;
      minZoom--;
    }

    gtk_widget_set_sensitive(GTK_WIDGET(zoomBox), true);

    int zMax = MaxZoom - minZoom;
    zMax     = std::max(zMax, 1 + MaxZoom - zoom);
    zMax     = std::min<size_t>(zMax, sizeof(zoomfactor) / sizeof(zoomfactor[0]));

    gtk_list_store_clear(zoomItems);
    for(int z = 0; z < zMax; z++)
    {
      GtkTreeIter iter;
      gtk_list_store_insert_with_values(zoomItems, &iter, z, COLUMN_TEXT, zoomfactor[z], COLUMN_ZOOM, MaxZoom - z, -1);

      if(zoom == MaxZoom - z)
      {
        gtk_combo_box_set_active_iter(zoomBox, &iter);
      }
    }
  }
  else
  {
    gtk_widget_set_sensitive(GTK_WIDGET(zoomBox), false);
    gtk_list_store_clear(zoomItems);
    GtkTreeIter iter;
    gtk_list_store_insert_with_values(zoomItems, &iter, 0, COLUMN_TEXT, zoomfactor[0], COLUMN_ZOOM, MaxZoom, -1);
  }
}

void View::updateRulers()
{
  auto visible = tweakRulers->tweakRulers(tweakedPosition(), drawingAreaSize, zoom);

  const auto topLeft     = visible.getTopLeft();
  const auto bottomRight = visible.getBottomRight();

  hruler->setRange(topLeft.x, bottomRight.x);
  vruler->setRange(topLeft.y, bottomRight.y);
}

void View::toolButtonToggled(GtkToggleButton* button)
{
  if(gtk_toggle_button_get_active(button))
  {
    for(const auto& tool: tools)
    {
      if(tool.first != button && gtk_toggle_button_get_active(tool.first))
      {
        gtk_toggle_button_set_active(tool.first, false);
        gtk_widget_set_sensitive(GTK_WIDGET(tool.first), true);
        tools[tool.first]->onDisable();
      }
    }
    gtk_widget_set_sensitive(GTK_WIDGET(button), false);
    tools[button]->onEnable();
  }
}

////////////////////////////////////////////////////////////////////////
// Scroom events

void View::on_newPresentationInterfaces_update(
  const std::map<NewPresentationInterface::Ptr, std::string>& newPresentationInterfaces)
{
  GtkWidget* new_menu_item = GTK_WIDGET(gtk_builder_get_object(scroomXml, "new"));

  if(newPresentationInterfaces.empty())
  {
    gtk_widget_set_sensitive(new_menu_item, false);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(new_menu_item), nullptr);
  }
  else
  {
    gtk_widget_set_sensitive(new_menu_item, true);

    GtkWidget* new_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(new_menu_item), new_menu);

    for(auto cur = newPresentationInterfaces.begin(); cur != newPresentationInterfaces.end(); cur++)
    {
      GtkWidget* menu_item = gtk_menu_item_new_with_label(cur->second.c_str());
      gtk_widget_show(menu_item);
      gtk_container_add(GTK_CONTAINER(new_menu), menu_item);

      g_signal_connect(static_cast<gpointer>(menu_item), "activate", G_CALLBACK(on_new_activate), cur->first.get());
    }
  }
}

void View::on_presentation_created(PresentationInterface::Ptr p)
{
  presentations[p] = nullptr;
  updateNewWindowMenu();
}

void View::on_presentation_destroyed() { updateNewWindowMenu(); }

void View::on_configure()
{
  // There should be a simpler way to do this...
  cairo_region_t*       r = gdk_window_get_visible_region(gtk_widget_get_window(drawingArea));
  cairo_rectangle_int_t rect;
  cairo_region_get_extents(r, &rect);

  Scroom::Utils::Point<int> newSize(rect.width, rect.height);

  if(drawingAreaSize != newSize)
  {
    on_window_size_changed(newSize);
  }

  cairo_region_destroy(r);
}

void View::on_window_size_changed(const Scroom::Utils::Point<int>& newSize)
{
  auto pixelSize = pixelSizeFromZoom(zoom);
  position += (drawingAreaSize - newSize) / pixelSize / 2;

  drawingAreaSize = newSize;
  updateZoom();
  updateScrollbars();
  invalidate();
}

void View::on_scrollwheel(GdkEventScroll* event)
{
  if(event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_DOWN)
  {
    int newZoom = zoom + ((event->direction == GDK_SCROLL_UP) ? 1 : -1);
    newZoom     = std::min(MaxZoom, newZoom);

    GtkTreeIter iter;
    for(bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(zoomItems), &iter); valid;
        valid      = gtk_tree_model_iter_next(GTK_TREE_MODEL(zoomItems), &iter))
    {
      GValue value = G_VALUE_INIT;
      gtk_tree_model_get_value(GTK_TREE_MODEL(zoomItems), &iter, COLUMN_ZOOM, &value);
      int foundZoom = g_value_get_int(&value);

      if(foundZoom == newZoom)
      {
        on_zoombox_changed(newZoom, {event->x, event->y});
        gtk_combo_box_set_active_iter(zoomBox, &iter);
        break;
      }
    }
  }
}

void View::on_zoombox_changed()
{
  GtkTreeIter iter;
  GValue      value = G_VALUE_INIT;
  gtk_combo_box_get_active_iter(zoomBox, &iter);

  if(gtk_list_store_iter_is_valid(zoomItems, &iter))
  {
    gtk_tree_model_get_value(GTK_TREE_MODEL(zoomItems), &iter, COLUMN_ZOOM, &value);
    int newZoom = g_value_get_int(&value);
    on_zoombox_changed(newZoom, drawingAreaSize.to<double>() / 2);
  }
}

void View::on_zoombox_changed(int newzoom, const Scroom::Utils::Point<double>& mousePos)
{
  if(newzoom != zoom)
  {
    position += mousePos / pixelSizeFromZoom(zoom);
    position -= mousePos / pixelSizeFromZoom(newzoom);

    zoom = newzoom;
    updateScrollbars();
    updateTextbox();
    invalidate();
  }
}

void View::on_textbox_value_changed(GtkEditable* /*editable*/)
{
  try
  {
    auto newPos = tweakPositionTextBox->parse(gtk_entry_get_text(xTextBox), gtk_entry_get_text(yTextBox), drawingAreaSize, zoom);

    updateXY(newPos, TEXTBOX);
  }
  catch(boost::bad_lexical_cast& ex)
  {
    // User typed something invalid, probably a letter. Ignore...
  }
}

void View::on_scrollbar_value_changed(GtkAdjustment* adjustment)
{
  auto newPos = position;

  if(adjustment == vscrollbaradjustment)
  {
    newPos.y = gtk_adjustment_get_value(adjustment);
  }
  else
  {
    newPos.x = gtk_adjustment_get_value(adjustment);
  }

  updateXY(newPos, SCROLLBAR);
}

void View::on_buttonPress(GdkEventButton* event)
{
  if(event->button == 1 && modifiermove == 0)
  {
    // Begin left-dragging
    modifiermove = GDK_BUTTON1_MASK;
    cachedPoint  = eventToPoint(event);
  }
  else if(event->button == 3)
  {
    auto point = windowPointToPresentationPoint(eventToPoint(event));
    selection  = boost::make_shared<Selection>(point);
    for(const auto& listener: selectionListeners)
    {
      listener->onSelectionStart(selection, shared_from_this<ViewInterface>());
    }
  }
}

void View::on_buttonRelease(GdkEventButton* event)
{
  if(event->button == 1 && modifiermove == GDK_BUTTON1_MASK)
  {
    // End left-dragging
    modifiermove  = 0;
    cachedPoint.x = 0;
    cachedPoint.y = 0;
  }
  else if(event->button == 3 && selection)
  {
    selection->end = windowPointToPresentationPoint(eventToPoint(event));
    for(const auto& listener: selectionListeners)
    {
      listener->onSelectionEnd(selection, shared_from_this<ViewInterface>());
    }
    invalidate();
  }
}

void View::on_motion_notify(GdkEventMotion* event)
{
  if((event->state & GDK_BUTTON1_MASK) && modifiermove == GDK_BUTTON1_MASK)
  {
    auto mousePos = eventToPoint(event);

    const auto pixelSize = pixelSizeFromZoom(zoom);
    const auto newPos    = position - (mousePos - cachedPoint) / pixelSize;
    cachedPoint          = mousePos;

    updateXY(newPos, OTHER);
  }
  else if((event->state & GDK_BUTTON3_MASK) && selection)
  {
    selection->end = windowPointToPresentationPoint(eventToPoint(event));
    for(const auto& listener: selectionListeners)
    {
      listener->onSelectionUpdate(selection, shared_from_this<ViewInterface>());
    }
    invalidate();
  }
}

void View::setFullScreen()
{
  gtk_window_fullscreen(window);
  gtk_widget_set_visible(toolbarArea, false);
  gtk_widget_set_visible(statusArea, false);
}

void View::unsetFullScreen()
{
  gtk_window_unfullscreen(window);
  gtk_widget_set_visible(toolbarArea, true);
  gtk_widget_set_visible(statusArea, true);
}

////////////////////////////////////////////////////////////////////////
// ViewInterface

void View::invalidate() { gdk_window_invalidate_rect(gtk_widget_get_window(drawingArea), nullptr, false); }

ProgressInterface::Ptr View::getProgressInterface() { return progressBarManager; }

void View::addSideWidget(std::string title, GtkWidget* w) { sidebarManager.addSideWidget(title, w); }

void View::removeSideWidget(GtkWidget* w) { sidebarManager.removeSideWidget(w); }

void View::addToToolbar(GtkToolItem* ti)
{
  if(toolBarCount == 0)
  {
    toolBarSeparator = gtk_separator_tool_item_new();
    g_object_set(G_OBJECT(toolBarSeparator), "visible", true, "draw", true, NULL);
    gtk_toolbar_insert(toolBar, toolBarSeparator, -1);
  }

  g_object_set(G_OBJECT(ti), "visible", true, NULL);

  gtk_toolbar_insert(toolBar, ti, -1);
  toolBarCount++;
}

void View::removeFromToolbar(GtkToolItem* ti)
{
  gtk_container_remove(GTK_CONTAINER(toolBar), GTK_WIDGET(ti));
  toolBarCount--;

  if(toolBarCount == 0)
  {
    gtk_container_remove(GTK_CONTAINER(toolBar), GTK_WIDGET(toolBarSeparator));
    toolBarSeparator = nullptr;
  }
}

void View::registerSelectionListener(SelectionListener::Ptr listener) { selectionListeners.push_back(listener); }

void View::registerPostRenderer(PostRenderer::Ptr renderer) { postRenderers.push_back(renderer); }

void View::setStatusMessage(const std::string& message)
{
  gtk_statusbar_pop(statusBar, statusBarContextId);
  gtk_statusbar_push(statusBar, statusBarContextId, message.c_str());
}

PresentationInterface::Ptr View::getCurrentPresentation() { return presentation; }

static void tool_button_toggled(GtkToggleButton* button, gpointer data) { static_cast<View*>(data)->toolButtonToggled(button); }

void View::addToolButton(GtkToggleButton* button, ToolStateListener::Ptr callback)
{
  Scroom::GtkHelpers::sync_on_ui_thread(
    [=]
    {
      GtkToolItem* toolItem = gtk_tool_item_new();
      gtk_container_add(GTK_CONTAINER(toolItem), GTK_WIDGET(button));
      gtk_widget_set_visible(GTK_WIDGET(button), true);
      g_signal_connect(static_cast<gpointer>(button), "toggled", G_CALLBACK(tool_button_toggled), this);

      addToToolbar(toolItem);

      tools[button] = callback;
      if(tools.size() == 1)
      {
        gtk_toggle_button_set_active(button, true);
        gtk_widget_set_sensitive(GTK_WIDGET(button), false);
        callback->onEnable();
      }
      else
      {
        gtk_toggle_button_set_active(button, false);
        gtk_widget_set_sensitive(GTK_WIDGET(button), true);
      }
    });
}

////////////////////////////////////////////////////////////////////////
// Helpers

Scroom::Utils::Point<double> View::windowPointToPresentationPoint(Scroom::Utils::Point<double> wp) const
{
  return tweakedPosition() + wp / pixelSizeFromZoom(zoom);
}

Scroom::Utils::Point<double> View::presentationPointToWindowPoint(Scroom::Utils::Point<double> presentationpoint) const
{
  return (presentationpoint - tweakedPosition()) * pixelSizeFromZoom(zoom);
}

void View::updateNewWindowMenu()
{
  GtkWidget* newWindow_menu_item = GTK_WIDGET(gtk_builder_get_object(scroomXml, "newWindow"));

  GtkWidget* newWindow_menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(newWindow_menu_item));
  if(!newWindow_menu)
  {
    newWindow_menu = gtk_menu_new();
  }
  g_object_ref_sink(G_OBJECT(newWindow_menu));

  auto cur = presentations.begin();
  auto end = presentations.end();

  while(cur != end)
  {
    auto next = cur;
    next++;

    //// Update menu
    PresentationInterface::Ptr p = cur->first.lock();
    GtkWidget*                 m = cur->second;
    if(p && m)
    {
      // Do nothing
    }
    else if(p && !m)
    {
      // Add a menu item
      std::string s = p->getTitle();
      if(!s.length())
      {
        s = "Default";
      }
      m = gtk_menu_item_new_with_label(s.c_str());
      gtk_widget_show(m);
      cur->second = m;
      gtk_container_add(GTK_CONTAINER(newWindow_menu), m);

      g_signal_connect(static_cast<gpointer>(m),
                       "activate",
                       G_CALLBACK(on_newWindow_activate),
                       const_cast<PresentationInterface::WeakPtr*>(&cur->first));
    }
    else if(!p && m)
    {
      // Remove menu item, then remove this element from the map
      cur->second = NULL;
      gtk_widget_destroy(m);
      presentations.erase(cur);
    }
    else if(!p && !m)
    {
      // Remove this element from the map (menu already gone)
      presentations.erase(cur);
    }
    else
    {
      // This cannot happen
      defect();
    }
    //// Done updating menu

    cur = next;
  }

  if(presentations.empty())
  {
    gtk_widget_set_sensitive(newWindow_menu_item, false);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(newWindow_menu_item), nullptr);
  }
  else
  {
    gtk_widget_set_sensitive(newWindow_menu_item, true);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(newWindow_menu_item), newWindow_menu);
  }

  g_object_unref(G_OBJECT(newWindow_menu));
}

void View::updateXY(const Scroom::Utils::Point<double>& newPos, const View::LocationChangeCause& source)
{
  if(position != newPos)
  {
    position = newPos;

    if(source != SCROLLBAR)
    {
      updateScrollbars();
    }
    else
    {
      updateRulers();
    }

    if(source != TEXTBOX)
    {
      updateTextbox();
    }

    invalidate();
  }
}

Scroom::Utils::Point<double> View::tweakedPosition() const
{
  return tweakPresentationPosition->tweakPosition(position, drawingAreaSize, zoom);
}
