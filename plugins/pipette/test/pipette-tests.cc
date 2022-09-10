/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <memory>
#include <stack>
#include <thread>

#include <boost/dll.hpp>
#include <boost/test/unit_test.hpp>

#include <scroom/gtk-test-helpers.hh>

#include "pipette.hh"

BOOST_FIXTURE_TEST_SUITE(Pipette_Tests, Scroom::GtkTestHelpers::GtkMainLoop)

class DummyPresentation
  : public PresentationInterface
  , public PipetteViewInterface
{
public:
  static PresentationInterface::Ptr create() { return PresentationInterface::Ptr(new DummyPresentation()); }

  Scroom::Utils::Rectangle<double> getRect() override { return {0, 0, 100, 100}; }
  void                             redraw(ViewInterface::Ptr const& /*vi*/,
                                          cairo_t* /*cr*/,
                                          Scroom::Utils::Rectangle<double> /*presentationArea*/,
                                          int /*zoom*/) override
  {
  }
  bool        getProperty(const std::string& /*name*/, std::string& /*value*/) override { return false; }
  bool        isPropertyDefined(const std::string& name) override { return PIPETTE_PROPERTY_NAME == name; }
  std::string getTitle() override { return {}; }
  void        open(ViewInterface::WeakPtr /*vi*/) override{};
  void        close(ViewInterface::WeakPtr /*vi*/) override{};
  PipetteLayerOperations::PipetteColor getPixelAverages(Scroom::Utils::Rectangle<double> /*area*/) override
  {
    return {{"C", 1.0}};
  }
};

class DummyView : public ViewInterface
{
public:
  using Ptr = std::shared_ptr<DummyView>;

  static Ptr createWithPresentation() { return create(DummyPresentation::create()); }
  static Ptr createWithoutPresentation() { return create(nullptr); }
  static Ptr create(PresentationInterface::Ptr presentation_) { return std::make_shared<DummyView>(std::move(presentation_)); }

  explicit DummyView(PresentationInterface::Ptr presentation_)
    : presentation(std::move(presentation_))
  {
  }

  void                   invalidate() override {}
  ProgressInterface::Ptr getProgressInterface() override { return nullptr; }
  void                   addSideWidget(std::string /*title*/, GtkWidget* /*w*/) override {}
  void                   removeSideWidget(GtkWidget* /*w*/) override {}
  void                   addToToolbar(GtkToolItem* /*ti*/) override {}
  void                   removeFromToolbar(GtkToolItem* /*ti*/) override {}
  void                   registerSelectionListener(SelectionListener::Ptr /*unused*/) override { reg_sel++; }
  void                   registerPostRenderer(PostRenderer::Ptr /*unused*/) override { reg_post++; }
  void                   setStatusMessage(const std::string& msg) override
  {
    boost::mutex::scoped_lock const l(mut);
    statusMessages.push_back(msg);
    cond.notify_all();
  }
  PresentationInterface::Ptr getCurrentPresentation() override { return presentation; }
  void addToolButton(GtkToggleButton* /*unused*/, ToolStateListener::Ptr /*unused*/) override { tool_btn++; }

  std::string nextStatusMessage()
  {
    boost::mutex::scoped_lock l(mut);

    BOOST_REQUIRE(cond.wait_for(l, boost::chrono::milliseconds(500), [&] { return !statusMessages.empty(); }));

    auto result = statusMessages.front();
    statusMessages.pop_front();
    return result;
  }

  int                        reg_sel  = 0;
  int                        reg_post = 0;
  int                        tool_btn = 0;
  PresentationInterface::Ptr presentation;

  boost::mutex              mut;
  boost::condition_variable cond;
  std::list<std::string>    statusMessages;
};

class DummyPluginInterface : public ScroomPluginInterface
{
public:
  using Ptr = std::shared_ptr<DummyPluginInterface>;

  static Ptr create() { return std::make_shared<DummyPluginInterface>(); }

  void registerNewPresentationInterface(const std::string& /*identifier*/,
                                        NewPresentationInterface::Ptr /*newPresentationInterface*/) override{};
  void registerNewAggregateInterface(const std::string& /*identifier*/,
                                     NewAggregateInterface::Ptr /*newAggregateInterface*/) override{};
  void registerOpenPresentationInterface(const std::string& /*identifier*/,
                                         OpenPresentationInterface::Ptr /*openPresentationInterface*/) override{};
  void registerOpenTiledBitmapInterface(
    const std::string& /*identifier*/,
    std::shared_ptr<Scroom::TiledBitmap::OpenTiledBitmapInterface> /*openTiledBitmapInterface*/) override{};
  void registerOpenInterface(const std::string& /*identifier*/, OpenInterface::Ptr /*openInterface*/) override{};
  void registerViewObserver(const std::string& /*identifier*/, ViewObserver::Ptr /*observer*/) override { view_observers++; };
  void registerPresentationObserver(const std::string& /*identifier*/, PresentationObserver::Ptr /*observer*/) override{};

  int view_observers = 0;
};

