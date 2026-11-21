cmake_minimum_required(VERSION 3.20)
project(ECS_Engine VERSION 0.1.0
    DESCRIPTION "Entity Component System Engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(FT_DISABLE_ZLIB     ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2    ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG      ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI   ON CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_TESTS          OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT            ON  CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL              OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS   OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES        OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS   OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/freetype)
add_subdirectory(external/assimp)

add_library(Engine STATIC
    "Engine.cpp"
    "core/window/Window.cpp"
    "core/window/DebugWindow.cpp"
    "core/scenes/Scene.cpp"
    "core/rendering/texture/Texture.cpp"
    "core/rendering/texture/TextureMap.cpp"
    "core/rendering/texture/TextureUtils.cpp"
    "core/asset_management/ModelLoader.cpp"
    "external/implementations/glad.cpp"
    "external/implementations/stb.cpp"
)

target_include_directories(Engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/freetype/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
)

find_package(OpenGL REQUIRED)

if(MSVC)
    target_compile_options(Engine PUBLIC /MP)
endif()

target_link_libraries(Engine PUBLIC 
    OpenGL::GL
    glfw
    freetype
    assimp
)