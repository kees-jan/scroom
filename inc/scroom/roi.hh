/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <set>

#include <boost/variant/recursive_variant.hpp>

#include <scroom/scroominterface.hh>

namespace Scroom
{
  namespace Roi
  {
    namespace Detail
    {
      struct File;
      struct Aggregate;

      typedef boost::variant<File, Aggregate> Presentation;

      struct File
      {
        std::string name;
      };

      struct Aggregate
      {
        std::string name;
        std::vector<Presentation> children;
      };

      std::ostream& operator<<(std::ostream& os, File const& f);
      std::ostream& operator<<(std::ostream& os, Aggregate const& a);
    }

    struct RoiBase;
    struct Rect;

    typedef boost::variant<RoiBase, Rect> RoiItem;

    struct RoiBase
    {
      std::string description;
      std::vector<RoiItem> children;

    public:
      virtual ~RoiBase() {};
      virtual std::string str() const;
    };

    struct Rect : public RoiBase
    {
      double left;
      double top;
      double width;
      double height;

    public:
      virtual std::string str() const;
    };

    std::ostream& operator<<(std::ostream& os, RoiBase const& b);
    std::ostream& operator<<(std::ostream& os, Rect const& r);

    class List
    {
    public:
      typedef boost::shared_ptr<List> Ptr;
      typedef boost::shared_ptr<List const> ConstPtr;

    private:
      List();

    public:
      std::vector<Detail::Presentation> presentations;
      std::vector<RoiItem> regions;

    public:
      static Ptr create();
      std::set<ViewObservable::Ptr> instantiate(ScroomInterface::Ptr const& scroomInterface, std::string const& relativeTo=std::string());

    };

    List::Ptr parse(std::stringstream const& s);
    List::Ptr parse(std::string const& filename);
    List::Ptr parse(std::string::const_iterator first, std::string::const_iterator last);
  }
}

