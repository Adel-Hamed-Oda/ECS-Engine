#pragma once

#include <string>
#include <unordered_map>

#include "anim/BoneChannel.h"

struct AnimationClip {
    std::string name;
    float duration = 0.0f;
    float ticksPerSecond = 25.0f;
    std::unordered_map<std::string, BoneChannel> channels;

    const BoneChannel* FindChannel(const std::string& nodeName) const {
        auto it = channels.find(nodeName);
        return (it != channels.end()) ? &it->second : nullptr;
    }
};
