#include "core/scenes/Scene.h"
#include "core/private_imports.h"

std::vector<Scene*> Scene::scenes;

Scene::Scene(const std::string& name) : name(name) {
    scenes.push_back(this);
}

Scene::~Scene() {
    registry.DestroyAllEntities();

    scenes.erase(std::remove(scenes.begin(), scenes.end(), this), scenes.end());
}

void Scene::Load() {}
void Scene::Unload() {}

void Scene::Start() {
    if (isStarted) return;

    for (auto& system : systems[static_cast<int>(SystemType::Start)]) {
        system->apply(registry);
    }

    isStarted = true;
}

void Scene::Update() {
    for (auto& system : systems[static_cast<int>(SystemType::Update)]) {
        system->apply(registry);
    }
}

void Scene::Render() {}

Entity Scene::Instantiate() { 
    return registry.CreateEntity();
}

void Scene::DeleteSystem(ISystem* system) {
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

void Scene::Destroy(Entity e) {
    registry.DestroyEntity(e);
}
