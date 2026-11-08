#pragma once

#include "core/private_imports.h"
#include "Component.h"
#include "ComponentPool.h"

class Registry {
public:
    Registry() {
        // Pre-fill free indices for initial entity creation
        for (uint32_t i = 0; i < 1024; ++i) {
            freeIndices.push(i);
            generations.push_back(0);
        }
    }

    Entity CreateEntity() {
        uint32_t index;
        if (!freeIndices.empty()) {
            index = freeIndices.front();
            freeIndices.pop();
        } else {
            index = static_cast<uint32_t>(generations.size());
            generations.push_back(0);
        }

        Entity e = CreateEntityID(index, generations[index]);
        if (index >= signatures.size()) {
            signatures.resize(index + 1);
        }
        signatures[index].reset();
        aliveEntities.push_back(e);
        return e;
    }

    void DestroyEntity(Entity e) {
        uint32_t index = GetEntityIndex(e);
        assert(IsAlive(e));

        for (auto& [id, pool] : componentPools) {
            pool->Destroy(e);
        }

        signatures[index].reset();
        generations[index]++; // Invalidate existing handles
        freeIndices.push(index);

        aliveEntities.erase(std::remove(aliveEntities.begin(), aliveEntities.end(), e), aliveEntities.end());
    }

    bool IsAlive(Entity e) const {
        uint32_t index = GetEntityIndex(e);
        return index < generations.size() && generations[index] == GetEntityGeneration(e);
    }

    template <typename T, typename... Args>
    T& AddComponent(Entity e, Args&&... args) {
        auto pool = GetOrCreatePool<T>();
        T comp(std::forward<Args>(args)...);
        pool->InsertData(e, std::move(comp));

        signatures[GetEntityIndex(e)].set(GetComponentTypeID<T>());
        return pool->GetData(e);
    }

    template <typename T>
    void RemoveComponent(Entity e) {
        GetOrCreatePool<T>()->RemoveData(e);
        signatures[GetEntityIndex(e)].reset(GetComponentTypeID<T>());
    }

    template <typename T>
    T& GetComponent(Entity e) {
        return GetOrCreatePool<T>()->GetData(e);
    }

    template <typename T>
    bool HasComponent(Entity e) const {
        return signatures[GetEntityIndex(e)].test(GetComponentTypeID<T>());
    }

    // Iterate entities matching a set of components
    template <typename... Comps, typename Func>
    void ForEach(Func&& func) {
        ComponentSignature mask;
        (mask.set(GetComponentTypeID<Comps>()), ...);

        for (Entity e : aliveEntities) {
            if ((signatures[GetEntityIndex(e)] & mask) == mask) {
                func(e, GetComponent<Comps>(e)...);
            }
        }
    }

private:
    template <typename T>
    std::shared_ptr<ComponentPool<T>> GetOrCreatePool() {
        ComponentTypeID id = GetComponentTypeID<T>();
        if (!componentPools.contains(id)) {
            componentPools[id] = std::make_shared<ComponentPool<T>>();
        }
        return std::static_pointer_cast<ComponentPool<T>>(componentPools[id]);
    }

    std::vector<uint32_t> generations;
    std::queue<uint32_t> freeIndices;
    std::vector<ComponentSignature> signatures;
    std::vector<Entity> aliveEntities;
    std::unordered_map<ComponentTypeID, std::shared_ptr<IComponentArray>> componentPools;
};