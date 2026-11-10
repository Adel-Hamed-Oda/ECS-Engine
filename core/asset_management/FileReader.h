#pragma once

#include "core/private_imports.h"

class FileReader {
public:
    static std::string ReadFile(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "ERROR::FILE_NOT_FOUND_OR_READ_FAILED: " << filepath << std::endl;
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};