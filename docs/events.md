# Events & Delegates

The engine provides a type-safe delegate and event mechanism via [`Action`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/events/Action.h) and [`Event`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/events/Event.h).

---

## Overview: `Action` vs `Event`

| Feature | `Action<Args...>` | `Event<Owner, Args...>` |
| :--- | :--- | :--- |
| **Concept** | Public multicast delegate | Encapsulated C#-style event |
| **Who can subscribe?** | Anyone (`+=`, `AddListener`) | Anyone (`+=`, `AddListener`) |
| **Who can invoke?** | Anyone (`Invoke()`, `operator()`) | **Only `Owner` class** (`Invoke()`, `operator()`) |
| **Who can clear/reassign?** | Anyone (`Clear()`, `=`) | **Only `Owner` class** (`Clear()`, `=`) |
| **Best suited for** | General-purpose callbacks and functions | Public class events (e.g. `onWindowClose`, `onLoad`) |

---

## 1. Action (`core/events/Action.h`)

[`Action<Args...>`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/events/Action.h) is an open multicast delegate. It holds multiple listeners and allows any caller to subscribe, invoke, or clear.

### API Reference

```cpp
template <typename... Args>
struct Action {
    using Callback = std::function<void(Args...)>;

    Action();
    Action(Callback callback);

    Action& operator=(Callback callback);
    Action& operator=(std::nullptr_t);
    Action& operator+=(Callback callback);

    void AddListener(Callback callback);

    template <typename Func, typename Arg, typename... BoundArgs>
    void AddListener(Func&& func, Arg&& arg, BoundArgs&&... boundArgs);

    void Clear();
    void Invoke(Args... args) const;
    void operator()(Args... args) const;

    explicit operator bool() const;
};
```

### Usage Examples

#### Subscribing with Lambdas
```cpp
Action<int, float> onScoreChanged;

// Subscribe via operator+=
onScoreChanged += [](int score, float multiplier) {
    std::cout << "Score: " << score << " (x" << multiplier << ")" << std::endl;
};

// Invoke
onScoreChanged.Invoke(100, 1.5f);
// Or call directly like a function
onScoreChanged(200, 2.0f);
```

#### Binding Member Functions
`AddListener` supports passing member functions along with object instances and bound arguments:
```cpp
class ScoreUI {
public:
    void UpdateDisplay(int score, float multiplier) {
        std::cout << "UI updated: " << score << std::endl;
    }
};

ScoreUI ui;
Action<int, float> onScoreChanged;

// Automatically binds member function to object reference
onScoreChanged.AddListener(&ScoreUI::UpdateDisplay, &ui);

onScoreChanged(500, 1.0f);
```

---

## 2. Event (`core/events/Event.h`)

[`Event<Owner, Args...>`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/events/Event.h) enforces encapsulation using the template parameter `Owner` and `friend Owner;`.

External consumers can **only** subscribe or inspect:
- `operator+=(callback)`
- `AddListener(callback)`
- `AddListener(func, instance, ...)`
- `operator bool()`

Only the `Owner` class has permission to call:
- `Invoke(args...)` or `operator()(args...)`
- `Clear()`
- Reassignment via `operator=`

This prevents outside callers from accidentally broadcasting another class's internal state or clearing other listeners.

### API Reference

```cpp
template <typename Owner, typename... Args>
class Event {
    friend Owner;

public:
    using Callback = std::function<void(Args...)>;

    Event() = default;

    Event& operator+=(Callback callback);
    void AddListener(Callback callback);

    template <typename Func, typename Arg, typename... BoundArgs>
    void AddListener(Func&& func, Arg&& arg, BoundArgs&&... boundArgs);

    explicit operator bool() const;

private:
    // Only Owner can access these:
    void Invoke(Args... args) const;
    void operator()(Args... args) const;
    void Clear();
    Event& operator=(Callback callback);
    Event& operator=(std::nullptr_t);
};
```

---

## Built-In Engine Events

The engine uses `Event<Owner, ...>` across its core classes:

| Class | Event Member | Type | Triggered When |
| :--- | :--- | :--- | :--- |
| [`Window`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/window/Window.h) | `onWindowClose` | `Event<Window>` | The window is closed via `Close()` or window destruction. |
| [`Scene`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/scenes/Scene.h) | `onLoad` | `Event<Scene>` | The scene is loaded via `Scene::Load()`. |
| [`Scene`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/scenes/Scene.h) | `onUnload` | `Event<Scene>` | The scene is unloaded via `Scene::Unload()`. |

---

## Custom Event Example

Creating a class with a custom encapsulated event:

```cpp
#include <Engine.h>
#include <iostream>

class HealthComponent {
public:
    // Only HealthComponent can fire or clear this event
    Event<HealthComponent, int /* currentHealth */> onHealthChanged;
    Event<HealthComponent> onDied;

    void TakeDamage(int damage) {
        m_health = std::max(0, m_health - damage);
        onHealthChanged.Invoke(m_health); // Allowed: HealthComponent is Owner

        if (m_health == 0) {
            onDied.Invoke();
        }
    }

private:
    int m_health = 100;
};

int main() {
    HealthComponent playerHealth;

    // External code subscribes
    playerHealth.onHealthChanged += [](int hp) {
        std::cout << "Player HP: " << hp << std::endl;
    };

    playerHealth.onDied += []() {
        std::cout << "Player has perished!" << std::endl;
    };

    // playerHealth.onHealthChanged.Invoke(50); // Compile error! Only HealthComponent can invoke.

    playerHealth.TakeDamage(30);
    playerHealth.TakeDamage(70);

    return 0;
}
```
