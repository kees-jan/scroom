/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "callbacks.hh"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glade/glade.h>

#include <string>
#include <list>
#include <map>

#include <scroom/bookkeeping.hh>

#include "workinterface.hh"

#include "view.hh"
#include "pluginmanager.hh"
#include "loader.hh"

#ifdef _WIN32
  #include <boost/dll.hpp>
  #include <windows.h>
  #include <shellapi.h>
#endif

static const std::string SCROOM_DEV_MODE="SCROOM_DEV_MODE";
const std::string REGULAR_FILES="Regular files";

static std::string xmlFileName;
static GladeXML* aboutDialogXml=NULL;
static GtkWidget* aboutDialog=NULL;

typedef std::map<View::Ptr, Scroom::Bookkeeping::Token> Views;
static Views views;
static std::list<PresentationInterface::WeakPtr> presentations;
static FileNameMap filenames;
static std::string currentFolder;

void on_scroom_hide (GtkWidget*, gpointer user_data)
{
  View* view = static_cast<View*>(user_data);
  on_view_destroyed(view);

  if(views.empty())
    gtk_main_quit();
}

void on_new_activate (GtkMenuItem*, gpointer user_data)
{
  NewPresentationInterface* newPresentationInterface = static_cast<NewPresentationInterface*>(user_data);
  create(newPresentationInterface);
}

void on_open_activate (GtkMenuItem*, gpointer user_data)
{
  GtkWidget* dialog;
  GtkWidget* scroom = static_cast<GtkWidget*>(user_data);

  printf("Creating the open dialog\n");
  dialog = gtk_file_chooser_dialog_new ("Open File",
                                        GTK_WINDOW(scroom),
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                        NULL);
  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog), currentFolder.c_str());

  const std::map<OpenPresentationInterface::Ptr, std::string>& openPresentationInterfaces = PluginManager::getInstance()->getOpenPresentationInterfaces();
  const std::map<OpenInterface::Ptr, std::string>& openInterfaces = PluginManager::getInstance()->getOpenInterfaces();

  for(auto const& cur: openPresentationInterfaces)
  {
    for(auto const& f: cur.first->getFilters())
    {
      gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f);
    }
  }
  for(auto const& cur: openInterfaces)
  {
    for(auto const& f: cur.first->getFilters())
    {
      gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f);
    }
  }

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    GFile* file = g_file_new_for_path(gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)));
    GFileInfo* fileInfo = g_file_query_info(file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, NULL);
    GtkFileFilterInfo filterInfo;
    filterInfo.filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (dialog));
    filterInfo.mime_type = g_content_type_get_mime_type(g_file_info_get_content_type (fileInfo));
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
  gchar* cf =  gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
  if(cf)
  {
    currentFolder = cf;
    g_free(cf);
  }
  gtk_widget_destroy (dialog);
}

void on_save_activate (GtkMenuItem*, gpointer)
{

}

void on_save_as_activate (GtkMenuItem*, gpointer)
{

}

void on_quit_activate (GtkMenuItem*, gpointer)
{
  Views v(views);
  for(const Views::value_type& p: v)
  {
    p.first->hide();
  }
  gtk_main_quit();
}

void on_cut_activate (GtkMenuItem*, gpointer)
{

}

void on_copy_activate (GtkMenuItem*, gpointer)
{

}

void on_paste_activate (GtkMenuItem*, gpointer)
{

}

void on_delete_activate (GtkMenuItem*, gpointer)
{

}

void on_fullscreen_activate (GtkMenuItem* item, gpointer user_data)
{
  View* view = static_cast<View*>(user_data);
  GtkCheckMenuItem* cmi = GTK_CHECK_MENU_ITEM(item);
  gboolean active = gtk_check_menu_item_get_active(cmi);

  if(active)
    view->setFullScreen();
  else
    view->unsetFullScreen();
}

void on_close_activate (GtkMenuItem*, gpointer user_data)
{
  View* view = static_cast<View*>(user_data);

  view->hide();
}

void on_about_activate (GtkMenuItem*, gpointer)
{
  // GtkWidget* aboutdialog;
  // aboutdialog = create_aboutdialog ();
  gtk_dialog_run (GTK_DIALOG (aboutDialog ));
  gtk_widget_hide(aboutDialog);
  // gtk_widget_destroy (aboutdialog);
}

gboolean on_drawingarea_expose_event (GtkWidget* widget, GdkEventExpose*, gpointer user_data)
{
  // printf("expose\n");

  cairo_t* cr = gdk_cairo_create(widget->window);
  View* view = static_cast<View*>(user_data);
  view->redraw(cr);

  cairo_destroy(cr);
  return FALSE;
}

