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

add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/freetype)

add_library(Engine STATIC
    "Engine.cpp"
)

target_include_directories(Engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

find_package(OpenGL REQUIRED)

target_link_libraries(Engine INTERFACE 
    OpenGL::GL
    glfw
)