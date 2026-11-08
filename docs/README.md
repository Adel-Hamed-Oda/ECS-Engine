# ECS Engine Documentation

Welcome to the documentation for the **ECS Engine**, a lightweight, modular C++20 game engine built around a high-performance **Entity Component System (ECS)** architecture, hardware-accelerated OpenGL rendering, and multi-window support via GLFW.

---

## Architecture Overview

The engine is structured into clear, user-facing subsystems designed to separate data, logic, and rendering:

```
engine/
├── Engine.h / Engine.cpp        # Engine lifecycle, execution loop, and headless control
├── core/
│   ├── window/
│   │   └── Window.h             # Window management, OpenGL contexts, and event callbacks
│   ├── ecs/
│   │   ├── Entity.h             # 32-bit generational entity handles
│   │   ├── Component.h          # Component IDs and bitset signatures
│   │   ├── ComponentPool.h      # Cache-friendly Sparse-Set component storage
│   │   ├── Registry.h           # Central ECS coordinator and entity query system
│   │   └── System.h             # Base system logic matching component signatures
│   └── scenes/
│       └── Scene.h              # Scene structure and lifecycle callbacks
└── docs/                        # Engine documentation (you are here)
```

> **Note**: Internal helper headers such as `private_imports.h` and third-party dependencies in `external/` (GLFW, GLAD, GLM, FreeType) are intentionally encapsulated and excluded from these guides.

---

## Documentation Sections

| Section | Description |
| :--- | :--- |
| **[Engine Lifecycle](engine-lifecycle.md)** | Initializing, running, and terminating the engine. Headless mode and loop throttling. |
| **[Window Management](window.md)** | Creating and configuring windows, handling resolutions, OpenGL contexts, and events. |
| **[Entity Component System (ECS)](ecs.md)** | Entities, components, sparse-set pools, the registry coordinator, and queries. |
| **[Scenes](scenes.md)** | Creating scenes, entity instantiation, lifecycle hooks (`Create`, `Update`, `Render`). |

---

## Quick Start Example

Here is a minimal example demonstrating how to set up the engine, create a window, and run:

```cpp
#include <Engine.h>

int main() {
    // 1. Create a primary game window (Title, Position, Size)
    Window window("My ECS Game", glm::ivec2(100, 100), glm::ivec2(1280, 720));

    // 2. Start the engine main loop
    Engine::run();

    return 0;
}
```

To create an entity and attach data via the ECS:

```cpp
#include "core/ecs/Registry.h"
#include <glm/glm.hpp>

// Define simple component structs
struct Position {
    glm::vec2 value;
};

struct Velocity {
    glm::vec2 value;
};

void GameSetup(Registry& registry) {
    // Create an entity
    Entity player = registry.CreateEntity();

    // Attach components
    registry.AddComponent<Position>(player, glm::vec2(0.0f, 0.0f));
    registry.AddComponent<Velocity>(player, glm::vec2(1.5f, 0.0f));

    // Query and update all entities that have both Position and Velocity
    registry.ForEach<Position, Velocity>([&](Entity e) {
        auto& pos = registry.GetComponent<Position>(e);
        auto& vel = registry.GetComponent<Velocity>(e);
        pos.value += vel.value;
    });
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
