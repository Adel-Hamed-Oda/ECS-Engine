#pragma once

#include "core/private_imports.h"

template <typename... Args>
struct Action
{
    using Callback = std::function<void(Args...)>; // bro wtf is this syntax
    
    std::vector<Callback> listeners;

    Action() = default;
    Action(std::nullptr_t) {}

    Action(Callback callback)
    {
        if (callback)
            listeners.push_back(std::move(callback));
    }

    Action& operator=(std::nullptr_t)
    {
        listeners.clear();
        return *this;
    }

    Action& operator=(Callback callback)
    {
        listeners.clear();
        if (callback)
            listeners.push_back(std::move(callback));
        return *this;
    }

    Action& operator+=(Callback callback)
    {
        if (callback)
            listeners.push_back(std::move(callback));
        return *this;
    }

    void AddListener(Callback callback)
    {
        if (callback)
            listeners.push_back(std::move(callback));
    }

    template <typename Func, typename Arg, typename... BoundArgs>
    void AddListener(Func&& func, Arg&& arg, BoundArgs&&... boundArgs)
    {
        if constexpr (std::invocable<std::decay_t<Func>, std::decay_t<Arg>, std::decay_t<BoundArgs>..., Args...>)
        {
            listeners.emplace_back(
                [f = std::forward<Func>(func),
                 firstArg = std::forward<Arg>(arg),
                 ...restArgs = std::forward<BoundArgs>(boundArgs)](Args... ActionArgs) mutable
                {
                    std::invoke(f, firstArg, restArgs..., ActionArgs...);
                }
            );
        }
        else
        {
            listeners.emplace_back(
                std::bind(
                    std::forward<Func>(func),
                    std::forward<Arg>(arg),
                    std::forward<BoundArgs>(boundArgs)...
                )
            );
        }
    }

    void Clear()
    {
        listeners.clear();
    }

    void Invoke(Args... args) const
    {
        auto currentListeners = listeners;
        for (const auto& listener : currentListeners)
        {
            if (listener)
                listener(args...);
        }
    }

    void operator()(Args... args) const
    {
        Invoke(args...);
    }

    explicit operator bool() const
    {
        return !listeners.empty();
    }
};

Action() -> Action<>;
Action(std::nullptr_t) -> Action<>;
template <typename... Args>
Action(std::function<void(Args...)>) -> Action<Args...>;
