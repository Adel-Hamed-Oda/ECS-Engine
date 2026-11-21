#pragma once

#include <string>

class FallBacks {
public:
    // ========================================================================
    // 3D Fallback Shader (also used as unlit fallback shader)
    // ========================================================================
    inline static const std::string VertexShaderCode = R"(
        #version 330 core

        // Same attribute layout as the main shader so existing VAOs work unchanged.
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec4 aColor;
        layout (location = 3) in vec2 aUV;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        out vec3 localPosition;

        void main()
        {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            localPosition = aPos;
        }
    )";

    inline static const std::string FragmentShaderCode = R"(
        #version 330 core

        in vec3 localPosition;

        out vec4 FragColor;

        // Number of checker cells per object-space unit (a unit cube gets 4 cells per edge).
        const float CHECKER_SCALE = 4.0;

        const vec3 BLACK  = vec3(0.0, 0.0, 0.0);
        const vec3 PURPLE = vec3(1.0, 0.0, 1.0);

        void main()
        {
            // The small offset keeps flat faces that sit exactly on a cell boundary
            // (e.g. a cube face at 0.5) from flickering between two cells.
            vec3 cell = floor(localPosition * CHECKER_SCALE + 0.001);

            // Alternates every cell along each axis; mod() is always non-negative here.
            float checker = mod(cell.x + cell.y + cell.z, 2.0);

            FragColor = vec4(mix(BLACK, PURPLE, checker), 1.0);
        }
    )";

    // Unlit shader code uses the same fallback shader code as the 3D fallback
    inline static const std::string& UnlitVertexShaderCode = VertexShaderCode;
    inline static const std::string& UnlitFragmentShaderCode = FragmentShaderCode;

    // ========================================================================
    // 2D Screen-space Image Fallback Shader
    // ========================================================================
    inline static const std::string ImageVertexShaderCode = R"(
        #version 330 core
        layout (location = 0) in vec4 vertex; // vec2 pos (0..1), vec2 uv (0..1)

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        out vec2 TexCoords;

        void main()
        {
            TexCoords = vertex.zw;
            gl_Position = projection * view * model * vec4(vertex.xy, 0.0, 1.0);
        }
    )";

    inline static const std::string ImageFragmentShaderCode = R"(
        #version 330 core
        in vec2 TexCoords;
        out vec4 FragColor;

        uniform sampler2D imageTexture;
        uniform bool hasTexture;
        uniform vec4 tintColor;

        void main()
        {
            vec4 texColor = hasTexture ? texture(imageTexture, TexCoords) : vec4(1.0);
            FragColor = texColor * tintColor;
        }
    )";
};
