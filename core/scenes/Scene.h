#pragma once

#include "core/private_imports.h"
#include "core/ecs/Registry.h"
#include "core/ecs/Component.h"
#include "core/ecs/System.h"

class Scene {
public:
    Scene(const std::string& name = "") : name(name) {
        scenes.push_back(this);

        if (currentScene == nullptr)
            currentScene = this;
    }
    virtual ~Scene() {
        registry.ForEach([](Entity e) {
            registry.DestroyEntity(e);
        });

        if (currentScene == this) {
            currentScene = scenes.empty() ? nullptr : scenes.front();
        }
        scenes.erase(std::remove(scenes.begin(), scenes.end(), this), scenes.end());
    }

    virtual void Load() {} // contains the setup of all entites, components and systems.
    virtual void Unload() {} // contains the cleanup of all entites, components and systems.
    void Start() {
        for (auto& system : systems[static_cast<int>(SystemType::Start)]) {
            system->apply(registry);
        }
    }
    void Update() {
        for (auto& system : systems[static_cast<int>(SystemType::Update)]) {
            system->apply(registry);
        }
    }
    void Render() {}
    void ChangeScene(Scene* newScene) {
        if (newScene == nullptr || newScene == currentScene) return;

        currentScene->Unload();
        currentScene = newScene;
        currentScene->Load();
    }
    void ChangeScene(const std::string& sceneName) {
        for (auto* scene : scenes) {
            if (scene->name == sceneName) {
                ChangeScene(scene);
                return;
            }
        }
    }

    Entity Instantiate() { 
        return registry.CreateEntity();
    }
    template <typename T, typename... Args>
    T* AddSystem(SystemType type, Args&&... args) {
        // Construct the specific system type (T) dynamically
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* rawPtr = system.get(); 
        
        // put it in its appropriate vector in the array of vectors
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
    void Destroy(Entity e) {
        registry.DestroyEntity(e);
    }

private:
    std::string name; // for referencing purposes, not as the unique identifier
    std::vector<std::unique_ptr<ISystem>> systems[SystemTypeCount];

    inline static Registry registry;
    inline static std::vector<Scene*> scenes;
    inline static Scene* currentScene = nullptr;
};