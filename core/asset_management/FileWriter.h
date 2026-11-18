#pragma once

#include "core/private_imports.h"

class FileWriter {
public:
    static bool Exists(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios::in | std::ios::binary);
        return file.is_open();
    }

    static void Write(const std::string& filepath, const std::string& content)
    {
        std::ofstream file(filepath, std::ios::out | std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "ERROR::FILE_WRITE_FAILED: " << filepath << std::endl;
            return;
        }
        file << content;
        file.close();
    }
};