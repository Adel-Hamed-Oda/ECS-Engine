#pragma once

#include "core/window/Window.h"
#include "core/scenes/Scene.h"
#include "core/global/EngineTime.h"

#include "core/events/Action.h"
#include "core/events/Event.h"

#include "anim/Timer.h"
#include "anim/Tween.h"

class Engine {
public:
    static void init();
    static void run();
    static void terminate();

private:
    inline static bool active = false;
    inline static float deltaTime = 0.0f;

    static void MainLoop();
    static void UpdateScenes();
    static void RenderWindows();
};