BOOST_AUTO_TEST_CASE(pipette_selection_end)
{
  PipetteHandler::Ptr const handler = PipetteHandler::create();

  Selection const sel(10, 11);

  handler->onSelectionEnd(sel, nullptr);
  BOOST_CHECK(!handler->getSelection());

  handler->onEnable();
  handler->onSelectionEnd(sel, DummyView::createWithPresentation());
  auto selection = handler->getSelection();
  BOOST_REQUIRE(selection);
  Scroom::Utils::Point<double> const expected{10, 11};
  BOOST_CHECK_EQUAL(selection->start, expected);

  handler->onDisable();
  handler->onSelectionEnd(sel, nullptr);
  BOOST_CHECK(!handler->getSelection());
}

BOOST_AUTO_TEST_CASE(pipette_selection_update)
{
  PipetteHandler::Ptr const handler = PipetteHandler::create();

  Selection const sel(10, 11);

  // should not do anything but will be called from the view so should not crash
  handler->onSelectionStart(sel, nullptr);

  handler->onSelectionUpdate(sel, nullptr);
  BOOST_CHECK(!handler->getSelection());

  handler->onEnable();
  handler->onSelectionUpdate(sel, nullptr);
  auto selection = handler->getSelection();
  BOOST_REQUIRE(selection);
  Scroom::Utils::Point<double> const expected{10, 11};
  BOOST_CHECK_EQUAL(selection->start, expected);

  ViewInterface::Ptr const vi = DummyView::createWithPresentation();
  cairo_t*                 cr = cairo_create(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));
  handler->render(vi, cr, {0, 0, 0, 0}, 1);
  handler->render(vi, cr, {0, 0, 0, 0}, -2);

  handler->onDisable();
  handler->onSelectionUpdate(sel, nullptr);
  BOOST_CHECK(!handler->getSelection());
  handler->render(vi, cr, {0, 0, 0, 0}, 1);
}

BOOST_AUTO_TEST_CASE(pipette_enable_disable)
{
  PipetteHandler::Ptr const handler = PipetteHandler::create();

  // questionably useful
  handler->onEnable();
  BOOST_CHECK(handler->isEnabled());
  handler->onDisable();
  BOOST_CHECK(!handler->isEnabled());
}

BOOST_AUTO_TEST_CASE(pipette_metadata)
{
  const auto pluginInterface = DummyPluginInterface::create();

  Pipette::Ptr const pipette = Pipette::create();

  const int pre_view_observers = pluginInterface->view_observers;

  pipette->registerCapabilities(pluginInterface);

  // maybe not worth testing
  BOOST_CHECK_EQUAL(pipette->getPluginName(), "Pipette");
  BOOST_CHECK(!pipette->getPluginVersion().empty());
  BOOST_CHECK_EQUAL(pre_view_observers + 1, pluginInterface->view_observers);
}

BOOST_AUTO_TEST_CASE(pipette_value_display_presentation)
{
  PipetteHandler::Ptr const handler = PipetteHandler::create();
  const auto                view    = DummyView::createWithPresentation();

  handler->onEnable();

  handler->computeValues(view, Scroom::Utils::Rectangle<double>(10, 11, 12, 13));
  BOOST_CHECK_EQUAL(view->nextStatusMessage(), "Computing color values...");
  BOOST_CHECK_EQUAL(view->nextStatusMessage(),
                    "Top-left: (10,11), Bottom-right: (22,24), Height: 13, Width: 12, "
                    "Colors: C: 1.00");

  handler->computeValues(view, Scroom::Utils::Rectangle<double>(-10, -11, 20, 22));
  BOOST_CHECK_EQUAL(view->nextStatusMessage(), "Computing color values...");
  BOOST_CHECK_EQUAL(view->nextStatusMessage(),
                    "Top-left: (0,0), Bottom-right: (10,11), Height: 11, Width: 10, "
                    "Colors: C: 1.00");
}

BOOST_AUTO_TEST_CASE(pipette_value_display_no_presentation)
{
  PipetteHandler::Ptr const handler = PipetteHandler::create();
  const auto                view    = DummyView::createWithoutPresentation();

  handler->onEnable();

  handler->computeValues(view, Scroom::Utils::Rectangle<double>(10, 11, 12, 13));
  BOOST_CHECK_EQUAL(view->nextStatusMessage(), "Computing color values...");
  BOOST_CHECK_EQUAL(view->nextStatusMessage(), "Pipette is not supported for this presentation.");
}

BOOST_AUTO_TEST_CASE(pipette_view_add)
{
  Pipette::Ptr const pipette = Pipette::create();
  const auto         view    = DummyView::createWithPresentation();

  const int pre_reg_sel  = view->reg_sel;
  const int pre_reg_post = view->reg_post;
  const int pre_tool_btn = view->tool_btn;

  Scroom::Bookkeeping::Token const token = pipette->viewAdded(view);

  BOOST_CHECK_EQUAL(pre_reg_sel + 1, view->reg_sel);
  BOOST_CHECK_EQUAL(pre_reg_post + 1, view->reg_post);
  BOOST_CHECK_EQUAL(pre_tool_btn + 1, view->tool_btn);
  BOOST_CHECK(token != nullptr);
}

BOOST_AUTO_TEST_SUITE_END()
