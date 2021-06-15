/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "callbacks.hh"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_VERSION_H
#  include <version.h>
#endif

#include <cstdlib>
#include <list>
#include <map>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/scoped_array.hpp>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <cairo.h>

#include <scroom/assertions.hh>
#include <scroom/bookkeeping.hh>

#include "loader.hh"
#include "pluginmanager.hh"
#include "view.hh"
#include "workinterface.hh"

#ifdef _WIN32
#  include <windows.h>
#  include <shellapi.h>

#  include <boost/dll.hpp>
#endif

static const std::string SCROOM_DEV_MODE = "SCROOM_DEV_MODE";
const std::string        REGULAR_FILES   = "Regular files";

static std::string xmlFileName;
static GtkBuilder* aboutDialogXml = nullptr;
GError*            error          = NULL;
static GtkWidget*  aboutDialog    = nullptr;

using Views = std::map<View::Ptr, Scroom::Bookkeeping::Token>;
static Views                                     views;
static std::list<PresentationInterface::WeakPtr> presentations;
static FileNameMap                               filenames;
static std::string                               currentFolder;

void ShowModalDialog(const std::string& message)
{
  printf("%s\n", message.c_str());
  if(gdk_display_get_default())
  {
    // We're not running headless, don't open the popup
    // We don't have a pointer to the parent window, so nullptr should
    // suffice
    GtkWidget* dialog = gtk_message_dialog_new(
      nullptr, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, "%s", message.c_str());

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
}

void on_scroom_hide(GtkWidget*, gpointer user_data)
{
  View* view = static_cast<View*>(user_data);
  on_view_destroyed(view);

  if(views.empty())
  {
    gtk_main_quit();
  }
}

void on_new_activate(GtkMenuItem*, gpointer user_data)
{
  auto* newPresentationInterface = static_cast<NewPresentationInterface*>(user_data);
  create(newPresentationInterface);
}

gboolean combinedFileFilter(const GtkFileFilterInfo* filter_info, gpointer data)
{
  // Convert the data back to a filter vector
  auto* filters = static_cast<std::vector<GtkFileFilter*>*>(data);

  // Return true if any of the filters matches the filter_info
  for(const auto& f: *filters)
  {
    if(gtk_file_filter_filter(f, filter_info))
    {
      return true;
    }
  }

  // None of the filters matched
  return false;
}

void on_open_activate(GtkMenuItem*, gpointer user_data)
{
  GtkWidget* dialog;
  auto*      scroom = static_cast<GtkWidget*>(user_data);

  printf("Creating the open dialog\n");
  dialog = gtk_file_chooser_dialog_new("Open File",
                                       GTK_WINDOW(scroom),
                                       GTK_FILE_CHOOSER_ACTION_OPEN,
                                       ("_Cancel"),
                                       GTK_RESPONSE_CANCEL,
                                       ("_Open"),
                                       GTK_RESPONSE_ACCEPT,
                                       NULL);
  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), currentFolder.c_str());

  const auto  pm                         = PluginManager::getInstance();
  const auto& openInterfaces             = pm->getOpenInterfaces();
  const auto& openPresentationInterfaces = pm->getOpenPresentationInterfaces();

  // Store all the file filters so that we can create a custom file filter that allows any supported type (by default)
  std::vector<GtkFileFilter*> filters;
  GtkFileFilter*              allSupportedFileTypesFilter = gtk_file_filter_new();
  gtk_file_filter_set_name(allSupportedFileTypesFilter, "Any supported file type");

  // Cannot beforehand determine which data might be needed for the plugins, so we ask GTK to load everything!
  auto filterFlags = static_cast<GtkFileFilterFlags>(GTK_FILE_FILTER_FILENAME | GTK_FILE_FILTER_MIME_TYPE
                                                     | GTK_FILE_FILTER_DISPLAY_NAME | GTK_FILE_FILTER_URI);

  gtk_file_filter_add_custom(allSupportedFileTypesFilter, filterFlags, &combinedFileFilter, &filters, nullptr);
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), allSupportedFileTypesFilter);

  for(auto const& cur: openPresentationInterfaces)
  {
    for(auto const& f: cur.first->getFilters())
    {
      gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f);
      filters.push_back(f);
    }
  }
  for(auto const& cur: openInterfaces)
  {
    for(auto const& f: cur.first->getFilters())
    {
      gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f);
      filters.push_back(f);
    }
  }

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
  {
    GFile*            file     = g_file_new_for_path(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
    GFileInfo*        fileInfo = g_file_query_info(file, "standard::*", G_FILE_QUERY_INFO_NONE, nullptr, nullptr);
    GtkFileFilterInfo filterInfo;
    filterInfo.filename     = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    filterInfo.mime_type    = g_content_type_get_mime_type(g_file_info_get_content_type(fileInfo));
    filterInfo.display_name = g_file_info_get_display_name(fileInfo);
    filterInfo.contains =
      static_cast<GtkFileFilterFlags>(GTK_FILE_FILTER_FILENAME | GTK_FILE_FILTER_DISPLAY_NAME | GTK_FILE_FILTER_MIME_TYPE);
    printf("Opening file %s\n", filterInfo.filename);

    try
    {
      load(filterInfo);
    }
    catch(std::exception& ex)
    {
      printf("ERROR: %s\n", ex.what());
      on_presentation_possibly_destroyed();
    }
  }
  gchar* cf = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
  if(cf)
  {
    currentFolder = cf;
    g_free(cf);
  }
  gtk_widget_destroy(dialog);
}

