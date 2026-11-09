#include "core/scenes/Scene.h"
#include "core/window/Window.h"
#include "core/private_imports.h"

std::vector<Scene*> Scene::scenes;

Scene* Scene::Get(const std::string& name) {
    for (Scene* scene : scenes) {
        if (scene && scene->name == name) return scene;
    }
    return nullptr;
}
const std::vector<Scene*>& Scene::GetAllScenes() {
    return scenes;
}

Scene::Scene(const std::string& name) : name(name) {
    scenes.push_back(this);
}

Scene::~Scene() {
    // Detach this scene from any window currently pointing to it
    for (Window* win : Window::GetAllWindows()) {
        if (win && win->GetScene() == this) {
            win->SetScene(nullptr);
        }
    }

    registry.DestroyAllEntities();

    scenes.erase(std::remove(scenes.begin(), scenes.end(), this), scenes.end());
}

void Scene::Load() {
    if (state == SceneState::Unloaded) {
        onLoad.Invoke();
        state = SceneState::Loaded;
    }
}

void Scene::Unload() {
    if (state == SceneState::Unloaded) return;

    onUnload.Invoke();
    registry.DestroyAllEntities();
    for (auto& systemList : systems) {
        systemList.clear();
    }
    state = SceneState::Unloaded;
}

void Scene::Update() {
    // If the scene is already active, just apply the update systems
    // otherwise the start systems will be applied first, and then the update systems
    if (state == SceneState::Active) {
        for (auto& system : systems[static_cast<int>(SystemType::Update)]) {
            system->apply(registry);
        }
        return;
    }
    for (auto& system : systems[static_cast<int>(SystemType::Update)]) {
        system->apply(registry);
    }
    state = SceneState::Active;
}

void Scene::Render() {
    // To be done later ISA
}

const std::string& Scene::GetName() const {
    return name;
}
const SceneState Scene::GetState() const {
    return state;
}

Registry& Scene::GetRegistry() {
    return registry;
}
