#pragma once

#include "core/private_imports.h"
#include "core/rendering/shader/Shader.h"
#include "core/inbuilt/components/Transform.h"
#include "core/inbuilt/components/Camera.h"

class Material {
public:
    std::shared_ptr<Shader> shader;

    Material(std::shared_ptr<Shader> shader) : shader(shader) {}
    virtual ~Material() = default;

    virtual void ApplyMaterial() = 0;
};