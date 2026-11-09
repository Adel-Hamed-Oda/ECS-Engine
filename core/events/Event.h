#pragma once

#include "core/private_imports.h"

template <typename... Args>
struct Event
{
    using Callback = std::function<void(Args...)>; // bro wtf is this syntax
    // I have to rant about this, in C# you don't need all this shit, you literally put <T>
    // next to any function, class, etc. and then you just use them.
    //
    // HELL, you can even do something like "T where T : ISomeInterface" it's insane
    // also wtf are "friend class" bruh.

    std::vector<Callback> listeners;

    Event() = default;
    Event(std::nullptr_t) {}

    Event(Callback callback)
    {
        if (callback)
            listeners.push_back(std::move(callback));
    }

    Event& operator=(std::nullptr_t)
    {
        listeners.clear();
        return *this;
    }

    Event& operator=(Callback callback)
    {
        listeners.clear();
        if (callback)
            listeners.push_back(std::move(callback));
        return *this;
    }

    Event& operator+=(Callback callback)
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
                 ...restArgs = std::forward<BoundArgs>(boundArgs)](Args... eventArgs) mutable
                {
                    std::invoke(f, firstArg, restArgs..., eventArgs...);
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

Event() -> Event<>;
Event(std::nullptr_t) -> Event<>;
template <typename... Args>
Event(std::function<void(Args...)>) -> Event<Args...>;
