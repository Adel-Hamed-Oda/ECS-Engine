#include "Engine.h"

DebugWindow* DebugWindow::s_instance = nullptr;

DebugWindow* DebugWindow::Open(glm::ivec2 position, glm::ivec2 size, Window* shareWindow) {
    if (s_instance && s_instance->IsOpen()) {
        return s_instance;
    }

    Window* shareContext = shareWindow ? shareWindow : Window::GetPrimaryWindow();
    s_instance = new DebugWindow(position, size, shareContext);
    return s_instance;
}

void DebugWindow::CloseAll() {
    if (s_instance) {
        DebugWindow* win = s_instance;
        s_instance = nullptr;
        if (win->IsOpen()) {
            win->Close();
        }
        delete win;
    }
}

DebugWindow* DebugWindow::GetInstance() {
    return s_instance;
}

bool DebugWindow::IsOpenStatic() {
    return s_instance != nullptr && s_instance->IsOpen();
}

DebugWindow::DebugWindow(glm::ivec2 position, glm::ivec2 size, Window* shareWindow)
    : Window("Debug", position, size, shareWindow ? shareWindow : Window::GetPrimaryWindow(), true)
{
    s_instance = this;

    // Restore active context to primary game window so creating DebugWindow doesn't steal focus/input
    if (Window* primary = Window::GetPrimaryWindow()) {
        if (primary != this && primary->IsOpen()) {
            primary->MakeContextCurrent();
        }
    }
}

