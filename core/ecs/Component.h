#pragma once

#include "core/import_files/private_imports.h"

using ComponentTypeID = size_t;
constexpr size_t MAX_COMPONENTS = 64; // For testing purposes only, my small factory game had over 100 components
using ComponentSignature = std::bitset<MAX_COMPONENTS>;

// Gets a unique ID for each component type. (Transform = 1, Sprite = 2, etc.)
inline ComponentTypeID GetUniqueComponentID() {
    static ComponentTypeID lastID = 0;
    return lastID++;
}

// Gets the unique ID for a specific component type T. (Player Transform = 1, Player Sprite = 2, etc.)
template <typename T>
inline ComponentTypeID GetComponentTypeID() noexcept {
    static ComponentTypeID typeID = GetUniqueComponentID();
    return typeID;
}