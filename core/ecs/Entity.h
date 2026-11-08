#pragma once

#include "core/import_files/private_imports.h"

using Entity = uint32_t;
constexpr Entity NULL_ENTITY = 0xFFFFFFFF;

// Lower 20 bits: Index (up to ~1M concurrent entities)
// Upper 12 bits: Generation (up to 4096 reuses before rollover)
// The reason this is used is because creating and destroying entites is expected to
// happen alot, so reusing a certain number of bits is recommended, but having a specific set (in
// this case the first 12 bits) be different would be necessary

inline uint32_t GetEntityIndex(Entity e)      { return e & 0xFFFFF; }
inline uint32_t GetEntityGeneration(Entity e) { return (e >> 20) & 0xFFF; }
inline Entity CreateEntityID(uint32_t index, uint32_t generation) {
    return (generation << 20) | (index & 0xFFFFF);
}