gboolean on_drawingarea_configure_event (GtkWidget*, GdkEventConfigure*, gpointer user_data)
{
  // printf("configure\n");
  View* view = static_cast<View*>(user_data);
  view->on_configure();
  return FALSE;
}

gboolean on_idle (gpointer user_data)
{
  if(user_data==NULL)
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
        gchar* dir = g_path_get_dirname(file.c_str());
        currentFolder = dir;
        g_free(dir);
        fn.pop_front();
      }
    }
    filenames.erase(REGULAR_FILES);

    PluginManager::Ptr instance = PluginManager::getInstance();
    std::map<std::string, NewAggregateInterface::Ptr> const& newAggregateInterfaces = instance->getNewAggregateInterfaces();

    for (FileNameMap::value_type const& v : filenames)
    {
      std::string const& aggregateName = v.first;
      std::list<std::string> const& files = v.second;

      std::map<std::string, NewAggregateInterface::Ptr>::const_iterator i =
        newAggregateInterfaces.find(aggregateName);
      if (i != newAggregateInterfaces.end())
      {
        try
        {
          Aggregate::Ptr aggregate = i->second->createNew();
          PresentationInterface::Ptr aggregatePresentation =
            boost::dynamic_pointer_cast<PresentationInterface>(aggregate);

          if (aggregatePresentation)
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
            printf("ERROR: Don't know how to display a %s\n", aggregateName.c_str());
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

void on_scroom_bootstrap (const FileNameMap& newFilenames)
{
  printf("Bootstrapping Scroom...\n");
  filenames = newFilenames;
  currentFolder = ".";

  bool devMode = NULL!=getenv(SCROOM_DEV_MODE.c_str());
  if(devMode)
  {
    printf("+----------------------------------------------------------------------+\n"
           "| ENTERING DEVELOPMENT MODE                                            |\n"
           "| All the default directories are not searched                         |\n"
           "| Instead, only environment variables and the local source tree        |\n"
           "| are consulted.                                                       |\n"
           "+----------------------------------------------------------------------+\n"
           );
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
  

  aboutDialogXml = glade_xml_new(xmlFileName.c_str(), "aboutDialog", NULL);
  if(aboutDialogXml!=NULL)
  {
    aboutDialog = glade_xml_get_widget(aboutDialogXml, "aboutDialog");
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutDialog), PACKAGE_NAME);
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

void create_scroom(PresentationInterface::Ptr presentation)
{
  GladeXML* xml = glade_xml_new(xmlFileName.c_str(), "scroom", NULL);

  if(xml==NULL)
  {
    printf("Opening xml failed\n");
    exit(-1);
  }

  View::Ptr view = View::create(xml, presentation);
  on_view_created(view);

  GtkWidget* scroom = glade_xml_get_widget(xml, "scroom");
  // GtkWidget* newMenuItem = glade_xml_get_widget(xml, "new");
  GtkWidget* openMenuItem = glade_xml_get_widget(xml, "open");
  GtkWidget* closeMenuItem = glade_xml_get_widget(xml, "close");
  GtkWidget* quitMenuItem = glade_xml_get_widget(xml, "quit");
  GtkWidget* fullScreenMenuItem = glade_xml_get_widget(xml, "fullscreen_menu_item");
  GtkWidget* aboutMenuItem = glade_xml_get_widget(xml, "about");
  GtkWidget* drawingArea = glade_xml_get_widget(xml, "drawingarea");
  GtkWidget* zoomBox = glade_xml_get_widget(xml, "zoomboxcombo");
  GtkWidget* vscrollbar = glade_xml_get_widget(xml, "vscrollbar");
  GtkWidget* hscrollbar = glade_xml_get_widget(xml, "hscrollbar");
  GtkAdjustment* vscrollbaradjustment = gtk_range_get_adjustment(GTK_RANGE(vscrollbar));
  GtkAdjustment* hscrollbaradjustment = gtk_range_get_adjustment(GTK_RANGE(hscrollbar));
  GtkEditable* xTextBox = GTK_EDITABLE(glade_xml_get_widget(xml, "x_textbox"));
  GtkEditable* yTextBox = GTK_EDITABLE(glade_xml_get_widget(xml, "y_textbox"));

  g_signal_connect (static_cast<gpointer>(scroom), "hide", G_CALLBACK (on_scroom_hide), view.get());
  g_signal_connect (static_cast<gpointer>(closeMenuItem), "activate", G_CALLBACK (on_close_activate), view.get());
  g_signal_connect (static_cast<gpointer>(quitMenuItem), "activate", G_CALLBACK (on_quit_activate), view.get());
  g_signal_connect (static_cast<gpointer>(openMenuItem), "activate", G_CALLBACK (on_open_activate), scroom);
  g_signal_connect (static_cast<gpointer>(fullScreenMenuItem), "activate", G_CALLBACK (on_fullscreen_activate), view.get());
  g_signal_connect (static_cast<gpointer>(zoomBox), "changed", G_CALLBACK (on_zoombox_changed), view.get());
  g_signal_connect (static_cast<gpointer>(vscrollbaradjustment), "value-changed", G_CALLBACK(on_scrollbar_value_changed), view.get());
  g_signal_connect (static_cast<gpointer>(hscrollbaradjustment), "value-changed", G_CALLBACK(on_scrollbar_value_changed), view.get());
  g_signal_connect (static_cast<gpointer>(xTextBox), "changed", G_CALLBACK(on_textbox_value_changed), view.get());
  g_signal_connect (static_cast<gpointer>(yTextBox), "changed", G_CALLBACK(on_textbox_value_changed), view.get());
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
  g_signal_connect (static_cast<gpointer>(aboutMenuItem), "activate", G_CALLBACK (on_about_activate), view.get());
  g_signal_connect (static_cast<gpointer>(drawingArea), "expose_event", G_CALLBACK (on_drawingarea_expose_event), view.get());
  g_signal_connect (static_cast<gpointer>(drawingArea), "configure_event", G_CALLBACK (on_drawingarea_configure_event), view.get());
  g_signal_connect (static_cast<gpointer>(drawingArea), "button-press-event", G_CALLBACK (on_button_press_event), view.get());
  g_signal_connect (static_cast<gpointer>(drawingArea), "button-release-event", G_CALLBACK (on_button_release_event), view.get());
  g_signal_connect (static_cast<gpointer>(drawingArea), "scroll-event", G_CALLBACK (on_scroll_event), view.get());
  g_signal_connect (static_cast<gpointer>(drawingArea), "motion-notify-event", G_CALLBACK (on_motion_notify_event), view.get());
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
  presentations.push_back(presentation);

  for(const Views::value_type& p: views)
  {
    p.first->on_presentation_created(presentation);
  }

  const std::map<PresentationObserver::Ptr, std::string>& presentationObservers =
    PluginManager::getInstance()->getPresentationObservers();

  std::map<PresentationObserver::Ptr, std::string>::const_iterator cur = presentationObservers.begin();
  std::map<PresentationObserver::Ptr, std::string>::const_iterator end = presentationObservers.end();
  for(;cur!=end; cur++)
    cur->first->presentationAdded(presentation);
}

void on_view_created(View::Ptr v)
{
  Scroom::Bookkeeping::Token t;
  views[v] = t;

  for(std::list<PresentationInterface::WeakPtr>::iterator cur = presentations.begin();
      cur != presentations.end(); cur++)
  {
    PresentationInterface::Ptr p = cur->lock();
    if(p)
    {
      v->on_presentation_created(p);
    }
  }

  const std::map<ViewObserver::Ptr, std::string>& viewObservers =
    PluginManager::getInstance()->getViewObservers();

  std::map<ViewObserver::Ptr, std::string>::const_iterator cur = viewObservers.begin();
  std::map<ViewObserver::Ptr, std::string>::const_iterator end = viewObservers.end();
  for(;cur!=end; cur++)
    t.add(cur->first->viewAdded(v));
}

void on_presentation_possibly_destroyed()
{
  bool presentationDestroyed = false;

  for (std::list<PresentationInterface::WeakPtr>::iterator cur =
      presentations.begin(); cur != presentations.end(); cur++)
  {
    PresentationInterface::Ptr p = cur->lock();
    if (!p)
    {
      presentationDestroyed = true;
      std::list<PresentationInterface::WeakPtr>::iterator temp = cur;
      temp--;
      presentations.erase(cur);
      cur = temp;
    }
  }
  if (presentationDestroyed)
  {
    for (auto const& p : views)
      p.first->on_presentation_destroyed();

    const std::map<PresentationObserver::Ptr, std::string>& presentationObservers =
        PluginManager::getInstance()->getPresentationObservers();

    for(auto const& p: presentationObservers)
      p.first->presentationDeleted();
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
  for(std::list<PresentationInterface::WeakPtr>::iterator cur = presentations.begin();
      cur != presentations.end(); cur++)
  {
    PresentationInterface::Ptr p = cur->lock();
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

