/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef ROI_HH_
#define ROI_HH_

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
    };

    struct Rect : public RoiBase
    {
      double left;
      double top;
      double width;
      double height;
    };

    std::ostream& operator<<(std::ostream& os, RoiBase const& b);
    std::ostream& operator<<(std::ostream& os, Rect const& r);

    class List
    {
    public:
      std::vector<Detail::Presentation> presentations;
      std::vector<RoiItem> regions;

      std::set<ViewObservable::Ptr> instantiate(ScroomInterface::Ptr const& scroomInterface, std::string const& relativeTo=std::string());


    };

    List parse(std::stringstream const& s);
    List parse(std::string const& filename);
    List parse(std::string::const_iterator first, std::string::const_iterator last);
  }
}



#endif /* ROI_HH_ */
