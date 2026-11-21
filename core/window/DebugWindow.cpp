#include "Engine.h"

DebugWindow* DebugWindow::s_instance = nullptr;

DebugWindow* DebugWindow::Open(glm::ivec2 position, glm::ivec2 size, Window* shareWindow, bool lightweight) {
    if (s_instance && s_instance->IsOpen()) {
        s_instance->SetLightweight(lightweight);
        return s_instance;
    }

    Window* shareContext = shareWindow ? shareWindow : Window::GetPrimaryWindow();
    s_instance = new DebugWindow(position, size, shareContext, lightweight);
    return s_instance;
}

DebugWindow* DebugWindow::Open(bool lightweight) {
    glm::ivec2 defaultSize = lightweight ? glm::ivec2(380, 160) : glm::ivec2(720, 580);
    return Open(glm::ivec2(40, 40), defaultSize, nullptr, lightweight);
}

void DebugWindow::CloseAll() {
    if (s_instance) {
        DebugWindow* win = s_instance;
        s_instance = nullptr;
        Profiler::SetEnabled(false);
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

DebugWindow::DebugWindow(glm::ivec2 position, glm::ivec2 size, Window* shareWindow, bool lightweight)
    : Window(lightweight ? "Debug (Lightweight)" : "Debug", position, size, shareWindow ? shareWindow : Window::GetPrimaryWindow(), true),
      m_lightweight(lightweight)
{
    s_instance = this;
    // Only enable full engine-wide profiling when NOT in lightweight mode
    Profiler::SetEnabled(!lightweight);

    // Restore active context to primary game window so creating DebugWindow doesn't steal focus/input
    if (Window* primary = Window::GetPrimaryWindow()) {
        if (primary != this && primary->IsOpen()) {
            primary->MakeContextCurrent();
        }
    }
}

void DebugWindow::SetLightweight(bool lightweight) {
    if (m_lightweight == lightweight) return;
    m_lightweight = lightweight;
    Profiler::SetEnabled(!lightweight);
    SetTitle(lightweight ? "Debug (Lightweight)" : "Debug");
}

DebugWindow::~DebugWindow() {
    if (s_instance == this) {
        s_instance = nullptr;
    }
    Profiler::SetEnabled(false);
}

void DebugWindow::Close() {
    if (s_instance == this) {
        s_instance = nullptr;
    }
    Profiler::SetEnabled(false);
    Window::Close();

    if (Window* primary = Window::GetPrimaryWindow()) {
        if (primary->IsOpen()) {
            primary->MakeContextCurrent();
        }
    }
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
    int at60Count = 0;
    const size_t streamCount = 50;

    for (size_t i = 0; i < history.size(); ++i) {
        float ms = history[i];
        sumMs += ms;
        if (ms < minMs) minMs = ms;
        if (ms > maxMs) maxMs = ms;
        if (i < streamCount && ms <= 18.0f) {
            at60Count++;
        }
    }
    float avgMs = sumMs / static_cast<float>(history.size());
    float fps = (avgMs > 0.001f) ? (1000.0f / avgMs) : 0.0f;
    float currentFps = (currentMs > 0.001f) ? (1000.0f / currentMs) : 0.0f;

    // Health color coding
    glm::vec3 badgeColor;
    if (fps >= 50.0f) {
        badgeColor = glm::vec3(0.25f, 0.85f, 0.40f); // Green
    } else if (fps >= 30.0f) {
        badgeColor = glm::vec3(0.95f, 0.75f, 0.15f); // Yellow
    } else {
        badgeColor = glm::vec3(0.95f, 0.25f, 0.25f); // Red
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

    // High-performance string formatting using stack buffer (no std::locale allocations)
    auto formatFloat = [](float val, int precision) -> std::string {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.*f", precision, static_cast<double>(val));
        return std::string(buf);
    };

    static Transform2D s_textTransform;
    auto print = [&](const std::string& str, float tx, float ty, const glm::vec3& col) {
        if (!debugText || !debugFont || !debugFont->IsValid() || str.empty()) return;
        debugText->text = str;
        if (debugMaterial) debugMaterial->textColor = col;
        s_textTransform.position = glm::vec2(tx, ty);
        s_textTransform.scale = glm::vec2(1.0f, 1.0f);
        debugText->Draw(&s_textTransform, ortho, col);
    };

    static std::unique_ptr<ImageRenderer> s_rectRenderer = nullptr;
    if (!s_rectRenderer) {
        s_rectRenderer = std::make_unique<ImageRenderer>();
    }

    static Transform2D s_quadTransform;
    auto drawRect = [&](float rx, float ry, float rw, float rh, const glm::vec4& col) {
        if (!s_rectRenderer || rw <= 0.0f || rh <= 0.0f) return;
        s_quadTransform.position = glm::vec2(rx, ry);
        s_quadTransform.scale = glm::vec2(rw, rh);
        s_rectRenderer->Draw(&s_quadTransform, ortho, col);
    };

    const float fHeight = static_cast<float>(height);
    const float leftMargin = 28.0f;

    auto drawDivider = [&](float dy) {
        float divW = static_cast<float>(width) - leftMargin * 2.0f;
        drawRect(leftMargin, dy, divW, 1.0f, glm::vec4(0.20f, 0.24f, 0.32f, 0.85f));
    };

    // ========================================================================
    // LIGHTWEIGHT MODE: Inexpensive checks only (FPS, Avg FPS, Min/Max ms)
    // Zero profiler overhead, zero software pixel rasterization, zero texture uploads
    // ========================================================================
    if (m_lightweight) {
        float cursorY = fHeight - 26.0f;

        print("DEBUG [LIGHTWEIGHT MODE]", leftMargin, cursorY, glm::vec3(0.45f, 0.75f, 1.0f));
        print("Inexpensive checks only", leftMargin + 200.0f, cursorY, glm::vec3(0.50f, 0.55f, 0.65f));
        cursorY -= 12.0f;
        drawDivider(cursorY);
        cursorY -= 14.0f;

        // Current FPS & frame time
        print("Current: " + formatFloat(currentFps, 1) + " FPS  (" + formatFloat(currentMs, 2) + " ms)", leftMargin, cursorY, badgeColor);
        print("Target: 60.0 FPS (16.67 ms)", leftMargin + 200.0f, cursorY, glm::vec3(0.40f, 0.85f, 0.45f));
        cursorY -= 16.0f;

        // Average FPS & frame time
        print("Average: " + formatFloat(fps, 1) + " FPS  (" + formatFloat(avgMs, 2) + " ms)", leftMargin, cursorY, glm::vec3(0.90f, 0.92f, 0.95f));
        cursorY -= 16.0f;

        // Min & Max frame times
        print("Min: " + formatFloat(minMs, 1) + " ms  |  Max: " + formatFloat(maxMs, 1) + " ms", leftMargin, cursorY, glm::vec3(0.70f, 0.75f, 0.80f));

        glm::vec3 stabilityColor = (at60Count >= 38)
            ? glm::vec3(0.40f, 0.85f, 0.45f)
            : ((at60Count >= 18) ? glm::vec3(0.95f, 0.75f, 0.15f) : glm::vec3(0.95f, 0.25f, 0.25f));
        print("Stability: " + std::to_string((at60Count * 100) / streamCount) + "% @ 60 FPS", leftMargin + 200.0f, cursorY, stabilityColor);

        cursorY -= 10.0f;
        drawDivider(cursorY);
        return;
    }

    float cursorY = fHeight - 32.0f;

    // 1. Header Section
    print("ENGINE PERFORMANCE DEBUGGER", leftMargin, cursorY, glm::vec3(0.95f, 0.95f, 1.0f));
    cursorY -= 18.0f;
    print("Real-Time Diagnostic Profiler", leftMargin, cursorY, glm::vec3(0.45f, 0.65f, 0.90f));
    cursorY -= 8.0f;
    drawDivider(cursorY);
    cursorY -= 14.0f;

    // 2. Performance Metrics Section
    print("PERFORMANCE METRICS", leftMargin, cursorY, glm::vec3(0.80f, 0.85f, 0.90f));
    cursorY -= 18.0f;

    print("FPS: " + std::to_string(static_cast<int>(std::round(fps))) + "  (" + formatFloat(currentMs, 2) + " ms)", leftMargin, cursorY, badgeColor);
    print("Target: 60.0 FPS (16.67 ms)", leftMargin + 250.0f, cursorY, glm::vec3(0.40f, 0.85f, 0.45f));
    cursorY -= 17.0f;

    print("Avg: " + formatFloat(avgMs, 2) + " ms | Min: " + formatFloat(minMs, 1) + " | Max: " + formatFloat(maxMs, 1) + " ms", leftMargin, cursorY, glm::vec3(0.70f, 0.75f, 0.80f));
    cursorY -= 8.0f;
    drawDivider(cursorY);
    cursorY -= 14.0f;

    // 3. Engine Pipeline Breakdown (Color-Coded Timestamps & Rectangles)
    print("ENGINE PIPELINE BREAKDOWN (Section Timings)", leftMargin, cursorY, glm::vec3(0.80f, 0.85f, 0.90f));
    cursorY -= 18.0f;

    const ProfileSection* profilerSections = Profiler::GetSectionData();
    const size_t profilerCount = Profiler::GetActiveCount();
    double totalSmoothed = Profiler::GetSmoothedTotalMs();
    if (totalSmoothed < 0.001) totalSmoothed = 16.6;

    int timelineBarWidth = width - static_cast<int>(leftMargin * 2.0f);
    if (timelineBarWidth < 60) timelineBarWidth = 60;
    const float timelineBarHeight = 16.0f;

    cursorY -= timelineBarHeight;

    // 3a. Background panel and border for the composite timeline bar
    drawRect(leftMargin, cursorY, static_cast<float>(timelineBarWidth), timelineBarHeight, glm::vec4(0.08f, 0.10f, 0.14f, 0.95f));
    drawRect(leftMargin, cursorY, static_cast<float>(timelineBarWidth), 1.0f, glm::vec4(0.22f, 0.26f, 0.35f, 1.0f));
    drawRect(leftMargin, cursorY + timelineBarHeight - 1.0f, static_cast<float>(timelineBarWidth), 1.0f, glm::vec4(0.22f, 0.26f, 0.35f, 1.0f));
    drawRect(leftMargin, cursorY, 1.0f, timelineBarHeight, glm::vec4(0.22f, 0.26f, 0.35f, 1.0f));
    drawRect(leftMargin + static_cast<float>(timelineBarWidth) - 1.0f, cursorY, 1.0f, timelineBarHeight, glm::vec4(0.22f, 0.26f, 0.35f, 1.0f));

    // 3b. Fill with color-coded section rectangles defining how long each part took
    float currentSegX = leftMargin + 2.0f;
    const float innerTimelineWidth = static_cast<float>(timelineBarWidth) - 4.0f;

    for (size_t i = 0; i < profilerCount; ++i) {
        const auto& sec = profilerSections[i];
        if (sec.smoothedMs <= 0.0001) continue;
        float segW = static_cast<float>((sec.smoothedMs / totalSmoothed) * static_cast<double>(innerTimelineWidth));
        if (segW < 2.0f) segW = 2.0f;

        if (currentSegX + segW > leftMargin + 2.0f + innerTimelineWidth) {
            segW = (leftMargin + 2.0f + innerTimelineWidth) - currentSegX;
        }
        if (segW <= 0.0f) break;

        drawRect(currentSegX, cursorY + 2.0f, segW, timelineBarHeight - 4.0f, sec.color);
        currentSegX += segW + 1.0f;
    }

    cursorY -= 17.0f;

    // 3c. Detailed section list: Color Badge Rectangles, Labels, Timestamps (ms / %), and Horizontal Rectangles
    float secBarMax = static_cast<float>(width) - (leftMargin + 320.0f) - leftMargin;
    if (secBarMax < 30.0f) secBarMax = 30.0f;

    for (size_t i = 0; i < profilerCount; ++i) {
        const auto& sec = profilerSections[i];
        // Color badge rectangle
        drawRect(leftMargin, cursorY + 2.0f, 10.0f, 10.0f, sec.color);

        // Section name
        print(sec.name, leftMargin + 16.0f, cursorY, glm::vec3(sec.color));

        // Milliseconds timestamp and percentage
        std::string timeStr = formatFloat(static_cast<float>(sec.smoothedMs), 2) + " ms  (" +
                              formatFloat(static_cast<float>(sec.percentage), 1) + "%)";
        print(timeStr, leftMargin + 185.0f, cursorY, glm::vec3(0.90f, 0.92f, 0.95f));

        // Color-coded horizontal rectangle defining relative duration
        float secBarW = std::clamp(static_cast<float>((sec.smoothedMs / totalSmoothed) * static_cast<double>(secBarMax)), 2.0f, secBarMax);
        drawRect(leftMargin + 320.0f, cursorY + 3.0f, secBarW, 8.0f, sec.color);

        cursorY -= 16.0f;
    }

    cursorY -= 6.0f;
    drawDivider(cursorY);
    cursorY -= 14.0f;

    // 4. Frame Time History Section (Rendered using Texture created from raw pixel data & ImageRenderer)
    print("FRAME TIME HISTORY (Recent 60 Frames)", leftMargin, cursorY, glm::vec3(0.80f, 0.85f, 0.90f));

    glm::vec3 stabilityColor = (at60Count >= 38)
        ? glm::vec3(0.40f, 0.85f, 0.45f)
        : ((at60Count >= 18) ? glm::vec3(0.95f, 0.75f, 0.15f) : glm::vec3(0.95f, 0.25f, 0.25f));

    print("Stability: " + std::to_string(at60Count) + "/" + std::to_string(streamCount) + " at 60 FPS (" + std::to_string((at60Count * 100) / streamCount) + "%)", leftMargin + 320.0f, cursorY, stabilityColor);
    cursorY -= 18.0f;

    // Build raw RGBA pixel buffer for the bar chart
    int graphWidth = width - static_cast<int>(leftMargin * 2.0f);
    if (graphWidth < 60) graphWidth = 60;
    const int graphHeight = 44;
    cursorY -= static_cast<float>(graphHeight);

    static std::vector<unsigned char> s_graphPixels;
    const size_t neededPixelBytes = static_cast<size_t>(graphWidth * graphHeight * 4);
    if (s_graphPixels.size() != neededPixelBytes) {
        s_graphPixels.resize(neededPixelBytes);
    }

    auto setPixel = [&](int px, int py, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
        if (px < 0 || px >= graphWidth || py < 0 || py >= graphHeight) return;
        size_t idx = (static_cast<size_t>(py) * static_cast<size_t>(graphWidth) + static_cast<size_t>(px)) * 4;
        s_graphPixels[idx + 0] = r;
        s_graphPixels[idx + 1] = g;
        s_graphPixels[idx + 2] = b;
        s_graphPixels[idx + 3] = a;
    };

    // 4a. Fast fill graph background and borders
    for (size_t p = 0; p < neededPixelBytes; p += 4) {
        s_graphPixels[p + 0] = 16;
        s_graphPixels[p + 1] = 20;
        s_graphPixels[p + 2] = 26;
        s_graphPixels[p + 3] = 235;
    }
    for (int x = 0; x < graphWidth; ++x) {
        setPixel(x, 0, 48, 56, 68, 255);
        setPixel(x, graphHeight - 1, 48, 56, 68, 255);
    }
    for (int y = 0; y < graphHeight; ++y) {
        setPixel(0, y, 48, 56, 68, 255);
        setPixel(graphWidth - 1, y, 48, 56, 68, 255);
    }

    const float maxScaleMs = 45.0f;

    // 4b. 60 FPS reference line (16.67ms)
    int ref60Y = static_cast<int>((16.67f / maxScaleMs) * static_cast<float>(graphHeight - 4)) + 2;
    if (ref60Y > 1 && ref60Y < graphHeight - 1) {
        for (int x = 1; x < graphWidth - 1; ++x) {
            setPixel(x, ref60Y, 40, 200, 80, 200);
        }
    }

    // 4c. 30 FPS reference line (33.33ms, dotted)
    int ref30Y = static_cast<int>((33.33f / maxScaleMs) * static_cast<float>(graphHeight - 4)) + 2;
    if (ref30Y > 1 && ref30Y < graphHeight - 1) {
        for (int x = 1; x < graphWidth - 1; ++x) {
            if ((x % 4) != 0) {
                setPixel(x, ref30Y, 180, 150, 40, 120);
            }
        }
    }

    // 4d. Draw latency bars from frame time history
    const size_t numBars = history.size();
    const int usableW = graphWidth - 4;
    const float barStep = static_cast<float>(usableW) / static_cast<float>(numBars);

    for (size_t i = 0; i < numBars; ++i) {
        float ms = history[(hIndex + i) % history.size()];
        int barH = std::clamp(static_cast<int>((ms / maxScaleMs) * static_cast<float>(graphHeight - 4)), 1, graphHeight - 4);

        unsigned char br, bg, bb;
        if (ms <= 17.5f) {
            br = 45; bg = 215; bb = 95;   // Healthy Green (<= 60 FPS target)
        } else if (ms <= 34.0f) {
            br = 240; bg = 195; bb = 40;  // Moderate Yellow (30-60 FPS)
        } else {
            br = 240; bg = 65;  bb = 65;  // Slow/Spike Red (< 30 FPS)
        }

        int xStart = 2 + static_cast<int>(static_cast<float>(i) * barStep);
        int xNext = 2 + static_cast<int>(static_cast<float>(i + 1) * barStep);
        int xEnd = (xNext - xStart > 1) ? xNext - 1 : xNext;

        for (int bx = xStart; bx < xEnd; ++bx) {
            for (int by = 2; by <= 2 + barH; ++by) {
                setPixel(bx, by, br, bg, bb, 255);
            }
        }
    }

    // 4e. Create / update texture from raw pixel data and draw with ImageRenderer
    static std::shared_ptr<Texture> s_graphTexture = nullptr;
    static std::unique_ptr<ImageRenderer> s_graphRenderer = nullptr;

    if (!s_graphTexture) {
        s_graphTexture = Texture::CreateFromPixels(
            s_graphPixels.data(), graphWidth, graphHeight, 4,
            TextureWrapMode::CLAMP_TO_EDGE, TextureFilterMode::NEAREST
        );
    } else {
        s_graphTexture->UpdatePixels(s_graphPixels.data(), graphWidth, graphHeight, 4);
    }

    if (!s_graphRenderer) {
        s_graphRenderer = std::make_unique<ImageRenderer>(s_graphTexture);
    } else {
        s_graphRenderer->texture = s_graphTexture;
    }

    Transform2D graphTransform;
    graphTransform.position = glm::vec2(leftMargin, cursorY);
    graphTransform.scale = glm::vec2(static_cast<float>(graphWidth), static_cast<float>(graphHeight));
    s_graphRenderer->Draw(&graphTransform, ortho);

    cursorY -= 16.0f;
    drawDivider(cursorY);
    cursorY -= 14.0f;

    // 5. Scene & ECS Breakdown Section
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
            totalMeshes += static_cast<int>(reg.GetComponentCount<MeshRenderer>());
            totalLights += static_cast<int>(reg.GetComponentCount<LightSource>());
            totalCameras += static_cast<int>(reg.GetComponentCount<Camera>());
        }
    }

    Window* primary = Window::GetPrimaryWindow();
    std::string primaryTitle = primary ? primary->GetTitle() : "None";
    glm::ivec2 primaryRes = primary ? primary->GetSize() : glm::ivec2(0, 0);

    print("SCENE & ECS BREAKDOWN", leftMargin, cursorY, glm::vec3(0.80f, 0.85f, 0.90f));
    cursorY -= 18.0f;

    print("Active Scene: " + primarySceneName + " (" + std::to_string(activeScenesCount) + " active)", leftMargin, cursorY, glm::vec3(0.90f, 0.92f, 0.95f));
    cursorY -= 17.0f;
    print("Target Window: " + primaryTitle + " (" + std::to_string(primaryRes.x) + "x" + std::to_string(primaryRes.y) + ")", leftMargin + 320.0f, cursorY, glm::vec3(0.65f, 0.70f, 0.78f));
    cursorY -= 17.0f;

    print("Alive Entities: " + std::to_string(totalEntities), leftMargin, cursorY, glm::vec3(0.75f, 0.85f, 0.95f));
    print("Draw Calls: " + std::to_string(totalMeshes), leftMargin + 190.0f, cursorY, glm::vec3(0.95f, 0.80f, 0.45f));
    print("Point Lights: " + std::to_string(totalLights) + " | Cameras: " + std::to_string(totalCameras), leftMargin + 370.0f, cursorY, glm::vec3(0.90f, 0.65f, 0.85f));
    cursorY -= 8.0f;
    drawDivider(cursorY);
}
