#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

struct BoneInfo {
    int id = -1;
    glm::mat4 offsetMatrix = glm::mat4(1.0f);
    glm::mat4 invBindTransform = glm::mat4(1.0f);
};

struct SkeletonNode {
    std::string name;
    glm::mat4 transformation = glm::mat4(1.0f);
    std::vector<SkeletonNode> children;
};

struct Skeleton {
    std::unordered_map<std::string, BoneInfo> boneInfoMap;
    int boneCount = 0;
    SkeletonNode rootNode;
    glm::mat4 globalInverseTransform = glm::mat4(1.0f);

    bool HasBones() const {
        return boneCount > 0;
    }
};
