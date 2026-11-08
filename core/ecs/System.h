#pragma once

#include "core/private_imports.h"
#include "Registry.h"

class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void apply(Registry& registry) = 0;
};

template <typename... Comps>
class UpdateSystem : public ISystem {
public:
    // The function signature matches the components requested in the template
    using SystemFunc = std::function<void(Entity, Comps&...)>;

    // Constructor accepts the lambda
    UpdateSystem(SystemFunc func) : m_func(std::move(func)) {}

    // Executes the stored function on all entities matching the query
    void apply(Registry& registry) override {
        registry.ForEach<Comps...>(m_func);
    }

private:
    SystemFunc m_func;
};