//
// Created by andy on 13-06-21.
//

#include "metadata.hh"

#include <gdk/gdk.h>

/** On Metadata button press, open and populate properties window
 *
 *  @param user_data gpointer to the view passed on clicking metadata button
 *  @post metadata of image is shown in window if presentation != null
 **/
void on_image_properties_activate(GtkMenuItem*, gpointer user_data)
{
  auto*      view   = static_cast<ViewInterface*>(user_data);
  GtkWidget* dialog = nullptr;
  GtkWindow* main   = nullptr;

  if(view->getCurrentPresentation().get() != nullptr) // if there is a presentation
  {
    view->getCurrentPresentation()->showMetadata(); // call method to show metadata
  }
  else
  { // there is no presentation => Show window with warning
    dialog = gtk_message_dialog_new(main,
                                    static_cast<GtkDialogFlags>(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                                    GTK_MESSAGE_WARNING,
                                    GTK_BUTTONS_OK,
                                    "No image loaded!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_hide(dialog);
  }
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
  // create button for metadata and add it to the toolbar
  GtkToolItem* button         = gtk_tool_item_new();
  GtkWidget*   buttonMetadata = gtk_button_new_with_label("Metadata");
  gtk_widget_set_visible(buttonMetadata, true);
  gpointer presentation = view.get();
  gtk_container_add(GTK_CONTAINER(button), buttonMetadata);
  // connect signal to the button for when it is being pressed
  g_signal_connect(static_cast<gpointer>(buttonMetadata), "pressed", G_CALLBACK(on_image_properties_activate), presentation);

  gdk_threads_enter();
  view->addToToolbar(button); // adds metadata button next to other tools in toolbar
  gdk_threads_leave();

  return Scroom::Bookkeeping::Token();
}
