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
    void DestroyAllEntities() {
        while (!aliveEntities.empty()) {
            DestroyEntity(aliveEntities.back());
        }
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

    // Iterates over all entities that have the specified component signature and applies the function to them.
    template <typename Func>
    void ForEach(Func&& func, const ComponentSignature& mask) {
        for (Entity e : aliveEntities) {
            if ((signatures[GetEntityIndex(e)] & mask) == mask) {
                func(e);
            }
        }
    }

    // Same as previous, taking in individual component types
    template <typename... Comps, typename Func>
    void ForEach(Func&& func) {
        if constexpr (sizeof...(Comps) == 0) {
            for (Entity e : aliveEntities) {
                func(e);
            }
        } else if constexpr (sizeof...(Comps) == 1) {
            using CompT = std::tuple_element_t<0, std::tuple<Comps...>>;
            ComponentPool<CompT>* pool = TryGetPool<CompT>();
            if (!pool) return;

            std::vector<CompT>& dense = pool->GetDenseArray();
            const std::vector<Entity>& entities = pool->GetDenseEntities();
            for (size_t i = 0; i < entities.size(); ++i) {
                if constexpr (std::is_invocable_v<Func, Entity, CompT&>) {
                    func(entities[i], dense[i]);
                } else {
                    func(entities[i]);
                }
            }
        } else {
            auto pools = std::make_tuple(TryGetPool<Comps>()...);
            ForEachMulti(std::forward<Func>(func), pools, std::index_sequence_for<Comps...>{});
        }
    }

private:
    template <typename Func, typename... Comps, size_t... Is>
    void ForEachMulti(Func&& func, const std::tuple<ComponentPool<Comps>*...>& pools, std::index_sequence<Is...>) {
        const bool anyEmpty = ((std::get<Is>(pools) == nullptr || std::get<Is>(pools)->GetDenseEntities().empty()) || ...);
        if (anyEmpty) return;

        const std::vector<Entity>* smallestEntities = nullptr;
        size_t minSize = SIZE_MAX;
        (
            [&]() {
                const auto& ents = std::get<Is>(pools)->GetDenseEntities();
                if (ents.size() < minSize) {
                    minSize = ents.size();
                    smallestEntities = &ents;
                }
            }(),
            ...
        );

        ComponentSignature mask;
        (mask.set(GetComponentTypeID<Comps>()), ...);

        const std::vector<Entity>& entities = *smallestEntities;
        for (size_t i = 0; i < entities.size(); ++i) {
            Entity e = entities[i];
            uint32_t entityIndex = GetEntityIndex(e);
            if ((signatures[entityIndex] & mask) == mask) {
                if constexpr (std::is_invocable_v<Func, Entity, Comps&...>) {
                    func(e, std::get<Is>(pools)->GetDataByIndex(entityIndex)...);
                } else {
                    func(e);
                }
            }
        }
    }

    template <typename T>
    ComponentPool<T>* TryGetPool() const {
        ComponentTypeID id = GetComponentTypeID<T>();
        auto it = componentPools.find(id);
        if (it == componentPools.end()) {
            return nullptr;
        }
        return static_cast<ComponentPool<T>*>(it->second.get());
    }

    template <typename T>
    ComponentPool<T>* GetOrCreatePool() {
        ComponentTypeID id = GetComponentTypeID<T>();
        auto it = componentPools.find(id);
        if (it == componentPools.end()) {
            auto pool = std::make_shared<ComponentPool<T>>();
            ComponentPool<T>* rawPtr = pool.get();
            componentPools.emplace(id, std::move(pool));
            return rawPtr;
        }
        return static_cast<ComponentPool<T>*>(it->second.get());
    }

    std::vector<uint32_t> generations;
    std::queue<uint32_t> freeIndices;
    std::vector<ComponentSignature> signatures;
    std::vector<Entity> aliveEntities;
    std::unordered_map<ComponentTypeID, std::shared_ptr<IComponentArray>> componentPools;
};