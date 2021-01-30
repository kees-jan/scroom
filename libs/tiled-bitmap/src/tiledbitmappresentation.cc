/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <utility>

#include <scroom/opentiledbitmapinterface.hh>

namespace
{
  using namespace Scroom::TiledBitmap;

  class TiledBitmapPresentation
  {
  };

  class OpenTiledBitmapAsPresentation : public OpenPresentationInterface
  {
  public:
    using Ptr = boost::shared_ptr<OpenTiledBitmapAsPresentation>;

  private:
    OpenTiledBitmapInterface::Ptr openTiledBitmapInterface;

  public:
    static Ptr create(OpenTiledBitmapInterface::Ptr openTiledBitmapInterface_)
    {
      return Ptr(new OpenTiledBitmapAsPresentation(std::move(openTiledBitmapInterface_)));
    }

    // OpenPresentationInterface
    std::list<GtkFileFilter*> getFilters() override { return openTiledBitmapInterface->getFilters(); }

    PresentationInterface::Ptr open(const std::string& fileName) override;

  private:
    explicit OpenTiledBitmapAsPresentation(OpenTiledBitmapInterface::Ptr openTiledBitmapInterface_)
      : openTiledBitmapInterface(std::move(openTiledBitmapInterface_))
    {}
  };

  PresentationInterface::Ptr OpenTiledBitmapAsPresentation::open(const std::string& fileName)
  {
    auto           t     = openTiledBitmapInterface->open(fileName);
    BitmapMetaData bmd   = std::move(std::get<0>(t));
    Layer::Ptr     layer = std::move(std::get<1>(t));
    ReloadFunction load  = std::move(std::get<2>(t));

    // Todo... add stuff...
    return PresentationInterface::Ptr();
  }
} // namespace

namespace Scroom
{
  namespace TiledBitmap
  {
    OpenPresentationInterface::Ptr ToOpenPresentationInterface(OpenTiledBitmapInterface::Ptr openTiledBitmapInterface)
    {
      return OpenTiledBitmapAsPresentation::create(std::move(openTiledBitmapInterface));
    }
  } // namespace TiledBitmap
} // namespace Scroom
