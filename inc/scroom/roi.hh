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
    }
    
    class List
    {
      std::vector<Detail::Presentation> presentations;
      
    public:
      List(std::vector<Detail::Presentation> const& presentations);
      
      std::set<ViewObservable::Ptr> instantiate(ScroomInterface::Ptr const& scroomInterface);


    };
  }
}



#endif /* ROI_HH_ */
