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

#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/foreach.hpp>

#include <scroom/roi.hh>
#include <scroom/unused.hh>

namespace Scroom
{
  namespace Roi
  {
    namespace Detail
    {
      std::ostream& operator<<(std::ostream& os, File const& f)
      {
        return os << "[" << f.name << "]";
      }

      std::ostream& operator<<(std::ostream& os, Aggregate const& a)
      {
        os << "[" << a.name << "(";
        BOOST_FOREACH(Presentation const& p, a.children)
          os << p;
        os << ")]";

        return os;
      }

      
      class Instantiate : public boost::static_visitor<PresentationInterface::Ptr>
      {
      private:
        ScroomInterface::Ptr scroomInterface;
        std::string const relativeTo;

      public:
        Instantiate(ScroomInterface::Ptr const& scroomInterface, std::string const& relativeTo);

        PresentationInterface::Ptr operator()(File const& file);
        PresentationInterface::Ptr operator()(Aggregate const& aggregate);
      };

      Instantiate::Instantiate(ScroomInterface::Ptr const& scroomInterface, std::string const& relativeTo) :
          scroomInterface(scroomInterface), relativeTo(relativeTo)
      {
      }

      PresentationInterface::Ptr Instantiate::operator()(File const& file)
      {
        return scroomInterface->loadPresentation(file.name, relativeTo);
      }

      PresentationInterface::Ptr Instantiate::operator()(Aggregate const& aggregate)
      {
        ::Aggregate::Ptr a = scroomInterface->newAggregate(aggregate.name);
        PresentationInterface::Ptr aggregatePresentation =
            boost::dynamic_pointer_cast<PresentationInterface>(a);

        if(!aggregatePresentation)
        {
          std::stringstream message;
          message << "Aggregate " << aggregate.name << " is not a Presentation";
          throw std::invalid_argument(message.str());
        }

        BOOST_FOREACH(Detail::Presentation const& p, aggregate.children)
        {
          a->addPresentation(boost::apply_visitor(*this, p));
        }

        return aggregatePresentation;
      }
    }

    std::set<ViewObservable::Ptr> List::instantiate(ScroomInterface::Ptr const& scroomInterface, std::string const& relativeTo)
    {
      std::set<ViewObservable::Ptr> result;
      std::list<PresentationInterface::Ptr> presentationInterfaces;
      Detail::Instantiate instantiate(scroomInterface, relativeTo);

      for(auto const& p: presentations)
      {
        PresentationInterface::Ptr presentation = boost::apply_visitor(instantiate, p);
        presentationInterfaces.push_back(presentation);
      }

      for(auto const& presentation: presentationInterfaces)
      {
        scroomInterface->showPresentation(presentation);
        result.insert(presentation);
      }

      return result;
    }

    std::ostream& operator<<(std::ostream& os, RoiBase const& b)
    {
      os << "[ \"" << b.description << "\" (";
      for(auto const& rr: b.children)
        os << rr;
      os << ")]";
      
      return os;
    }

    std::ostream& operator<<(std::ostream& os, Rect const& r)
    {
      os << "[ {"
         << r.left << ", "
         << r.top << ", "
         << r.width << ", "
         << r.height
         << "} ";
      if(!r.description.empty())
        os << "\"" << r.description << "\" ";
      os << "(";
      for(auto const& rr: r.children)
        os << rr;
      os << ")]";
      
      return os;
    }

  }
}



