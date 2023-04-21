/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <vector>

#include <scroom/context.hh>
namespace Scroom::Utils
{

  class ParentContext : public RecursiveContext
  {
  public:
    explicit ParentContext(Context::Ptr first_);

    void                          set(std::string name, std::any value) override;
    [[nodiscard]] const std::any& get(std::string name) const override;
    [[nodiscard]] std::any        try_get(std::string name) const override;

    void add(const Context::ConstPtr& child) override;

  private:
    Context::Ptr                   first;
    std::vector<Context::ConstPtr> contexts;
  };

} // namespace Scroom::Utils
