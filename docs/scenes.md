# Scenes

The `Scene` class ([core/scenes/Scene.h](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/scenes/Scene.h)) organizes game state, entity lifetimes, and gameplay logic into discrete, modular levels or views.

---

## Class Overview

```cpp
class Scene {
public:
    Scene(const std::string& name = "");
    virtual ~Scene();

    virtual void Create();
    virtual void Update();
    virtual void Render();

    Entity Instantiate();
    void Destroy(Entity e);

    template <typename T, typename... Args>
    Entity AddComponent(Entity e, Args&&... args);

    template <typename T>
    T& GetComponent(Entity e);

    template <typename T>
    void RemoveComponent(Entity e);
};
```

---

## Lifecycle Callbacks

### `Create()`
```cpp
virtual void Create();
```
Called when the scene is loaded. Override this method to spawn entities, construct level layout, attach components, and initialize systems.

### `Update()`
```cpp
virtual void Update();
```
Called every frame during the main loop while the scene is active. Handles gameplay logic, timers, physics stepping, and input processing.

### `Render()`
```cpp
virtual void Render();
```
Called during the render phase. Renders visible meshes, sprites, particles, and UI elements.

---

## Entity Management in Scenes

Scenes provide convenience wrappers around the engine's `Registry`:

| Method | Description |
| :--- | :--- |
| `Instantiate()` | Spawns a new `Entity` within the scene registry. |
| `Destroy(Entity e)` | Removes the entity and destroys all associated components. |
| `AddComponent<T>(Entity, Args...)` | Constructs and attaches component `T` to the entity. |
| `GetComponent<T>(Entity)` | Retrieves a reference to component `T` on the entity. |
| `RemoveComponent<T>(Entity)` | Removes component `T` from the entity. |

---

## Example: Creating a Custom Scene

```cpp
#include "core/scenes/Scene.h"
#include <glm/glm.hpp>

struct PlayerTag {};
struct Transform { glm::vec3 pos; };
struct Sprite { std::string texture; };

class MainMenuScene : public Scene {
public:
    MainMenuScene() : Scene("MainMenu") {}

    void Create() override {
        // Spawn title text entity
        Entity title = Instantiate();
        AddComponent<Transform>(title, glm::vec3(400.0f, 300.0f, 0.0f));
        AddComponent<Sprite>(title, "title_logo.png");

        // Spawn player preview entity
        Entity player = Instantiate();
        AddComponent<PlayerTag>(player);
        AddComponent<Transform>(player, glm::vec3(400.0f, 150.0f, 0.0f));
    }

    void Update() override {
        // Handle input and animations
    }

    void Render() override {
        // Render menu entities
    }
};
```

---

## Automatic Cleanup

When a `Scene` is destroyed (for example, when transitioning to another level or unloading):
- Its destructor automatically iterates through its entities and calls `registry.DestroyEntity(...)`.
- The scene unregisters itself from the active scenes vector.
- Memory from all associated component pools is freed.
