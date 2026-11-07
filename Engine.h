#pragma once

#include "core/import_files/public_imports.h"

class Engine {
public:
    static void run();
    static void terminate();

private:
    inline static bool terminated = false;
};