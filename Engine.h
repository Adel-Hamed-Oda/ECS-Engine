#pragma once

#include <string>
#include "core/window/Window.h"
#include "core/scenes/Scene.h"
#include "core/global/EngineTime.h"

#include "anim/Event.h"
#include "anim/Timer.h"
#include "anim/Tween.h"

class Engine {
public:
    static void init();
    static void run();
    static void terminate();

private:
    inline static bool initialized = false;
    inline static bool terminated = false;
    inline static float deltaTime = 0.0f;

    static void SetupEngine();
    static void MainLoop();
    static void UpdateScenes();
    static void RenderWindows();
};
