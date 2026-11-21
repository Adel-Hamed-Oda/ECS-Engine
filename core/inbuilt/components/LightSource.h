#pragma once

#include <glm/glm.hpp>

struct LightSource {
public:
    glm::vec3 color;
    float intensity;

    LightSource(const glm::vec3& col, float inten)
        : color(col), intensity(inten) {}
};