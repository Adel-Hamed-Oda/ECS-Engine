#pragma once

enum class ModelLoaderFlags : unsigned int {
    None                  = 0,
    Triangulate           = 1 << 0,
    GenSmoothNormals      = 1 << 1,
    GenNormals            = 1 << 2,
    FlipUVs               = 1 << 3,
    CalcTangentSpace      = 1 << 4,
    JoinIdenticalVertices = 1 << 5,
    OptimizeMeshes        = 1 << 6,
    Default               = Triangulate | GenSmoothNormals | CalcTangentSpace | JoinIdenticalVertices
};

inline ModelLoaderFlags operator|(ModelLoaderFlags a, ModelLoaderFlags b) {
    return static_cast<ModelLoaderFlags>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}

inline bool operator&(ModelLoaderFlags a, ModelLoaderFlags b) {
    return (static_cast<unsigned int>(a) & static_cast<unsigned int>(b)) != 0;
}