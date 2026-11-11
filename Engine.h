#pragma once

#include "core/window/Window.h"
#include "core/scenes/Scene.h"
#include "core/global/EngineTime.h"
#include "core/global/Input.h"
#include "core/global/KeyCode.h"

#include "core/events/Action.h"
#include "core/events/Event.h"

#include "anim/Timer.h"
#include "anim/Tween.h"

#include "core/inbuilt/components/Transform.h"
#include "core/inbuilt/components/Transform2D.h"
#include "core/inbuilt/components/Camera.h"
#include "core/inbuilt/components/Camera2D.h"
#include "core/inbuilt/components/MeshRenderer.h"

#include "core/rendering/mesh/Mesh.h"
#include "core/rendering/mesh/Primitives.h"
#include "core/rendering/shader/Shader.h"
#include "core/rendering/Renderer.h"

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
