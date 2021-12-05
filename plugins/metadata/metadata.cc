/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "metadata.hh"

#include <spdlog/spdlog.h>

#include <gdk/gdk.h>

#include <scroom/showmetadatainterface.hh>

/** On Metadata button press, open and populate properties window
 *
 *  @param user_data gpointer to the view passed on clicking metadata button
 *  @post metadata of image is shown in window if presentation != null
 */
void on_image_properties_activate(GtkMenuItem*, gpointer user_data)
{
  auto* view                  = static_cast<ViewInterface*>(user_data);
  auto  showMetaDataInterface = boost::dynamic_pointer_cast<ShowMetadataInterface>(view->getCurrentPresentation());
  require(showMetaDataInterface);

  showMetaDataInterface->showMetadata();
}

////////////////////////////////////////////////////////////////////////
// Metadata
////////////////////////////////////////////////////////////////////////

/**
 * Creates a pointer to a metadata object
 *
 * @return Pointer to metadata object
 */
Metadata::Ptr Metadata::create() { return Ptr(new Metadata()); }

////////////////////////////////////////////////////////////////////////
// PluginInformationInterface
////////////////////////////////////////////////////////////////////////

/**
 * Gets the name of metadata plugin
 *
 * @return std::string "Metadata"
 */
std::string Metadata::getPluginName() { return "Metadata"; }

/**
 * Gets the version number of metadata plugin
 *
 * @return std::string "0.0"
 */
std::string Metadata::getPluginVersion() { return "0.0"; }

/**
 * Register plugin to observers
 *
 * @param host Pointer to scroom plugin interface
 * @return std::string "0.0"
 */
void Metadata::registerCapabilities(ScroomPluginInterface::Ptr host)
{
  host->registerViewObserver("Metadata", shared_from_this<Metadata>());
}

////////////////////////////////////////////////////////////////////////
// ViewObserver
////////////////////////////////////////////////////////////////////////

/**
 * Shows an image metadata button in the view that can be activated by clicking.
 *
 * @param view Screen or view that is displayed
 * @post Metadata button is shown in toolbar
 * @return Scroom::Bookkeeping::Token token for bookkeeping of scroom
 */
Scroom::Bookkeeping::Token Metadata::viewAdded(ViewInterface::Ptr view)
{
  auto presentation          = view->getCurrentPresentation();
  auto showMetaDataInterface = boost::dynamic_pointer_cast<ShowMetadataInterface>(presentation);
  if(presentation->isPropertyDefined(METADATA_PROPERTY_NAME))
  {
    require(showMetaDataInterface);

    // create button for metadata and add it to the toolbar
    GtkToolItem* button         = gtk_tool_item_new();
    GtkWidget*   buttonMetadata = gtk_button_new_with_label("Metadata");
    gtk_widget_set_visible(buttonMetadata, true);
    gtk_container_add(GTK_CONTAINER(button), buttonMetadata);
    // connect signal to the button for when it is being pressed
    g_signal_connect(static_cast<gpointer>(buttonMetadata), "pressed", G_CALLBACK(on_image_properties_activate), view.get());

    Scroom::GtkHelpers::sync_on_ui_thread([&] {
      view->addToToolbar(button); // adds metadata button next to other tools in toolbar
    });
  }
  return {};
}
