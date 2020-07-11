/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/test/unit_test.hpp>

#include <algorithm>

#include <scroom/presentationinterface.hh>
#include <scroom/resizablepresentationinterface.hh>
#include <scroom/gtk-helpers.hh>

#include "sizedeterminer.hh"

using Scroom::GtkHelpers::createGdkRectangle;

template<typename Iter>
std::ostream& dumpContainer(std::ostream& os, std::string const& name, Iter const& begin, Iter const& end)
{
  os << name << "(";
  Iter cur = begin;
  if(cur != end)
    os << *cur++;

  while(cur != end)
    os << ", " << *cur++;

  os << ")";
  return os;
}

namespace std
{
  template<typename T>
  std::ostream& operator<<(std::ostream& os, std::list<T> const& l)
  {
    return dumpContainer(os, "std::list", l.begin(), l.end());
  }
}
template<typename T>
bool operator==(boost::weak_ptr<T> const& left, boost::weak_ptr<T> const& right)
{
  return !(left<right) && !(right<left);
}

namespace
{
  class PresentationInterfaceStub : public PresentationInterface
  {
  public:
    typedef boost::shared_ptr<PresentationInterfaceStub> Ptr;

  private:
    Scroom::Utils::Rectangle<double> rect;

  protected:
    PresentationInterfaceStub(Scroom::Utils::Rectangle<double> const& rect_)
      : rect(rect_)
    {}

  public:
    static Ptr create(Scroom::Utils::Rectangle<double> const& rect) { return Ptr(new PresentationInterfaceStub(rect)); }

    virtual Scroom::Utils::Rectangle<double> getRect() { return rect; }

    virtual void redraw(ViewInterface::Ptr const&, cairo_t*, Scroom::Utils::Rectangle<double>, int) {}
    virtual bool getProperty(const std::string&, std::string&) { return false; }
    virtual bool isPropertyDefined(const std::string&) { return false; }
    virtual std::string getTitle() { return ""; }

    virtual void open(ViewInterface::WeakPtr) {}
    virtual void close(ViewInterface::WeakPtr) {}
  };

  class ResizablePresentationInterfaceStub: public PresentationInterfaceStub, public ResizablePresentationInterface
  {
  public:
    typedef boost::shared_ptr<ResizablePresentationInterfaceStub> Ptr;

  public:
    std::list<ViewInterface::WeakPtr> receivedVi;
    std::list<Scroom::Utils::Rectangle<double>> receivedRect;

  private:
    ResizablePresentationInterfaceStub(Scroom::Utils::Rectangle<double> const& rect_)
      : PresentationInterfaceStub(rect_)
    {}

  public:
    static Ptr create(Scroom::Utils::Rectangle<double> const& rect) { return Ptr(new ResizablePresentationInterfaceStub(rect)); }

    virtual void setRect(ViewInterface::WeakPtr const& vi, Scroom::Utils::Rectangle<double> const& rect_)
    {
      receivedVi.push_back(vi);
      receivedRect.push_back(rect_);
    }

    void CheckAllEqual(Scroom::Utils::Rectangle<double> const& rect_)
    {
      BOOST_CHECK_EQUAL(std::list<Scroom::Utils::Rectangle<double>>(receivedRect.size(), rect_), receivedRect);
    }

    bool Contains(ViewInterface::WeakPtr const& vi)
    {
      return std::find(receivedVi.begin(), receivedVi.end(), vi) != receivedVi.end();
    }

    void CheckEmpty()
    {
      BOOST_CHECK(receivedVi.empty());
      BOOST_CHECK(receivedRect.empty());
    }

    void Clear()
    {
      receivedVi.clear();
      receivedRect.clear();
    }
  };

  class ViewInterfaceDummy : public ViewInterface
  {
  private:
    ViewInterfaceDummy() {}

  public:
    static Ptr create() { return Ptr(new ViewInterfaceDummy()); }

    virtual void invalidate() {}
    virtual ProgressInterface::Ptr getProgressInterface() { return ProgressInterface::Ptr(); }
    virtual void addSideWidget(std::string, GtkWidget*) {}
    virtual void removeSideWidget(GtkWidget*) {}
    virtual void addToToolbar(GtkToolItem*) {}
    virtual void removeFromToolbar(GtkToolItem*) {}
    virtual void registerSelectionListener(SelectionListener::Ptr) {};
    virtual void registerPostRenderer(PostRenderer::Ptr) {};
    virtual void setStatusMessage(const std::string&) {};
    virtual boost::shared_ptr<PresentationInterface> getCurrentPresentation() { return boost::shared_ptr<PresentationInterface>(); };
    virtual void addToolButton(GtkToggleButton*, ToolStateListener::Ptr) {};
  };

}

BOOST_AUTO_TEST_SUITE(Determine_size_tests)

BOOST_AUTO_TEST_CASE(determine_size_of_one_regular)
{
  Scroom::Utils::Rectangle<double> const expected = createGdkRectangle(1,2,3,4);
  PresentationInterfaceStub::Ptr p = PresentationInterfaceStub::create(expected);
  SizeDeterminer::Ptr sd = SizeDeterminer::create();
  sd->add(p);

  BOOST_CHECK_EQUAL(expected, sd->getRect());
}

BOOST_AUTO_TEST_CASE(determine_size_of_two_regular)
{
  Scroom::Utils::Rectangle<double> const expected = createGdkRectangle(1,1,5,5);
  PresentationInterfaceStub::Ptr p1 = PresentationInterfaceStub::create(createGdkRectangle(1,2,3,4));
  PresentationInterfaceStub::Ptr p2 = PresentationInterfaceStub::create(createGdkRectangle(2,1,4,3));
  SizeDeterminer::Ptr sd = SizeDeterminer::create();
  sd->add(p1);
  sd->add(p2);

  BOOST_CHECK_EQUAL(expected, sd->getRect());
}