DebugWindow::~DebugWindow() {
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

void DebugWindow::Close() {
    if (s_instance == this) {
        s_instance = nullptr;
    }
    Window::Close();
}

void DebugWindow::SetScene(Scene* /*scene*/) {
    std::cerr << "Warning: DebugWindow does not have the ability to change scenes." << std::endl;
}

void DebugWindow::SetScene(const std::string& /*sceneName*/) {
    std::cerr << "Warning: DebugWindow does not have the ability to change scenes." << std::endl;
}

void DebugWindow::SetCamera(Entity /*camera*/) {
    // DebugWindow does not view scene cameras
}

void DebugWindow::SetAsPrimaryWindow() {
    // DebugWindow cannot become primary
}

void DebugWindow::Render() {
    glm::ivec2 fbSize = GetFramebufferSize();
    if (fbSize.x <= 0 || fbSize.y <= 0) return;

    // Delegate to engine's Window::Render() to set viewport and clear buffers
    Window::Render();

    // Render debug diagnostics using engine components only (TextRenderer, Transform2D, etc.)
    RenderDebugContent(fbSize.x, fbSize.y);
}

void DebugWindow::RenderDebugContent(int width, int height) {
    if (width <= 0 || height <= 0) return;

    // Frame time tracking via engine Time
    float dt = static_cast<float>(Time::deltaTime);
    float currentMs = dt * 1000.0f;

    static std::vector<float> history(60, 16.6f);
    static size_t hIndex = 0;
    history[hIndex] = currentMs;
    hIndex = (hIndex + 1) % history.size();

    float sumMs = 0.0f;
    float minMs = 9999.0f;
    float maxMs = 0.0f;
    for (float ms : history) {
        sumMs += ms;
        if (ms < minMs) minMs = ms;
        if (ms > maxMs) maxMs = ms;
    }
    float avgMs = sumMs / static_cast<float>(history.size());
    float fps = (avgMs > 0.001f) ? (1000.0f / avgMs) : 0.0f;

    // Scene and entity metrics via engine Scene & Registry
    std::vector<Scene*> allScenes = Scene::GetAllScenes();
    int activeScenesCount = 0;
    int totalEntities = 0;
    int totalMeshes = 0;
    int totalLights = 0;
    int totalCameras = 0;
    std::string primarySceneName = "None";

    for (Scene* s : allScenes) {
        if (!s) continue;
        if (s->GetState() != SceneState::Unloaded) {
            activeScenesCount++;
            if (primarySceneName == "None") {
                primarySceneName = s->GetName();
            }
            Registry& reg = s->GetRegistry();
            totalEntities += static_cast<int>(reg.GetAliveEntityCount());
            reg.ForEach<MeshRenderer>([&](Entity) { totalMeshes++; });
            reg.ForEach<LightSource>([&](Entity) { totalLights++; });
            reg.ForEach<Camera>([&](Entity) { totalCameras++; });
        }
    }

    Window* primary = Window::GetPrimaryWindow();
    std::string primaryTitle = primary ? primary->GetTitle() : "None";
    glm::ivec2 primaryRes = primary ? primary->GetSize() : glm::ivec2(0, 0);

    // Health color coding
    glm::vec3 badgeColor;
    if (fps >= 50.0f) {
        badgeColor = glm::vec3(0.25f, 0.85f, 0.40f); // Green
    } else if (fps >= 30.0f) {
        badgeColor = glm::vec3(0.95f, 0.75f, 0.15f); // Yellow
    } else {
        badgeColor = glm::vec3(0.95f, 0.25f, 0.25f); // Red
    }

    // Calculate recent frame stream and stability
    const size_t streamCount = 50;
    std::string historyStream;
    historyStream.reserve(streamCount);
    int at60Count = 0;

    for (size_t i = 0; i < streamCount; ++i) {
        float sampleMs = history[(hIndex + i) % history.size()];
        if (sampleMs <= 18.0f) {
            historyStream += '|';
            at60Count++;
        } else if (sampleMs <= 34.0f) {
            historyStream += ':';
        } else {
            historyStream += '.';
        }
    }

    // ------------------------------------------------------------------------
    // Render text diagnostics using engine's TextRenderer, Transform2D & TextMaterial
    // ------------------------------------------------------------------------
    glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

    static std::shared_ptr<Font> debugFont = nullptr;
    static std::shared_ptr<TextMaterial> debugMaterial = nullptr;
    static std::unique_ptr<TextRenderer> debugText = nullptr;

    if (!debugFont) {
        debugFont = Font::Create("fonts/arial.ttf", 15.0f);
    }
    if (!debugMaterial) {
        debugMaterial = std::make_shared<TextMaterial>();
    }
    if (!debugText && debugFont && debugFont->IsValid()) {
        debugText = std::make_unique<TextRenderer>("", debugFont, debugMaterial);
    }

    auto formatFloat = [](float val, int precision) -> std::string {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(precision) << val;
        return ss.str();
    };

    auto print = [&](const std::string& str, float tx, float ty, const glm::vec3& col) {
        if (!debugText || !debugFont || !debugFont->IsValid() || str.empty()) return;
        debugText->text = str;
        if (debugMaterial) debugMaterial->textColor = col;
        Transform2D t2d;
        t2d.position = glm::vec2(tx, ty);
        t2d.scale = glm::vec2(1.0f, 1.0f);
        debugText->Draw(&t2d, ortho, col);
    };

    const float fHeight = static_cast<float>(height);
    const float leftMargin = 32.0f;
    const std::string divider = "----------------------------------------------------------------";

    float cursorY = fHeight - 38.0f;

    // 1. Header Section
    print("ENGINE PERFORMANCE DEBUGGER", leftMargin, cursorY, glm::vec3(0.95f, 0.95f, 1.0f));
    cursorY -= 20.0f;
    print("Real-Time Diagnostic Profiler", leftMargin, cursorY, glm::vec3(0.45f, 0.65f, 0.90f));
    cursorY -= 24.0f;

    print(divider, leftMargin, cursorY, glm::vec3(0.25f, 0.28f, 0.35f));
    cursorY -= 24.0f;

    // 2. Performance Metrics Section
    print("PERFORMANCE METRICS", leftMargin, cursorY, glm::vec3(0.80f, 0.85f, 0.90f));
    cursorY -= 22.0f;

    print("FPS: " + std::to_string(static_cast<int>(std::round(fps))) + "  (" + formatFloat(currentMs, 2) + " ms)", leftMargin, cursorY, badgeColor);
    cursorY -= 20.0f;

    print("Avg: " + formatFloat(avgMs, 2) + " ms | Min: " + formatFloat(minMs, 1) + " | Max: " + formatFloat(maxMs, 1) + " ms", leftMargin, cursorY, glm::vec3(0.70f, 0.75f, 0.80f));
    cursorY -= 20.0f;

    print("Target: 60.0 FPS (16.67 ms)", leftMargin, cursorY, glm::vec3(0.40f, 0.85f, 0.45f));
    cursorY -= 24.0f;

    print(divider, leftMargin, cursorY, glm::vec3(0.25f, 0.28f, 0.35f));
    cursorY -= 24.0f;

    // 3. Frame Time History Section
    print("FRAME TIME HISTORY (Recent 50 Frames)", leftMargin, cursorY, glm::vec3(0.80f, 0.85f, 0.90f));
    cursorY -= 22.0f;

    glm::vec3 stabilityColor = (at60Count >= 38)
        ? glm::vec3(0.40f, 0.85f, 0.45f)
        : ((at60Count >= 18) ? glm::vec3(0.95f, 0.75f, 0.15f) : glm::vec3(0.95f, 0.25f, 0.25f));

    print("Stability: " + std::to_string(at60Count) + "/" + std::to_string(streamCount) + " frames at 60 FPS (" + std::to_string((at60Count * 100) / streamCount) + "%)", leftMargin, cursorY, stabilityColor);
    cursorY -= 20.0f;

    print("History: [" + historyStream + "]", leftMargin, cursorY, badgeColor);
    cursorY -= 20.0f;

    print(divider, leftMargin, cursorY, glm::vec3(0.25f, 0.28f, 0.35f));
    cursorY -= 24.0f;

    // 4. Scene & ECS Breakdown Section
    print("SCENE & ECS BREAKDOWN", leftMargin, cursorY, glm::vec3(0.80f, 0.85f, 0.90f));
    cursorY -= 22.0f;

    print("Active Scene: " + primarySceneName + " (" + std::to_string(activeScenesCount) + " active)", leftMargin, cursorY, glm::vec3(0.90f, 0.92f, 0.95f));
    cursorY -= 20.0f;

    print("Total Alive Entities: " + std::to_string(totalEntities), leftMargin, cursorY, glm::vec3(0.75f, 0.85f, 0.95f));
    cursorY -= 20.0f;

    print("Mesh Renderers (Draw Calls): " + std::to_string(totalMeshes), leftMargin, cursorY, glm::vec3(0.95f, 0.80f, 0.45f));
    cursorY -= 20.0f;

    print("Dynamic Point Lights: " + std::to_string(totalLights) + " | Cameras: " + std::to_string(totalCameras), leftMargin, cursorY, glm::vec3(0.90f, 0.65f, 0.85f));
    cursorY -= 24.0f;

    print(divider, leftMargin, cursorY, glm::vec3(0.25f, 0.28f, 0.35f));
    cursorY -= 24.0f;
}