void on_save_activate(GtkMenuItem*, gpointer) {}

void on_save_as_activate(GtkMenuItem*, gpointer) {}

void on_quit_activate(GtkMenuItem*, gpointer)
{
  Views v(views);
  for(const Views::value_type& p: v)
  {
    p.first->hide();
  }
  gtk_main_quit();
}

void on_cut_activate(GtkMenuItem*, gpointer) {}

void on_copy_activate(GtkMenuItem*, gpointer) {}

void on_paste_activate(GtkMenuItem*, gpointer) {}

void on_delete_activate(GtkMenuItem*, gpointer) {}

void on_fullscreen_activate(GtkMenuItem* item, gpointer user_data)
{
  View*             view   = static_cast<View*>(user_data);
  GtkCheckMenuItem* cmi    = GTK_CHECK_MENU_ITEM(item);
  gboolean          active = gtk_check_menu_item_get_active(cmi);

  if(active)
  {
    view->setFullScreen();
  }
  else
  {
    view->unsetFullScreen();
  }
}

void on_close_activate(GtkMenuItem*, gpointer user_data)
{
  View* view = static_cast<View*>(user_data);

  view->hide();
}

void on_about_activate(GtkMenuItem*, gpointer)
{
  // GtkWidget* aboutdialog;
  // aboutdialog = create_aboutdialog ();
  gtk_dialog_run(GTK_DIALOG(aboutDialog));
  gtk_widget_hide(aboutDialog);
  // gtk_widget_destroy (aboutdialog);
}

gboolean on_drawingarea_expose_event(GtkWidget* widget, GdkEventExpose*, gpointer user_data)
{
  // printf("expose\n");

  cairo_t* cr   = gdk_cairo_create(gtk_widget_get_window(widget));
  View*    view = static_cast<View*>(user_data);
  view->redraw(cr);

  cairo_destroy(cr);
  return FALSE;
}

gboolean on_drawingarea_configure_event(GtkWidget*, GdkEventConfigure*, gpointer user_data)
{
  // printf("configure\n");
  View* view = static_cast<View*>(user_data);
  view->on_configure();
  return FALSE;
}

gboolean on_idle(gpointer user_data)
{
  if(user_data == nullptr)
  {
    return 0;
  }
  else
  {
    return reinterpret_cast<WorkInterface*>(user_data)->doWork();
  }
}

