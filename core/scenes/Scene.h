#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include "core/ecs/Registry.h"
#include "core/ecs/Component.h"
#include "core/ecs/System.h"

// Defines the current state of the scene
// - Unloaded: The scene is not loaded and has no active entities or systems.
// - Loaded: The scene is loaded, but not yet active. Systems can be added, and entities can be created, but the scene is not yet being updated.
// - Active: The scene is active and being updated. Systems are applied to entities, and the scene is rendered.
enum class SceneState {
    Unloaded,
    Loaded,
    Active
};

class Scene {
public:
    static Scene* Get(const std::string& name);
    static const std::vector<Scene*>& GetAllScenes();

    Scene(const std::string& name = "");
    virtual ~Scene();

    // the _ is for internal functions that shouldn't be altered by the user
    void _Load();
    virtual void Load();
    void _Unload();
    virtual void Unload();

    void Update();
    void Render();

    const std::string& GetName() const;
    const SceneState GetState() const;
    Registry& GetRegistry();

    Entity Instantiate() { return registry.CreateEntity(); }
    void Destroy(Entity e) { registry.DestroyEntity(e); }

    template <typename T, typename... Args>
    T& AddComponent(Entity e, Args&&... args) { return registry.AddComponent<T>(e, std::forward<Args>(args)...); }
    template <typename T>
    T& GetComponent(Entity e) { return registry.GetComponent<T>(e); }
    template <typename T>
    bool HasComponent(Entity e) const { return registry.HasComponent<T>(e); }
    template <typename T>
    void RemoveComponent(Entity e) { registry.RemoveComponent<T>(e); }

    template <typename T, typename... Args>
    T* AddSystem(SystemType type, Args&&... args) {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* rawPtr = system.get(); 
        systems[static_cast<int>(type)].push_back(std::move(system));
        return rawPtr;
    }
    void DeleteSystem(ISystem* system) {
        for (auto& systemList : systems) {
            systemList.erase(
                std::remove_if(systemList.begin(), systemList.end(),
                    [system](const std::unique_ptr<ISystem>& ptr) {
                        return ptr.get() == system;
                    }), 
                systemList.end()
            );
        }
    }

private:
    std::string name;
    SceneState state = SceneState::Unloaded;

    Registry registry; // entites, components
    std::vector<std::unique_ptr<ISystem>> systems[SystemTypeCount]; // systems

    static std::vector<Scene*> scenes;
};
