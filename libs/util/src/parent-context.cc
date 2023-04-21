//
// Created by kees-jan on 2-10-22.
//

#include "parent-context.hh"

#include <scroom/assertions.hh>

namespace Scroom::Utils
{
  ParentContext::ParentContext(Context::Ptr first_)
    : first(std::move(first_))
  {
    require(this->first);
    contexts.emplace_back(this->first);
  }

  void ParentContext::add(const Context::ConstPtr& child)
  {
    require(child);
    contexts.emplace_back(std::move(child));
  }

  void ParentContext::set(std::string name, std::any value) { first->set(std::move(name), std::move(value)); }

  const std::any& ParentContext::get(std::string name) const
  {
    for(const auto& context: contexts)
    {
      try
      {
        return context->get(name);
      }
      catch(name_not_found&)
      {
        // Try the next one
      }
    }

    throw name_not_found();
  }

  std::any ParentContext::try_get(std::string name) const
  {
    std::any result;

    for(const auto& context: contexts)
    {
      result = context->try_get(name);
      if(result.has_value())
      {
        break;
      }
    }

    return result;
  }


} // namespace Scroom::Utils