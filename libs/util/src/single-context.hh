/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <map>

#include <scroom/context.hh>

namespace Scroom::Utils
{

  class SingleContext : public Context
  {
  public:
    void                          set(std::string name, std::any value) override;
    [[nodiscard]] const std::any& get(std::string name) const override;
    [[nodiscard]] std::any        try_get(std::string name) const override;

  private:
    std::map<std::string, std::any> content;
  };

} // namespace Scroom::Utils