void on_done_loading_plugins()
{
  if(!filenames.empty())
  {
    {
      std::list<std::string>& fn = filenames[REGULAR_FILES];

      while(!fn.empty())
      {
        std::string& file = fn.front();
        try
        {
          load(file);
        }
        catch(std::exception& ex)
        {
          printf("ERROR: %s\n", ex.what());
          on_presentation_possibly_destroyed();
        }
        gchar* dir    = g_path_get_dirname(file.c_str());
        currentFolder = dir;
        g_free(dir);
        fn.pop_front();
      }
    }
    filenames.erase(REGULAR_FILES);

    PluginManager::Ptr                                       instance               = PluginManager::getInstance();
    std::map<std::string, NewAggregateInterface::Ptr> const& newAggregateInterfaces = instance->getNewAggregateInterfaces();

    for(FileNameMap::value_type const& v: filenames)
    {
      std::string const&            aggregateName = v.first;
      std::list<std::string> const& files         = v.second;

      auto i = newAggregateInterfaces.find(aggregateName);
      if(i != newAggregateInterfaces.end())
      {
        try
        {
          Aggregate::Ptr             aggregate             = i->second->createNew();
          PresentationInterface::Ptr aggregatePresentation = boost::dynamic_pointer_cast<PresentationInterface>(aggregate);

          if(aggregatePresentation)
          {
            for(std::string const& file: files)
            {
              PresentationInterface::Ptr p = loadPresentation(file);
              aggregate->addPresentation(p);
            }

            on_presentation_created(aggregatePresentation);
            find_or_create_scroom(aggregatePresentation);
          }
          else
          {
            printf("ERROR: Don't know how to display a %s\n", aggregateName.c_str());
          }
        }
        catch(std::exception& ex)
        {
          printf("ERROR: While creating %s: %s\n", aggregateName.c_str(), ex.what());
          on_presentation_possibly_destroyed();
        }
      }
      else
      {
        printf("ERROR: Don't know how to create %s\n", aggregateName.c_str());
      }
    }

    if(presentations.empty())
    {
      // Aparently, we couldn't load any of our presentations. Terminate...
      ensure(views.empty());
      gtk_main_quit();
    }
  }
}

void on_zoombox_changed(GtkComboBox*, gpointer user_data)
{
  View* view = static_cast<View*>(user_data);
  view->on_zoombox_changed();
}

void on_textbox_value_changed(GtkEditable* editable, gpointer user_data)
{
  View* view = static_cast<View*>(user_data);
  view->on_textbox_value_changed(editable);
}

void on_scrollbar_value_changed(GtkAdjustment* adjustment, gpointer user_data)
{
  View* view = static_cast<View*>(user_data);
  view->on_scrollbar_value_changed(adjustment);
}

gboolean on_button_press_event(GtkWidget*, GdkEventButton* event, gpointer user_data)
{
  View* view = static_cast<View*>(user_data);
  view->on_buttonPress(event);
  return true;
}

gboolean on_button_release_event(GtkWidget*, GdkEventButton* event, gpointer user_data)
{
  View* view = static_cast<View*>(user_data);
  view->on_buttonRelease(event);
  return true;
}

gboolean on_motion_notify_event(GtkWidget*, GdkEventMotion* event, gpointer user_data)
{
  View* view = static_cast<View*>(user_data);
  view->on_motion_notify(event);
  return true;
}

gboolean on_scroll_event(GtkWidget*, GdkEventScroll* event, gpointer user_data)
{
  View* view = static_cast<View*>(user_data);
  view->on_scrollwheel(event);
  return true;
}

#ifdef _WIN32
gboolean on_open_scroom_website(GtkAboutDialog*, gchar* uri, gpointer)
{
  ShellExecute(nullptr, nullptr, uri, nullptr, nullptr, SW_SHOW);
  return true;
}
#endif

