#pragma once

#include "core/private_imports.h"
#include "Registry.h"

// later on, End, Awake, Enable and Disable systems would be added
enum class SystemType {
    Start = 0,
    Update = 1
};
inline constexpr int SystemTypeCount = 2; // Update this if more system types are added

class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void apply(Registry& registry) = 0;
};

template <typename... Comps>
class System : public ISystem {
public:
    // The function signature matches the components requested in the template
    using SystemFunc = std::function<void(Entity, Comps&...)>;

    // Constructor accepts the lambda
    System(SystemFunc func) : m_func(std::move(func)) {}

    // Executes the stored function on all entities matching the query
    void apply(Registry& registry) override {
        registry.ForEach<Comps...>(m_func);
    }

private:
    SystemFunc m_func;
};