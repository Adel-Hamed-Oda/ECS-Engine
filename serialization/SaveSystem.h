#pragma once

#include "core/private_imports.h"
#include "core/asset_management/FileReader.h"
#include "core/asset_management/FileWriter.h"
#include "ISerializable.h"

// NOTE: this save system is gonna be an unsafe one where overwriting can happen without warning
// a better system would include ownership rules or (even better) server-side saving.
class SaveSystem {
public:
    static void Save(const std::string& filename, const ISerializable* serializable) {
        nlohmann::json jsonData = serializable->Serialize();
        FileWriter::Write(filename, jsonData.dump(4));
    }

    static void Load(const std::string& filename, ISerializable* serializable) {
        if (!FileReader::Exists(filename)) {
            std::cerr << "ERROR::FILE_NOT_FOUND: " << filename << std::endl;
            return;
        }

        if (!serializable || serializable->GetID() == -1) {
            std::cerr << "ERROR::NULL_SERIALIZABLE: Cannot load into a null serializable object." << std::endl;
            return;
        }

        std::string fileContent = FileReader::Read(filename);
        nlohmann::json jsonData = nlohmann::json::parse(fileContent);
        serializable->Deserialize(jsonData);
    }
};