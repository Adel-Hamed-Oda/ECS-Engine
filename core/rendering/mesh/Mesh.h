#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Mesh
{
public:
    static constexpr int MAX_BONE_INFLUENCE = 4;
    static constexpr int MAX_BONES = 200;

    Mesh() = default;

    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> triangles;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> tangents;
    std::vector<glm::ivec4> boneIDs;
    std::vector<glm::vec4> boneWeights;
};
