# Scenes

The `Scene` class ([core/scenes/Scene.h](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/scenes/Scene.h), [core/scenes/Scene.cpp](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/scenes/Scene.cpp)) encapsulates world state, entity lifetimes, component registries, and system pipelines into self-contained, modular runtime levels.

---

## Scene State Machine

Every scene operates within three distinct lifecycle states defined by [`SceneState`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/scenes/Scene.h#L17-L21):

```cpp
enum class SceneState {
    Unloaded, // Inactive; no running entities or systems
    Loaded,   // Loaded into memory; entities and systems populated, not yet updating
    Active    // Actively running; systems updated and rendered each frame
};
```

---

## Class Overview

```cpp
class Scene {
public:
    Event<Scene> onLoad;
    Event<Scene> onUnload;

    static Scene* Get(const std::string& name);
    static const std::vector<Scene*>& GetAllScenes();

    Scene(const std::string& name = "");
    virtual ~Scene();

    void Load();
    void Unload();

    void Update();
    void Render();

    const std::string& GetName() const;
    const SceneState GetState() const;
    Registry& GetRegistry();

    // Entity & Component helpers
    Entity Instantiate();
    void Destroy(Entity e);

    template <typename T, typename... Args>
    T& AddComponent(Entity e, Args&&... args);

    template <typename T>
    T& GetComponent(Entity e);

    template <typename T>
    bool HasComponent(Entity e) const;

    template <typename T>
    void RemoveComponent(Entity e);

    // Systems
    template <typename T, typename... Args>
    T* AddSystem(SystemType type, Args&&... args);

    void DeleteSystem(ISystem* system);
};
```

---

## Lifecycle & Events

### Lifecycle Methods

- **`Load()`**:
  - Checks if the scene is in `SceneState::Unloaded`.
  - Invokes the `onLoad` event.
  - Transitions `SceneState` to `Loaded`.
  - Automatically called by the engine before running `Update()` or `Render()` if the scene has not yet been loaded.
- **`Unload()`**:
  - If already `SceneState::Unloaded`, returns immediately.
  - Invokes the `onUnload` event (giving user code time to detach listeners, clean up timers, or serialize state).
  - Destroys all entities and components in the underlying [`Registry`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/ecs/Registry.h).
  - Clears and deallocates all registered systems across all `SystemType` categories.
  - Transitions `SceneState` back to `Unloaded`.
- **`Update()`**:
  - Invoked during the engine's update phase for every scene attached to an open window (or all scenes in headless mode).
  - Iterates over all systems registered under `SystemType::Update` and applies them to the scene's `Registry`.
  - Transitions `SceneState` to `Active`.
- **`Render()`**:
  - Invoked during the engine's render phase when the owning window sets its viewport.

### Lifecycle Events

| Event | Type | Description |
| :--- | :--- | :--- |
| `onLoad` | `Event<Scene>` | Fired when `Load()` is called. Perfect for spawning entities, registering systems, or starting scene-scoped timers. |
| `onUnload` | `Event<Scene>` | Fired when `Unload()` is called before entities or systems are cleared. Ideal for stopping timers or saving progress. |

```cpp
Scene myScene("Level1");

myScene.onLoad += [&]() {
    std::cout << "Level1 loaded! Spawning entities..." << std::endl;
};

myScene.onUnload += [&]() {
    std::cout << "Level1 unloading! Releasing resources..." << std::endl;
};
```

---

## Systems Pipeline

Scenes manage systems partitioned by [`SystemType`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/ecs/System.h):

```cpp
enum class SystemType {
    Start = 0,
    Update = 1
};
```

### Adding & Removing Systems
```cpp
// Add a system that processes entities with Position and Velocity components
scene.AddSystem<System<Position, Velocity>>(
    SystemType::Update,
    [](Entity e, Position& pos, Velocity& vel) {
        pos.x += vel.dx * Time::deltaTime;
        pos.y += vel.dy * Time::deltaTime;
    }
);
```

| Method | Description |
| :--- | :--- |
| `AddSystem<T>(SystemType type, Args&&... args)` | Instantiates system `T` with forwarded constructor arguments and registers it under `type`. Returns a raw pointer `T*`. |
| `DeleteSystem(ISystem* system)` | Searches system lists across all `SystemType` categories and removes matching system pointers. |

---

## Entity & Component Convenience Methods

The `Scene` class wraps its internal `Registry` instance with easy-to-use helper functions:

| Method | Return Type | Description |
| :--- | :--- | :--- |
| `Instantiate()` | `Entity` | Allocates a new entity handle via `registry.CreateEntity()`. |
| `Destroy(Entity e)` | `void` | Destroys an entity and all its attached components. |
| `AddComponent<T>(Entity e, Args&&...)` | `T&` | Constructs component `T` in-place and returns a reference to it. |
| `GetComponent<T>(Entity e)` | `T&` | Retrieves a reference to component `T` on entity `e`. |
| `HasComponent<T>(Entity e)` | `bool` | Checks whether entity `e` currently possesses component `T`. |
| `RemoveComponent<T>(Entity e)` | `void` | Deletes component `T` from entity `e`. |
| `GetRegistry()` | `Registry&` | Direct access to the underlying ECS registry for custom queries or batch iterations. |

---

## Scene Retrieval & Tracking

- **`Scene::Get(const std::string& name)`**: Looks up an instantiated scene by name. Returns `nullptr` if not found.
- **`Scene::GetAllScenes()`**: Returns a const reference to `std::vector<Scene*>` containing all existing scene instances.
- **`GetName()`**: Returns the name string provided in the scene constructor.
- **`GetState()`**: Returns the current `SceneState` (`Unloaded`, `Loaded`, `Active`).

---

## Complete Scene Example

```cpp
#include <Engine.h>
#include <iostream>

struct Transform {
    glm::vec2 position;
};

struct Movement {
    glm::vec2 velocity;
};

class GameScene : public Scene {
public:
    Timer* gameTimer = nullptr;

    GameScene() : Scene("GameLevel") {
        onLoad += [this]() {
            // 1. Register gameplay systems
            AddSystem<System<Transform, Movement>>(
                SystemType::Update,
                [](Entity e, Transform& tf, Movement& mov) {
                    tf.position += mov.velocity * static_cast<float>(Time::deltaTime);
                }
            );

            // 2. Instantiate player entity
            Entity player = Instantiate();
            AddComponent<Transform>(player, glm::vec2(0.0f, 0.0f));
            AddComponent<Movement>(player, glm::vec2(100.0f, 50.0f));

            // 3. Start a repeating 1-second timer
            gameTimer = new Timer(1.0f, true, []() {
                std::cout << "Timer tick: 1 second passed!" << std::endl;
            });
        };

        onUnload += [this]() {
            if (gameTimer) {
                delete gameTimer;
                gameTimer = nullptr;
            }
        };
    }

    ~GameScene() override {
        if (gameTimer) {
            delete gameTimer;
            gameTimer = nullptr;
        }
    }
};
```
