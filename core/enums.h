#pragma once

// later on, End, Awake, Enable and Disable systems would be added
enum class SystemType {
    Start = 0,
    Update = 1
};
inline constexpr int SystemTypeCount = 2; // Update this if more system types are added