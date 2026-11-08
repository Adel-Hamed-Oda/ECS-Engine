#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include "core/ecs/Registry.h"
#include "core/ecs/Component.h"
#include "core/ecs/System.h"

class Scene {
public:
    Scene(const std::string& name = "");
    virtual ~Scene();

    virtual void Load();
    virtual void Unload();
    void Start();
    void Update();
    void Render();

    Entity Instantiate();
    void DeleteSystem(ISystem* system);
    void Destroy(Entity e);

    template <typename T, typename... Args>
    T* AddSystem(SystemType type, Args&&... args) {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* rawPtr = system.get(); 
        systems[static_cast<int>(type)].push_back(std::move(system));
        return rawPtr;
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
    void RemoveComponent(Entity e) {
        registry.RemoveComponent<T>(e);
    }

private:
    std::string name;
    std::vector<std::unique_ptr<ISystem>> systems[SystemTypeCount];
    Registry registry;

    // flags to track the state of the scene
    bool isStarted = false;
    bool isLoaded = false;
    bool active = false;

    static std::vector<Scene*> scenes;
};
