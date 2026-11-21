#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

class FileReader {
public:
    static bool Exists(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios::in | std::ios::binary);
        return file.is_open();
    }

    static std::string Read(const std::string& filepath, bool logOnError = true)
    {
        std::ifstream file(filepath, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            if (logOnError)
            {
                std::cerr << "ERROR::FILE_NOT_FOUND_OR_READ_FAILED: " << filepath << std::endl;
            }
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    static std::string ReadFirstExisting(const std::vector<std::string>& candidatePaths)
    {
        for (const auto& path : candidatePaths)
        {
            if (Exists(path))
            {
                return Read(path, false);
            }
        }
        return "";
    }
};