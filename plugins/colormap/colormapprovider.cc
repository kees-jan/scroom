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

/** Names of the columns in the GtkListStore */
enum
  {
    COLUMN_NAME,
    COLUMN_POINTER,
    N_COLUMNS
  };

////////////////////////////////////////////////////////////////////////

ColormapProvider::ColormapProvider(PresentationInterface::Ptr p)
  : presentation(p), colormaps(NULL)
{
  Colormappable* c = dynamic_cast<Colormappable*>(p.get());
  if(c)
  {
    int numColors = c->getNumberOfColors();
    std::list<Colormap::ConstPtr> maps = Colormaps::getInstance().getColormaps();

    colormaps = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);

    {
      // Add a default map containing the grey values.
      Colormap::ConstPtr* cc = new Colormap::ConstPtr(Colormap::createDefault(numColors));
      GtkTreeIter iter;
      gtk_list_store_append(colormaps, &iter);
      gtk_list_store_set(colormaps, &iter,
                         COLUMN_NAME, (*cc)->name.c_str(),
                         COLUMN_POINTER, cc,
                         -1);
    }
    
    while(!maps.empty())
    {
      if(maps.front()->colors.size()>=numColors)
      {
        Colormap::ConstPtr* cc = new Colormap::ConstPtr(maps.front());

        GtkTreeIter iter;
        gtk_list_store_append(colormaps, &iter);
        gtk_list_store_set(colormaps, &iter,
                           COLUMN_NAME, (*cc)->name.c_str(),
                           COLUMN_POINTER, cc,
                           -1);
      }

      maps.pop_front();
    }
    
    c->registerObserver(this);
  }
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

  // Clear out all the smart pointers
  if(colormaps)
  {
    GtkTreeIter iter;
    while(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(colormaps), &iter))
    {
      gpointer* pointer = NULL;
      gtk_tree_model_get(GTK_TREE_MODEL(colormaps), &iter, COLUMN_POINTER, &pointer, -1);
      Colormap::Ptr* colormap = (Colormap::Ptr*)pointer;
      delete colormap;
      gtk_list_store_remove(colormaps, &iter);
    }
    g_object_unref(colormaps);
  }
  colormaps = NULL;
}

void ColormapProvider::open(ViewInterface* vi)
{
  printf("ColormapProvider: Adding a view.\n");
  GtkTreeView* tv = GTK_TREE_VIEW(gtk_tree_view_new_with_model(GTK_TREE_MODEL(colormaps)));
  GtkCellRenderer* txt = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
  gtk_tree_view_insert_column_with_attributes(tv, -1, "Name", txt, "text", COLUMN_NAME, NULL);
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
      Colormappable* colormappable = dynamic_cast<Colormappable*>(p.get());
      if(colormappable)
      {
        if(gtk_list_store_iter_is_valid(colormaps, &iter))
        {
          gpointer* pointer = NULL;
          gtk_tree_model_get(GTK_TREE_MODEL(colormaps), &iter, COLUMN_POINTER, &pointer, -1);
          Colormap::Ptr& colormap = *(Colormap::Ptr*)pointer;
          colormappable->setColormap(colormap);
        }
      }
      else
        printf("PANIC: Presentation doesn't implement Colormappable\n");
    }
  }
  else
    printf("PANIC: Presentation is gone??");
}
