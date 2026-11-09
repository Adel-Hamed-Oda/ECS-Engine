# ECS Engine Documentation

Welcome to the documentation for the **ECS Engine**, a lightweight, modular C++20 game engine built around a high-performance **Entity Component System (ECS)** architecture, hardware-accelerated OpenGL rendering, multi-window support via GLFW, and integrated event, timer, and tweening systems.

---

## Architecture Overview

The engine is structured into clear, user-facing subsystems designed to separate data, logic, and rendering:

```
engine/
├── Engine.h / Engine.cpp        # Engine lifecycle, execution loop, and headless control
├── core/
│   ├── window/
│   │   ├── Window.h / .cpp      # Multi-windowing, context sharing, and window close events
│   ├── scenes/
│   │   ├── Scene.h / .cpp       # Scene states, entity lifecycles, and systems pipelines
│   ├── ecs/
│   │   ├── Entity.h             # 32-bit generational entity handles
│   │   ├── Component.h          # Component IDs and bitset signatures
│   │   ├── ComponentPool.h      # Cache-friendly Sparse-Set component storage
│   │   ├── Registry.h           # Central ECS coordinator and entity query system
│   │   └── System.h             # Base ISystem and templated System logic
│   ├── events/
│   │   ├── Action.h             # Public multicast delegates and function binders
│   │   └── Event.h              # Encapsulated owner-restricted C#-style events
│   └── global/
│       └── EngineTime.h         # Frame timing, deltaTime, FPS calculation, time scaling
├── anim/
│   ├── Timer.h                  # Self-managing one-shot & repeating timers
│   └── Tween.h                  # Templated property interpolation with custom easing
└── docs/                        # Engine documentation (you are here)
```

> **Note**: Internal helper headers such as `private_imports.h` and third-party dependencies in `external/` (GLFW, GLAD, GLM, FreeType) are encapsulated and excluded from these guides.

---

## Documentation Sections

| Section | Description |
| :--- | :--- |
| **[Engine Lifecycle](engine-lifecycle.md)** | Initializing, running, and terminating the engine. Headless mode, multi-window frame pacing, and main loop. |
| **[Window Management](window.md)** | Creating and configuring windows, OpenGL context sharing, scene assignment, and window events. |
| **[Scenes](scenes.md)** | Scene state machine (`Unloaded`, `Loaded`, `Active`), lifecycle events (`onLoad`, `onUnload`), and systems pipelines. |
| **[Entity Component System (ECS)](ecs.md)** | Generational entities, components, sparse-set pools, the registry coordinator, and systems. |
| **[Events & Delegates](events.md)** | Type-safe delegates (`Action`) and owner-encapsulated event patterns (`Event`). |
| **[Time, Timers & Tweens](time-and-animation.md)** | Global engine time tracking, delta time scaling, self-deleting timers, and property tweening. |

---

## Quick Start Example

Here is a minimal example demonstrating how to set up windows, attach a scene, register ECS systems, and start timers:

```cpp
#include <Engine.h>
#include <iostream>

// 1. Define ECS components
struct Position {
    glm::vec2 value;
};

struct Velocity {
    glm::vec2 value;
};

// 2. Define a custom scene
class GameScene : public Scene {
public:
    Timer* tickTimer = nullptr;

    GameScene() : Scene("MainScene") {
        onLoad += [this]() {
            std::cout << "[Scene] Loaded. Setting up entities and systems..." << std::endl;

            // Register an ECS update system
            AddSystem<System<Position, Velocity>>(
                SystemType::Update,
                [](Entity e, Position& pos, Velocity& vel) {
                    pos.value += vel.value * static_cast<float>(Time::deltaTime);
                }
            );

            // Instantiate an entity and attach components
            Entity player = Instantiate();
            AddComponent<Position>(player, glm::vec2(100.0f, 100.0f));
            AddComponent<Velocity>(player, glm::vec2(50.0f, 0.0f));

            // Start a repeating 1-second timer
            tickTimer = new Timer(1.0f, true, []() {
                std::cout << "Timer tick: 1 second passed (FPS: " << Time::FPS << ")" << std::endl;
            });
        };

        onUnload += [this]() {
            std::cout << "[Scene] Unloaded. Cleaning up resources..." << std::endl;
            if (tickTimer) {
                delete tickTimer;
                tickTimer = nullptr;
            }
        };
    }

    ~GameScene() override {
        if (tickTimer) {
            delete tickTimer;
            tickTimer = nullptr;
        }
    }
};

int main() {
    // 1. Create a game window
    Window window("ECS Game", glm::ivec2(100, 100), glm::ivec2(1280, 720));

    // 2. Instantiate and attach scene
    GameScene gameScene;
    window.SetScene(&gameScene);

    // 3. Handle window close
    window.onWindowClose += [&gameScene]() {
        gameScene.Unload();
    };

    // 4. Run the engine loop
    Engine::run();

    return 0;
}
```

---

## Building the Engine

The engine uses CMake 3.20+ with C++20 enabled:

```cmake
# In your root CMakeLists.txt
add_subdirectory(engine)
target_link_libraries(MyGame PRIVATE Engine)
```
