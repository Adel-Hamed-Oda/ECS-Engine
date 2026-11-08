#include "core/scenes/Scene.h"
#include "core/private_imports.h"

Registry Scene::registry;
std::vector<Scene*> Scene::scenes;
Scene* Scene::currentScene = nullptr;

Scene::Scene(const std::string& name) : name(name) {
    scenes.push_back(this);

    if (currentScene == nullptr)
        currentScene = this;
}

Scene::~Scene() {
    registry.ForEach([](Entity e) {
        registry.DestroyEntity(e);
    });

    if (currentScene == this) {
        currentScene = scenes.empty() ? nullptr : scenes.front();
    }
    scenes.erase(std::remove(scenes.begin(), scenes.end(), this), scenes.end());
}

void Scene::Load() {}
void Scene::Unload() {}

void Scene::Start() {
    for (auto& system : systems[static_cast<int>(SystemType::Start)]) {
        system->apply(registry);
    }
}

void Scene::Update() {
    for (auto& system : systems[static_cast<int>(SystemType::Update)]) {
        system->apply(registry);
    }
}

void Scene::Render() {}

void Scene::ChangeScene(Scene* newScene) {
    if (newScene == nullptr || newScene == currentScene) return;

    currentScene->Unload();
    currentScene = newScene;
    currentScene->Load();
}

void Scene::ChangeScene(const std::string& sceneName) {
    for (auto* scene : scenes) {
        if (scene->name == sceneName) {
            ChangeScene(scene);
            return;
        }
    }
}

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
