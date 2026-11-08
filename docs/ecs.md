# Entity Component System (ECS)

The engine features a high-performance, cache-friendly **Entity Component System (ECS)** implementing a **Sparse-Set architecture**. 

In this paradigm:
- **Entities** are lightweight numeric identifiers.
- **Components** are pure data structs without logic.
- **ComponentPools** store components contiguously in memory for maximum CPU cache locality.
- **Registry** coordinates entities, components, and queries.
- **Systems** encapsulate logic and iterate over entities with specific component sets.

---

## 1. Entities (`core/ecs/Entity.h`)

An `Entity` is a packed 32-bit integer:
```cpp
using Entity = uint32_t;
constexpr Entity NULL_ENTITY = 0xFFFFFFFF;
```

### Generational Indexing
To prevent "dangling entity" bugs (where an entity is destroyed and another entity is created with the same index, causing stale handles to modify wrong entities), the 32 bits are split:
- **Lower 20 bits**: Index (supports up to ~1,048,576 concurrent entities).
- **Upper 12 bits**: Generation counter (allows up to 4,096 reuses of an index before rollover).

```cpp
uint32_t index = GetEntityIndex(entity);
uint32_t gen   = GetEntityGeneration(entity);
```

---

## 2. Components (`core/ecs/Component.h`)

Components are plain C++ structs containing data:

```cpp
struct Transform {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct RigidBody {
    glm::vec3 velocity;
    float mass;
    bool useGravity;
};
```

### Component Identification
- **`ComponentTypeID`**: A unique `size_t` assigned automatically per component type via `GetComponentTypeID<T>()`.
- **`MAX_COMPONENTS`**: Defines the maximum distinct component types in the registry (default: 64).
- **`ComponentSignature`**: A `std::bitset<MAX_COMPONENTS>` representing which components an entity owns.

---

## 3. Sparse-Set Component Pool (`core/ecs/ComponentPool.h`)

Each component type has a dedicated `ComponentPool<T>` implementing the Sparse-Set pattern:

```
Sparse Array (Indexed by EntityID index):
[ Entity 0: index 1 ] [ Entity 1: -1 ] [ Entity 2: index 0 ] ...

Dense Array (Contiguous memory for fast iteration):
[ Index 0: ComponentData for Entity 2 ]
[ Index 1: ComponentData for Entity 0 ]
```

### Performance Benefits:
- **O(1) Insertion**: New components are pushed to the end of the dense array.
- **O(1) Removal**: Uses swap-and-pop (replaces the removed component with the last element in the dense array).
- **O(1) Lookup**: Checking or getting component data by entity index is an instant direct memory read.
- **Cache-Optimized Iteration**: Processing components linearly iterates through a flat, contiguous vector (`dense`), avoiding pointer chasing.

Accessing raw dense buffers for rendering/physics batches:
```cpp
std::vector<T>& denseData = pool->GetDenseArray();
const std::vector<Entity>& denseEntities = pool->GetDenseEntities();
```

---

## 4. The Registry (`core/ecs/Registry.h`)

The `Registry` is the central coordinator managing entities and their components.

### Managing Entities
```cpp
Registry registry;

// Create a new entity with generational tracking
Entity e = registry.CreateEntity();

// Check if an entity handle is still valid
bool alive = registry.IsAlive(e);

// Destroy an entity and all its attached components
registry.DestroyEntity(e);
```

### Managing Components on an Entity
```cpp
// Add a component (in-place constructed)
registry.AddComponent<Transform>(e, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
registry.AddComponent<RigidBody>(e, glm::vec3(0.0f), 1.0f, true);

// Check if an entity possesses a component
if (registry.HasComponent<RigidBody>(e)) {
    // Retrieve a reference to component data
    RigidBody& rb = registry.GetComponent<RigidBody>(e);
    rb.velocity.y += 9.8f;
}

// Remove a component
registry.RemoveComponent<RigidBody>(e);
```

### Querying Entities (`ForEach`)

Iterate over all active entities that match a set of component requirements:

```cpp
// Queries all entities that have BOTH Transform and RigidBody components
registry.ForEach<Transform, RigidBody>([&](Entity entity) {
    Transform& transform = registry.GetComponent<Transform>(entity);
    RigidBody& rb = registry.GetComponent<RigidBody>(entity);

    transform.position += rb.velocity * deltaTime;
});
```

---

## 5. Systems (`core/ecs/System.h`)

A `System` represents logic filtered by a specific `ComponentSignature`:

```cpp
class PhysicsSystem : public System {
public:
    PhysicsSystem() {
        ComponentSignature signature;
        signature.set(GetComponentTypeID<Transform>());
        signature.set(GetComponentTypeID<RigidBody>());
        // Register signature
    }

    void Update(Registry& registry, float deltaTime) {
        registry.ForEach<Transform, RigidBody>([&](Entity e) {
            auto& tf = registry.GetComponent<Transform>(e);
            auto& rb = registry.GetComponent<RigidBody>(e);
            tf.position += rb.velocity * deltaTime;
        });
    }
};
```
