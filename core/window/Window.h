#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "core/ecs/Entity.h"
#include "core/events/Event.h"

// Forward declaration of GLFWwindow to avoid exposing GLFW headers to users
struct GLFWwindow;
class Scene;

class Window
{
    friend class Engine;
    friend class DebugWindow;

public:
    Event<Window> onWindowClose;

    static Window* GetPrimaryWindow(); // used to ensure scenes don't load on empty contexts
    static Window* Get(int id);
    static const std::vector<Window*>& GetAllWindows();

    Window(
        const char* title = "New window",
        glm::ivec2 position = glm::ivec2(0, 0),
        glm::ivec2 size = glm::ivec2(1920, 1080),
        Window* shareWindow = nullptr,
        bool isDebug = false
    );

    virtual ~Window();

    virtual void Close();
    bool IsOpen() const;

    int GetId() const;
    bool GLWindowShouldClose() const;

    glm::ivec2 GetPosition() const;
    glm::ivec2 GetSize() const;
    std::string GetTitle() const;

    void SetPosition(int x, int y);
    void SetSize(int width = -1, int height = -1);
    void SetTitle(const std::string& title);
    virtual void SetAsPrimaryWindow();

    Scene* GetScene() const;
    virtual void SetScene(Scene* scene);
    virtual void SetScene(const std::string& sceneName);

    Entity GetCamera() const;
    virtual void SetCamera(Entity camera);

    bool IsDebugWindow() const;
    virtual void Render();

protected:
    static std::vector<Window*> AllWindows;
    static int nextId;

    int id = 0;
    bool m_isDebugWindow = false;
    GLFWwindow* m_window = nullptr;
    Scene* m_scene = nullptr;
    Entity m_assignedCamera = NULL_ENTITY;

    glm::ivec2 GetFramebufferSize() const;
    void MakeContextCurrent();
    void SwapBuffers();

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};

class DebugWindow : public Window
{
    friend class Engine;

public:
    static DebugWindow* Open(
        glm::ivec2 position = glm::ivec2(40, 40),
        glm::ivec2 size = glm::ivec2(720, 580),
        Window* shareWindow = nullptr,
        bool lightweight = false
    );
    static DebugWindow* Open(bool lightweight);
    static void CloseAll();
    static DebugWindow* GetInstance();
    static bool IsOpenStatic();

    DebugWindow(
        glm::ivec2 position = glm::ivec2(40, 40),
        glm::ivec2 size = glm::ivec2(720, 580),
        Window* shareWindow = nullptr,
        bool lightweight = false
    );
    ~DebugWindow() override;

    bool IsLightweight() const { return m_lightweight; }
    void SetLightweight(bool lightweight);

    void Close() override;
    void SetScene(Scene* scene) override;
    void SetScene(const std::string& sceneName) override;
    void SetCamera(Entity camera) override;
    void SetAsPrimaryWindow() override;
    void Render() override;

private:
    static DebugWindow* s_instance;
    bool m_lightweight = false;
    void RenderDebugContent(int width, int height);
};