BOOST_AUTO_TEST_CASE(determine_size_of_one_regular_one_resizable)
{
  Scroom::Utils::Rectangle<double> const expected = createGdkRectangle(2,1,4,3);
  ResizablePresentationInterfaceStub::Ptr p1 = ResizablePresentationInterfaceStub::create(createGdkRectangle(1,2,3,4));
  PresentationInterfaceStub::Ptr p2 = PresentationInterfaceStub::create(createGdkRectangle(2,1,4,3));
  SizeDeterminer::Ptr sd = SizeDeterminer::create();
  sd->add(p1);
  sd->add(p2);

  BOOST_CHECK_EQUAL(expected, sd->getRect());
  BOOST_CHECK(p1->receivedVi.empty());
  BOOST_CHECK(p1->receivedRect.empty());
  ViewInterface::Ptr v1 = ViewInterfaceDummy::create();
  sd->open(p1, v1);
  p1->CheckAllEqual(expected);
  BOOST_CHECK(p1->Contains(v1));

  p1->Clear();
  ViewInterface::Ptr v2 = ViewInterfaceDummy::create();
  sd->open(p2, v2);
  p1->CheckEmpty();
}

BOOST_AUTO_TEST_CASE(determine_size_of_two_resizable)
{
  Scroom::Utils::Rectangle<double> const expected = createGdkRectangle(1,1,5,5);
  ResizablePresentationInterfaceStub::Ptr p1 = ResizablePresentationInterfaceStub::create(createGdkRectangle(1,2,3,4));
  ResizablePresentationInterfaceStub::Ptr p2 = ResizablePresentationInterfaceStub::create(createGdkRectangle(2,1,4,3));
  SizeDeterminer::Ptr sd = SizeDeterminer::create();
  sd->add(p1);
  sd->add(p2);

  BOOST_CHECK_EQUAL(expected, sd->getRect());

  p1->CheckEmpty();
  ViewInterface::Ptr v1 = ViewInterfaceDummy::create();
  sd->open(p1, v1);
  p1->CheckAllEqual(expected);
  BOOST_CHECK(p1->Contains(v1));
  p2->CheckEmpty();

  p1->Clear();
  ViewInterface::Ptr v2 = ViewInterfaceDummy::create();
  sd->open(p2, v2);
  p2->CheckAllEqual(expected);
  BOOST_CHECK(p2->Contains(v2));
  p1->CheckEmpty();
}

BOOST_AUTO_TEST_CASE(open_a_view_then_add_presentations_one_regular_one_resizable)
{
  SizeDeterminer::Ptr sd = SizeDeterminer::create();

  Scroom::Utils::Rectangle<double> r1 = createGdkRectangle(1,2,3,4);
  ResizablePresentationInterfaceStub::Ptr p1 = ResizablePresentationInterfaceStub::create(r1);
  sd->add(p1);
  ViewInterface::Ptr v1 = ViewInterfaceDummy::create();
  sd->open(p1, v1);
  BOOST_CHECK_EQUAL(r1, sd->getRect());
  p1->CheckAllEqual(r1);
  BOOST_CHECK(p1->Contains(v1));
  p1->Clear();

  Scroom::Utils::Rectangle<double> const r2 = createGdkRectangle(2,1,4,3);
  PresentationInterfaceStub::Ptr p2 = PresentationInterfaceStub::create(r2);
  sd->add(p2);
  BOOST_CHECK_EQUAL(r2, sd->getRect());
  p1->CheckAllEqual(r2);
  BOOST_CHECK(p1->Contains(v1));
  p1->Clear();
  ViewInterface::Ptr v2 = ViewInterfaceDummy::create();
  sd->open(p2, v2);
  p1->CheckEmpty();
}

BOOST_AUTO_TEST_CASE(updates_are_sent_to_multiple_views)
{
  SizeDeterminer::Ptr sd = SizeDeterminer::create();

  Scroom::Utils::Rectangle<double> r1 = createGdkRectangle(1,2,3,4);
  ResizablePresentationInterfaceStub::Ptr p1 = ResizablePresentationInterfaceStub::create(r1);
  sd->add(p1);
  ViewInterface::Ptr vi1 = ViewInterfaceDummy::create();
  sd->open(p1, vi1);
  ViewInterface::Ptr vi2 = ViewInterfaceDummy::create();
  sd->open(p1, vi2);
  ViewInterface::Ptr vi3 = ViewInterfaceDummy::create();
  sd->open(p1, vi3);

  BOOST_CHECK_EQUAL(r1, sd->getRect());
  p1->CheckAllEqual(r1);
  BOOST_CHECK(p1->Contains(vi1));
  BOOST_CHECK(p1->Contains(vi2));
  BOOST_CHECK(p1->Contains(vi3));
  p1->Clear();

  sd->close(p1, vi3);
  Scroom::Utils::Rectangle<double> const r2 = createGdkRectangle(2,1,4,3);
  PresentationInterfaceStub::Ptr p2 = PresentationInterfaceStub::create(r2);
  sd->add(p2);
  BOOST_CHECK_EQUAL(r2, sd->getRect());
  p1->CheckAllEqual(r2);
  BOOST_CHECK(p1->Contains(vi1));
  BOOST_CHECK(p1->Contains(vi2));
  BOOST_CHECK(!p1->Contains(vi3));
}

BOOST_AUTO_TEST_SUITE_END()
