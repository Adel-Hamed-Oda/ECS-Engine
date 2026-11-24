#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Mesh
{
public:
    Mesh() = default;

    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> triangles;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> tangents;
};
