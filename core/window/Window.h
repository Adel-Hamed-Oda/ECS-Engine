#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "core/ecs/Entity.h"

// Forward declaration of GLFWwindow to avoid exposing GLFW headers to users
struct GLFWwindow;
class Scene;

class Window
{
public:
    static Window* GetPrimaryWindow();
    static Window* Get(int id);
    static const std::vector<Window*>& GetAllWindows();

    Window(
        const char* title = "New window",
        glm::ivec2 position = glm::ivec2(0, 0),
        glm::ivec2 size = glm::ivec2(1920, 1080),
        Window* shareWindow = nullptr
    );

    ~Window();

    void Close();
    bool IsOpen() const;

    int GetId() const;
    bool GLWindowShouldClose() const;

    glm::ivec2 GetPosition() const;
    glm::ivec2 GetSize() const;
    glm::ivec2 GetFramebufferSize() const;
    std::string GetTitle() const;

    void SetPosition(int x, int y);
    void SetSize(int width = -1, int height = -1);
    void SetTitle(const std::string& title);
    void SetAsPrimaryWindow();

    void MakeContextCurrent();
    void SwapBuffers();

    // Danger, this function bites
    GLFWwindow* GetGLFWWindow() const;

    Scene* GetScene() const;
    void SetScene(Scene* scene);
    void SetScene(const std::string& sceneName);

    Entity GetCamera() const;
    void SetCamera(Entity camera);

private:
    static std::vector<Window*> AllWindows;
    static int nextId;

    int id = 0;
    GLFWwindow* m_window = nullptr;
    Scene* m_scene = nullptr;
    Entity m_assignedCamera = NULL_ENTITY;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};
