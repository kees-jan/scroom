#include "colormapprovider.hh"

// #include <tiffpresentation.hh>
#include <colormappable.hh>

#include "colormaps.hh"

ColormapProvider::ColormapProvider(PresentationInterface::Ptr p)
  : presentation(p)
{
  Colormappable* c = dynamic_cast<Colormappable*>(p.get());
  if(c)
    c->registerObserver(this);
  else
    printf("PANIC: Presentation doesn't implement Colormappable\n");
}

ColormapProvider::~ColormapProvider()
{
  PresentationInterface::Ptr p = presentation.lock();
  if(p)
  {
    Colormappable* c = dynamic_cast<Colormappable*>(p.get());
    if(c)
      c->unregisterObserver(this);
    else
      printf("PANIC: Presentation doesn't implement Colormappable\n");
  }
}

void callback(GtkButton *button, gpointer user_data)
{
  ViewInterface* vi = (ViewInterface*)user_data;
  vi->removeSideWidget(GTK_WIDGET(button));
}


void ColormapProvider::open(ViewInterface* vi)
{
  printf("ColormapProvider: Adding a view.\n");
  views.push_back(vi);
  GtkListStore* filenames = Colormaps::getInstance().getFileNames();
  GtkTreeView* tv = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(filenames)));
  GtkCellRenderer* txt = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
  gtk_tree_view_insert_column_with_attributes(tv, -1, "Name", txt, "text", COLUMN_NAME, NULL);

  vi->addSideWidget("Colormap", GTK_WIDGET(tv));
  // g_signal_connect ((gpointer)button, "clicked", G_CALLBACK (callback), vi);
}

void ColormapProvider::close(ViewInterface* vi)
{
  printf("ColormapProvider: Removing a view.\n");
  views.remove(vi);
  if(views.empty())
  {
    printf("ColormapProvider: Last view has gone. Self-destructing\n");
    delete this;
  }
}
