#include "colormapprovider.hh"

#include <colormappable.hh>

#include "colormaps.hh"

////////////////////////////////////////////////////////////////////////

void on_colormap_selected (GtkTreeView *tv, gpointer user_data)
{
  ColormapProvider* cmp = (ColormapProvider*)user_data;
  cmp->on_colormap_selected(tv);
}

////////////////////////////////////////////////////////////////////////

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

void ColormapProvider::open(ViewInterface* vi)
{
  printf("ColormapProvider: Adding a view.\n");
  GtkListStore* filenames = Colormaps::getInstance().getFileNames();
  GtkTreeView* tv = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(filenames)));
  GtkCellRenderer* txt = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
  gtk_tree_view_insert_column_with_attributes(tv, -1, "Name", txt, "text", COLUMN_NAME, "sensitive", COLUMN_ENABLED, NULL);
  g_signal_connect ((gpointer)tv, "cursor_changed", G_CALLBACK (::on_colormap_selected), this);
  views[vi]=tv;

  vi->addSideWidget("Colormap", GTK_WIDGET(tv));
}

void ColormapProvider::close(ViewInterface* vi)
{
  printf("ColormapProvider: Removing a view.\n");
  std::map<ViewInterface*, GtkTreeView*>::iterator cur = views.find(vi);
  if(cur != views.end())
    views.erase(cur);
  if(views.empty())
  {
    printf("ColormapProvider: Last view has gone. Self-destructing\n");
    delete this;
  }
}

void ColormapProvider::on_colormap_selected(GtkTreeView* tv)
{
  PresentationInterface::Ptr p = presentation.lock();
  if(p)
  {
    GtkTreeSelection* ts = gtk_tree_view_get_selection(tv);
    GtkTreeIter iter;
    GtkTreeModel* model = NULL;
    bool selected = gtk_tree_selection_get_selected(ts, &model, &iter);
    if(selected)
    {
      Colormappable* c = dynamic_cast<Colormappable*>(p.get());
      if(c)
        Colormaps::getInstance().select(iter, c);
      else
        printf("PANIC: Presentation doesn't implement Colormappable\n");
    }
  }
  else
    printf("PANIC: Presentation is gone??");
}