void on_scroom_bootstrap(const FileNameMap& newFilenames)
{
  printf("Bootstrapping Scroom...\n");
  filenames     = newFilenames;
  currentFolder = ".";

  bool devMode = nullptr != getenv(SCROOM_DEV_MODE.c_str());
  if(devMode)
  {
    printf(
      "+----------------------------------------------------------------------+\n"
      "| ENTERING DEVELOPMENT MODE                                            |\n"
      "| All the default directories are not searched                         |\n"
      "| Instead, only environment variables and the local source tree        |\n"
      "| are consulted.                                                       |\n"
      "+----------------------------------------------------------------------+\n");
  }

  startPluginManager(devMode);

  if(devMode)
  {
    xmlFileName = TOP_SRCDIR "/gui/scroom.glade";
  }
  else
  {
#ifdef _WIN32
    // We want to keep everything portable on windows so we look for the .glade file in the same directory as the .exe
    xmlFileName = (boost::dll::program_location().parent_path() / "scroom.glade").generic_string();
#else
    xmlFileName = PACKAGE_DATA_DIR "/scroom.glade";
#endif
  }


  aboutDialogXml = gtk_builder_new();
  boost::scoped_array<gchar*> obj{new gchar*[2]};
  obj[0] = "aboutDialog";
  obj[1] = nullptr;
  gtk_builder_add_objects_from_file(aboutDialogXml, xmlFileName.c_str(), obj.get(), NULL);


  if(aboutDialogXml != nullptr)
  {
    aboutDialog = GTK_WIDGET(gtk_builder_get_object(aboutDialogXml, "aboutDialog"));
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutDialog), "Scroom");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutDialog), PACKAGE_VERSION);
#ifdef _WIN32
    g_signal_connect(G_OBJECT(aboutDialog), "activate-link", G_CALLBACK(on_open_scroom_website), NULL);
#endif
  }
  else
  {
    printf("Opening xml failed\n");
    exit(-1);
  }

  if(filenames.empty())
  {
    create_scroom(PresentationInterface::Ptr());
  }
}

void on_scroom_terminating() { ensure(views.empty()); }

void find_or_create_scroom(PresentationInterface::Ptr presentation)
{
  for(const Views::value_type& p: views)
  {
    View::Ptr view = p.first;
    if(!view->hasPresentation())
    {
      view->setPresentation(presentation);
      return;
    }
  }
  create_scroom(presentation);
}

void onDragDataReceived(GtkWidget*, GdkDragContext*, int, int, GtkSelectionData* seldata, guint, guint, gpointer)
{
  printf("Dropping file(s) onto Scroom:\n");
  gchar** uris = g_uri_list_extract_uris(reinterpret_cast<const gchar*>(gtk_selection_data_get_data(seldata)));
  for(gchar** uri = uris; *uri != nullptr; uri++)
  {
    printf("\t%s\n", *uri);

    GError* error    = nullptr;
    gchar*  filename = g_filename_from_uri(*uri, nullptr, &error);
    if(error != nullptr)
    {
      ShowModalDialog(error->message);
      g_error_free(error);
    }
    else
    {
      try
      {
        load(filename);
      }
      catch(std::invalid_argument& ex)
      {
        boost::format warning = boost::format("Warning: unable to load file %s") % filename;
        ShowModalDialog(warning.str());
      }
    }

    g_free(filename);
  }

  g_strfreev(uris);
}

