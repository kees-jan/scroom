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

#include "roi-parser.hh"

namespace Scroom
{
  namespace Roi
  {
    namespace Detail
    {
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

    List::List(std::vector<Detail::Presentation> const& presentations)
      : presentations(presentations)
    {}
     
    std::set<ViewObservable::Ptr> List::instantiate(ScroomInterface::Ptr const& scroomInterface, std::string const& relativeTo)
    {
      std::set<ViewObservable::Ptr> result;
      Detail::Instantiate instantiate(scroomInterface, relativeTo);

      BOOST_FOREACH(Detail::Presentation const& p, presentations)
      {
        PresentationInterface::Ptr presentation = boost::apply_visitor(instantiate, p);
        scroomInterface->showPresentation(presentation);
        result.insert(presentation);
      }

      return result;
    }

    List parse(std::stringstream const& s)
    {
      std::string input = s.str();
      return List(Detail::parse(input.begin(), input.end()));
    }

  }
}



