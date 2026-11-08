#pragma once

#include "core/import_files/private_imports.h"
#include "Entity.h"

class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void Destroy(Entity e) = 0;
};

template <typename T>
class ComponentPool : public IComponentArray {
public:
    void InsertData(Entity e, T component) {
        uint32_t index = GetEntityIndex(e);
        assert(index < sparse.size() || (sparse.resize(index + 1, -1), true));

        // Point sparse to the end of the dense array
        sparse[index] = static_cast<int32_t>(dense.size());
        denseEntities.push_back(e);
        dense.push_back(std::move(component));
    }

    void RemoveData(Entity e) {
        // Get the index of the entity in the sparse array
        uint32_t index = GetEntityIndex(e);
        assert(HasData(e)); // ensure the entity has a component to remove

        size_t indexOfRemoved = sparse[index];
        size_t indexOfLast = dense.size() - 1;

        // Swap with the last element for O(1) removal
        dense[indexOfRemoved] = std::move(dense[indexOfLast]);
        denseEntities[indexOfRemoved] = denseEntities[indexOfLast];

        // Update sparse entry of moved entity
        uint32_t movedEntityIndex = GetEntityIndex(denseEntities[indexOfLast]);
        sparse[movedEntityIndex] = static_cast<int32_t>(indexOfRemoved);

        // Pop last elements and invalidate sparse
        dense.pop_back();
        denseEntities.pop_back();
        sparse[index] = -1;
    }

    // Gets the component data for the given entity (the assert is for error handling)
    T& GetData(Entity entity) {
        assert(HasData(entity));
        return dense[sparse[GetEntityIndex(entity)]];
    }

    // checks if the given entity has a component of this type
    bool HasData(Entity entity) const {
        uint32_t index = GetEntityIndex(entity);
        return index < sparse.size() && sparse[index] != -1;
    }

    // Remove all data for the given entity
    void Destroy(Entity entity) override {
        if (HasData(entity)) {
            RemoveData(entity);
        }
    }

    // Direct access to dense contiguous buffer for rendering/physics loops
    // Please don't change these at whim 3ashan khatry :)
    std::vector<T>& GetDenseArray() { return dense; }
    const std::vector<Entity>& GetDenseEntities() const { return denseEntities; }

private:
    // this is the array of components put side by side in memory for optimal cache usage
    std::vector<T> dense; 

    // this is the array of entities corresponding to the dense array, for each element
    // in the dense array, there is one here at the same position corresponding to the 
    // entity that owns that component, aka at denseEntites[i] is the entity that owns dense[i]
    std::vector<Entity> denseEntities; 

    // this is a sparse array used for O(1) lookups, if the last active entity has the id FFFF, then
    // sparse[FFFF] will contain the index of it's component in the dense array
    std::vector<int32_t> sparse; // Maps EntityIndex -> Dense index (-1 if absent)
};