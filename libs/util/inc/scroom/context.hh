/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <any>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include <scroom/interface.hh>

namespace Scroom::Utils
{
  class Context : public Interface
  {
  public:
    using Ptr      = std::shared_ptr<Context>;
    using ConstPtr = std::shared_ptr<const Context>;

    class name_exists : public std::exception
    {
    };

    class name_not_found : public std::exception
    {
    };

    static Ptr Create();

    virtual void                          set(std::string name, std::any value) = 0;
    [[nodiscard]] virtual const std::any& get(std::string name) const           = 0;
    [[nodiscard]] virtual std::any        try_get(std::string name) const       = 0;
  };

  class RecursiveContext : public Context
  {
  public:
    using Ptr = std::shared_ptr<RecursiveContext>;

    static Ptr Create();

    virtual void add(const Context::ConstPtr& child) = 0;
  };

  template <typename T>
  [[nodiscard]] T get(const Context::ConstPtr& context, std::string name)
  {
    const std::any& result = context->get(std::move(name));
    try
    {
      return std::any_cast<T>(result);
    }
    catch(std::bad_any_cast&)
    {
      using F = std::function<T()>;
      return std::any_cast<F>(result)();
    }
  }

  template <typename T>
  [[nodiscard]] T get(const Context::ConstPtr& context)
  {
    return get<T>(context, typeid(T).name());
  }

  template <typename T, typename Callable>
  [[nodiscard]] auto get_or(const Context::ConstPtr& context, std::string name, Callable default_value)
    -> std::enable_if_t<std::is_same_v<T, std::remove_reference_t<decltype(default_value())>>, T>
  {
    try
    {
      return get<T>(context, std::move(name));
    }
    catch(...)
    {
      return default_value();
    }
  }

  template <typename T, typename Callable>
  [[nodiscard]] auto get_or(const Context::ConstPtr& context, Callable default_value)
    -> std::enable_if_t<std::is_same_v<T, std::remove_reference_t<decltype(default_value())>>, T>
  {
    return get_or<T>(context, typeid(T).name(), default_value);
  }

  template <typename T>
  [[nodiscard]] T get_or(const Context::ConstPtr& context, std::string name, T default_value)
  {
    return get_or<T>(context, std::move(name), [default_value] { return default_value; });
  }

  template <typename T>
  [[nodiscard]] T get_or(const Context::ConstPtr& context, T default_value)
  {
    return get_or<T>(context, typeid(T).name(), [default_value] { return default_value; });
  }

  template <typename T>
  void
    set(const Context::Ptr& context, std::string name, T value) // NOLINT(performance-unnecessary-value-param) // False positive?
  {
    context->set(std::move(name), value);
  }

  template <typename T>
  void set(const Context::Ptr& context, T value)
  {
    set(context, typeid(value).name(), std::move(value));
  }

  template <typename Callable>
  void setFactory(const Context::Ptr& context, std::string name, Callable value)
  {
    using T = std::remove_reference_t<decltype(value())>;
    set(context, std::move(name), std::function<T()>(value));
  }

  template <typename Callable>
  void setFactory(const Context::Ptr& context, Callable value)
  {
    using T = std::remove_reference_t<decltype(value())>;
    setFactory(context, typeid(T).name(), std::move(value));
  }

} // namespace Scroom::Utils
