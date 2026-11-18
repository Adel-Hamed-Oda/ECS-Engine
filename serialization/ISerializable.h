#pragma once

#include "core/private_imports.h"

class ISerializable {
    friend class SaveSystem; // SaveSystem needs access to the ID for saving and loading objects safely

public:
    ISerializable() : ID(nextID++) {}
    virtual ~ISerializable() = default;

    unsigned int GetID() const { return ID; }

    virtual nlohmann::json Serialize() const = 0;
    virtual void Deserialize(const nlohmann::json& jsonData) = 0;

private:
    unsigned int ID = -1; // Unique identifier for the serializable object, I use it for safety, bas it's not 100% foolproof
    static unsigned int nextID;
};