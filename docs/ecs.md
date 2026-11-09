# Entity Component System (ECS)

The engine features a high-performance, cache-friendly **Entity Component System (ECS)** implementing a **Sparse-Set architecture**. 

In this paradigm:
- **Entities** are lightweight 32-bit generational numeric handles.
- **Components** are pure data structs without logic.
- **ComponentPools** store components contiguously in memory for maximum CPU cache locality.
- **Registry** coordinates entities, components, signatures, and queries.
- **Systems** encapsulate logic and iterate over entities with specific component sets.

---

## 1. Entities (`core/ecs/Entity.h`)

An [`Entity`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/ecs/Entity.h) is a packed 32-bit integer:
```cpp
using Entity = uint32_t;
constexpr Entity NULL_ENTITY = 0xFFFFFFFF;
```

### Generational Indexing
To prevent "dangling entity" bugs (where an entity is destroyed and another is created with the same index, causing stale handles to modify unrelated objects), the 32 bits are partitioned:
- **Lower 20 bits**: Index (supports up to ~1,048,576 concurrent entities).
- **Upper 12 bits**: Generation counter (allows up to 4,096 reuses of an index before rollover).

```cpp
uint32_t index = GetEntityIndex(entity);      // Lower 20 bits
uint32_t gen   = GetEntityGeneration(entity); // Upper 12 bits
Entity e       = CreateEntityID(index, gen);
```

---

## 2. Components (`core/ecs/Component.h`)

Components are plain C++ structs containing purely data:

```cpp
struct Transform {
    glm::vec2 position;
    float rotation = 0.0f;
    glm::vec2 scale = glm::vec2(1.0f);
};

struct RigidBody {
    glm::vec2 velocity;
    float mass = 1.0f;
};
```

### Component Identification & Signatures
- **`ComponentTypeID`**: A unique `size_t` assigned automatically per component type via [`GetComponentTypeID<T>()`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/ecs/Component.h).
- **`MAX_COMPONENTS`**: Defines the maximum distinct component types in the registry (default: 64).
- **`ComponentSignature`**: A `std::bitset<MAX_COMPONENTS>` representing which components an entity owns.

---

## 3. Sparse-Set Component Pool (`core/ecs/ComponentPool.h`)

Each component type has a dedicated [`ComponentPool<T>`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/ecs/ComponentPool.h) implementing the Sparse-Set pattern:

```
Sparse Array (Indexed by EntityIndex):
[ Entity 0: index 1 ] [ Entity 1: -1 ] [ Entity 2: index 0 ] ...

Dense Array (Contiguous memory for fast iteration):
[ Index 0: ComponentData for Entity 2 ]
[ Index 1: ComponentData for Entity 0 ]
```

### Performance Characteristics:
- **O(1) Insertion**: New components are appended to the dense buffer.
- **O(1) Removal**: Uses swap-and-pop (replaces the removed component with the last element in the dense array and updates sparse indices).
- **O(1) Lookup**: Checking or accessing component data by entity index is an instant direct memory read.
- **Cache-Friendly Iteration**: Processing components linearly iterates through a contiguous memory buffer (`dense`), maximizing L1/L2 cache hits.

Raw dense buffer access:
```cpp
std::vector<T>& denseData = pool->GetDenseArray();
const std::vector<Entity>& denseEntities = pool->GetDenseEntities();
```

---

## 4. The Registry (`core/ecs/Registry.h`)

The [`Registry`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/ecs/Registry.h) is the central coordinator managing entities and their components.

### Managing Entities
```cpp
Registry registry;

// Create a new entity with generational tracking
Entity e = registry.CreateEntity();

// Check if an entity handle is alive and valid
bool alive = registry.IsAlive(e);

// Destroy a single entity and remove all its attached components
registry.DestroyEntity(e);

// Destroy all active entities in the registry
registry.DestroyAllEntities();
```

### Managing Components on an Entity
```cpp
// Add a component (in-place constructed, returns reference to T)
Transform& tf = registry.AddComponent<Transform>(e, glm::vec2(100.0f, 200.0f));

// Check if an entity possesses a component
if (registry.HasComponent<Transform>(e)) {
    // Retrieve reference to component
    Transform& t = registry.GetComponent<Transform>(e);
    t.position.x += 10.0f;
}

// Remove a component
registry.RemoveComponent<Transform>(e);
```

### Querying Entities (`ForEach`)

The `ForEach` query automatically matches entities against requested component types.

#### 1. Invocable with Component References (Recommended)
When the lambda accepts `(Entity, Comps&...)`, the registry automatically unpacks the component references directly:
```cpp
registry.ForEach<Transform, RigidBody>([](Entity e, Transform& tf, RigidBody& rb) {
    tf.position += rb.velocity * static_cast<float>(Time::deltaTime);
});
```

#### 2. Entity-Only Query
If you only need the entity handle:
```cpp
registry.ForEach<Transform, RigidBody>([](Entity e) {
    std::cout << "Entity matching query: " << e << std::endl;
});
```

#### 3. Query via Custom Bitmask
```cpp
ComponentSignature mask;
mask.set(GetComponentTypeID<Transform>());
registry.ForEach([](Entity e) {
    // Process entity
}, mask);
```

---

## 5. Systems Architecture (`core/ecs/System.h`)

Systems encapsulate logic and process entities matching component signatures.

### Base Interface: `ISystem`
```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void apply(Registry& registry) = 0;
};
```

### System Types
```cpp
enum class SystemType {
    Start = 0,
    Update = 1
};
```

### Templated `System<Comps...>`
The engine provides a ready-to-use templated [`System<Comps...>`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/ecs/System.h) class that wraps a callback lambda:

```cpp
template <typename... Comps>
class System : public ISystem {
public:
    using SystemFunc = std::function<void(Entity, Comps&...)>;

    System(SystemFunc func);
    void apply(Registry& registry) override;
};
```

### Using Systems in a Scene
Systems are typically registered directly into scenes:

```cpp
// Register an update system in a scene
scene.AddSystem<System<Transform, RigidBody>>(
    SystemType::Update,
    [](Entity e, Transform& tf, RigidBody& rb) {
        tf.position += rb.velocity * static_cast<float>(Time::deltaTime);
    }
);
```

Or executed directly against any `Registry`:
```cpp
System<Transform, RigidBody> movementSystem([](Entity e, Transform& tf, RigidBody& rb) {
    tf.position += rb.velocity * 0.016f;
});

movementSystem.apply(registry);
```
