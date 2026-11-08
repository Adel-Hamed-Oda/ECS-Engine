#pragma once

#include "core/private_imports.h"
#include "core/ecs/Registry.h"
#include "core/ecs/Component.h"

class Scene {
public:
    Scene() {
        static int nextId = 0;
        id = nextId++;
    }
    virtual ~Scene() {
        registry.ForEach([](Entity e) {
            registry.DestroyEntity(e);
        });
    }

    virtual void Create() {} // contains the setup of all entites, components and systems.
    void Update() {}
    void Render() {}

    // if the entity index is greater than the size of the vector,
    // fill it with falses until the index is reached, then set the entity index to true
    // otherwise, look for the first false value in the vector and set it to true
    Entity Instantiate() { 
        return registry.CreateEntity();
    }

    template <typename T, typename... Args>
    Entity AddComponent(Entity e, Args&&... args) {
        return registry.AddComponent(e, std::forward<decltype(args)>(args)...);
    }

    template <typename T>
    T& GetComponent(Entity e) {
        return registry.GetComponent<T>(e);
    }

    template <typename T>
    T& RemoveComponent(Entity e) {
        return registry.RemoveComponent<T>(e);
    }

    void Destroy(Entity e) {
        registry.DestroyEntity(e);
    }

private:
    int id;
    std::string name; // for referencing purposes, not as the unique identifier
    static Registry registry; // for shared entities across scenes (aka DontDestroyOnLoad from unity)
};