void create_scroom(PresentationInterface::Ptr presentation)
{
  GtkBuilder*                xml = gtk_builder_new();
  boost::scoped_array<char*> obj{new gchar*[2]};
  obj[0] = "scroom";
  obj[1] = nullptr;
  gtk_builder_add_objects_from_file(xml, xmlFileName.c_str(), obj.get(), NULL);

  if(xml == nullptr)
  {
    printf("Opening xml failed\n");
    exit(-1);
  }

  View::Ptr view = View::create(xml, presentation);
  on_view_created(view);

  GtkWidget* scroom = GTK_WIDGET(gtk_builder_get_object(xml, "scroom"));
  // GtkWidget* newMenuItem = glade_xml_get_widget(xml, "new");
  GtkWidget*     openMenuItem         = GTK_WIDGET(gtk_builder_get_object(xml, "open"));
  GtkWidget*     closeMenuItem        = GTK_WIDGET(gtk_builder_get_object(xml, "close"));
  GtkWidget*     quitMenuItem         = GTK_WIDGET(gtk_builder_get_object(xml, "quit"));
  GtkWidget*     fullScreenMenuItem   = GTK_WIDGET(gtk_builder_get_object(xml, "fullscreen_menu_item"));
  GtkWidget*     aboutMenuItem        = GTK_WIDGET(gtk_builder_get_object(xml, "about"));
  GtkWidget*     drawingArea          = GTK_WIDGET(gtk_builder_get_object(xml, "drawingarea"));
  GtkWidget*     zoomBox              = GTK_WIDGET(gtk_builder_get_object(xml, "zoomboxcombo"));
  GtkWidget*     vscrollbar           = GTK_WIDGET(gtk_builder_get_object(xml, "vscrollbar"));
  GtkWidget*     hscrollbar           = GTK_WIDGET(gtk_builder_get_object(xml, "hscrollbar"));
  GtkAdjustment* vscrollbaradjustment = gtk_range_get_adjustment(GTK_RANGE(vscrollbar));
  GtkAdjustment* hscrollbaradjustment = gtk_range_get_adjustment(GTK_RANGE(hscrollbar));
  GtkEditable*   xTextBox             = GTK_EDITABLE(GTK_WIDGET(gtk_builder_get_object(xml, "x_textbox")));
  GtkEditable*   yTextBox             = GTK_EDITABLE(GTK_WIDGET(gtk_builder_get_object(xml, "y_textbox")));

  g_signal_connect(static_cast<gpointer>(scroom), "hide", G_CALLBACK(on_scroom_hide), view.get());
  g_signal_connect(static_cast<gpointer>(closeMenuItem), "activate", G_CALLBACK(on_close_activate), view.get());
  g_signal_connect(static_cast<gpointer>(quitMenuItem), "activate", G_CALLBACK(on_quit_activate), view.get());
  g_signal_connect(static_cast<gpointer>(openMenuItem), "activate", G_CALLBACK(on_open_activate), scroom);
  g_signal_connect(static_cast<gpointer>(fullScreenMenuItem), "activate", G_CALLBACK(on_fullscreen_activate), view.get());
  g_signal_connect(static_cast<gpointer>(zoomBox), "changed", G_CALLBACK(on_zoombox_changed), view.get());
  g_signal_connect(
    static_cast<gpointer>(vscrollbaradjustment), "value-changed", G_CALLBACK(on_scrollbar_value_changed), view.get());
  g_signal_connect(
    static_cast<gpointer>(hscrollbaradjustment), "value-changed", G_CALLBACK(on_scrollbar_value_changed), view.get());
  g_signal_connect(static_cast<gpointer>(xTextBox), "changed", G_CALLBACK(on_textbox_value_changed), view.get());
  g_signal_connect(static_cast<gpointer>(yTextBox), "changed", G_CALLBACK(on_textbox_value_changed), view.get());
  // g_signal_connect ((gpointer) cut, "activate",
  //                   G_CALLBACK (on_cut_activate),
  //                   view.get());
  // g_signal_connect ((gpointer) copy, "activate",
  //                   G_CALLBACK (on_copy_activate),
  //                   view.get());
  // g_signal_connect ((gpointer) paste, "activate",
  //                   G_CALLBACK (on_paste_activate),
  //                   view.get());
  // g_signal_connect ((gpointer) delete, "activate",
  //                   G_CALLBACK (on_delete_activate),
  //                   view.get());
  g_signal_connect(static_cast<gpointer>(aboutMenuItem), "activate", G_CALLBACK(on_about_activate), view.get());
  g_signal_connect(static_cast<gpointer>(drawingArea), "draw", G_CALLBACK(on_drawingarea_expose_event), view.get());
  g_signal_connect(static_cast<gpointer>(drawingArea), "configure_event", G_CALLBACK(on_drawingarea_configure_event), view.get());
  g_signal_connect(static_cast<gpointer>(drawingArea), "button-press-event", G_CALLBACK(on_button_press_event), view.get());
  g_signal_connect(static_cast<gpointer>(drawingArea), "button-release-event", G_CALLBACK(on_button_release_event), view.get());
  g_signal_connect(static_cast<gpointer>(drawingArea), "scroll-event", G_CALLBACK(on_scroll_event), view.get());
  g_signal_connect(static_cast<gpointer>(drawingArea), "motion-notify-event", G_CALLBACK(on_motion_notify_event), view.get());

  char           uriList[] = "text/uri-list";
  GtkTargetEntry targets[] = {{uriList, 0, 0}};
  gtk_drag_dest_set(
    scroom, GTK_DEST_DEFAULT_ALL, targets, 1, static_cast<GdkDragAction>(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK));

  g_signal_connect(static_cast<gpointer>(scroom), "drag_data_received", G_CALLBACK(onDragDataReceived), NULL);
  // delete xml; //Breaks code for some reason. It seems that this xml is freed somewhere else...
}

