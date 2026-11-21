#pragma once

#include "core/window/Window.h"
#include "core/scenes/Scene.h"
#include "core/global/EngineTime.h"
#include "core/global/Profiler.h"

#include "core/events/Action.h"
#include "core/events/Event.h"

#include "core/inbuilt/components/Transform.h"
#include "core/inbuilt/components/Transform2D.h"
#include "core/inbuilt/components/Camera.h"
#include "core/inbuilt/components/Camera2D.h"
#include "core/inbuilt/components/LightSource.h"
#include "core/inbuilt/components/MeshRenderer.h"
#include "core/inbuilt/components/TextRenderer.h"
#include "core/inbuilt/components/ImageRenderer.h"

#include "core/rendering/material/Material.h"
#include "core/rendering/material/WorldspaceMaterial.h"
#include "core/rendering/material/DefaultMaterial.h"
#include "core/rendering/material/UnlitMaterial.h"
#include "core/rendering/material/ScreenspaceMaterial.h"
#include "core/rendering/material/TextMaterial.h"
#include "core/rendering/material/ImageMaterial.h"

#include "core/rendering/mesh/Mesh.h"
#include "core/rendering/mesh/Primitives.h"
#include "core/rendering/texture/TextureEnums.h"
#include "core/rendering/texture/Texture.h"
#include "core/rendering/font/Font.h"
#include "core/rendering/shader/Shader.h"
#include "core/FallBacks.h"
#include "core/rendering/Renderer.h"
#include "core/asset_management/ModelLoader/ModelLoader.h"

#include "anim/Timer.h"
#include "anim/Tween.h"

#include "peripherals/KeyBoard.h"
#include "peripherals/KeyCode.h"
#include "peripherals/Mouse.h"

class Engine {
    friend class DebugWindow;

public:
    static void init();
    static void run();
    static void terminate();

    static DebugWindow* OpenDebugWindow(glm::ivec2 position = glm::ivec2(40, 40), glm::ivec2 size = glm::ivec2(720, 580));
    static void CloseDebugWindow();
    static DebugWindow* GetDebugWindow();
    static bool IsDebugWindowOpen();

private:
    inline static bool active = false;
    inline static float deltaTime = 0.0f;

    static void MainLoop();
    static void UpdateScenes();
    static void RenderWindows();
};
