cmake_minimum_required(VERSION 3.20)
project(ECS_Engine VERSION 0.1.0
    DESCRIPTION "Entity Component System Engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_subdirectory(external/glfw)

add_library(Engine
    "Engine.cpp"
)

target_include_directories(Engine INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(OpenGL REQUIRED)

target_link_libraries(Engine INTERFACE 
    OpenGL::GL
    # glfw  <-- Make sure glfw's add_subdirectory is uncommented if linking here
)