void on_newPresentationInterfaces_update(const std::map<NewPresentationInterface::Ptr, std::string>& newPresentationInterfaces)
{
  for(const Views::value_type& p: views)
  {
    p.first->on_newPresentationInterfaces_update(newPresentationInterfaces);
  }
}

void on_presentation_created(PresentationInterface::Ptr presentation)
{
  presentations.emplace_back(presentation);

  for(const Views::value_type& p: views)
  {
    p.first->on_presentation_created(presentation);
  }

  const std::map<PresentationObserver::Ptr, std::string>& presentationObservers =
    PluginManager::getInstance()->getPresentationObservers();

  auto cur = presentationObservers.begin();
  auto end = presentationObservers.end();
  for(; cur != end; cur++)
  {
    cur->first->presentationAdded(presentation);
  }
}

void on_view_created(View::Ptr v)
{
  Scroom::Bookkeeping::Token t;
  views[v] = t;

  for(auto& presentation: presentations)
  {
    PresentationInterface::Ptr p = presentation.lock();
    if(p)
    {
      v->on_presentation_created(p);
    }
  }

  const std::map<ViewObserver::Ptr, std::string>& viewObservers = PluginManager::getInstance()->getViewObservers();

  auto cur = viewObservers.begin();
  auto end = viewObservers.end();
  for(; cur != end; cur++)
  {
    t.add(cur->first->viewAdded(v));
  }
}

void on_presentation_possibly_destroyed()
{
  bool presentationDestroyed = false;

  for(auto cur = presentations.begin(); cur != presentations.end(); cur++)
  {
    PresentationInterface::Ptr p = cur->lock();
    if(!p)
    {
      presentationDestroyed = true;
      auto temp             = cur;
      temp--;
      presentations.erase(cur);
      cur = temp;
    }
  }
  if(presentationDestroyed)
  {
    for(auto const& p: views)
    {
      p.first->on_presentation_destroyed();
    }

    const std::map<PresentationObserver::Ptr, std::string>& presentationObservers =
      PluginManager::getInstance()->getPresentationObservers();

    for(auto const& p: presentationObservers)
    {
      p.first->presentationDeleted();
    }
  }
}

void on_view_destroyed(View* v)
{
  View::Ptr view = v->shared_from_this<View>();
  view->clearPresentation();

  views.erase(view);
  view.reset();

  on_presentation_possibly_destroyed();
}

void on_new_presentationobserver(PresentationObserver::Ptr po)
{
  for(auto& presentation: presentations)
  {
    PresentationInterface::Ptr p = presentation.lock();
    if(p)
    {
      po->presentationAdded(p);
    }
  }
}

void on_new_viewobserver(ViewObserver::Ptr v)
{
  for(Views::value_type& p: views)
  {
    p.second.add(v->viewAdded(p.first));
  }
}
