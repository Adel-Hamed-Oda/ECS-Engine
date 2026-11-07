#pragma once

#include "core/window/Window.h"

class Engine {
public:
    static void run();
    static void terminate();

private:
    inline static bool terminated = false;
    inline static Window* defaultWindow = nullptr;

    static void SetupEngine();
    static void MainLoop();
};
