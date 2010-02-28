#include "colormapprovider.hh"

// #include <tiffpresentation.hh>
#include <colormappable.hh>

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
  views.push_back(vi);
  GtkWidget* button = gtk_button_new_with_label("Test 1..2..3...");
  vi->addSideWidget("Colormap", button);
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
