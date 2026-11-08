#pragma once

#include "core/private_imports.h"

class Window
{
public:
    inline static Window* GetPrimaryWindow() { 
        return AllWindows.empty() ? nullptr : AllWindows[0]; 
    }

    Window(
        const char* title = "New window",
        glm::ivec2 position = glm::ivec2(0, 0),
        glm::ivec2 size = glm::ivec2(1920, 1080)
    ) {
        id = nextId++;

        GLFWmonitor* monitor = glfwGetPrimaryMonitor(); // Should be used when fullscreen is implemented
        m_window = glfwCreateWindow(size.x, size.y, title, nullptr, nullptr);
        if (m_window) {
            glfwSetWindowPos(m_window, position.x, position.y);
            AllWindows.push_back(this);
            SetAsPrimaryWindow();
            gladLoadGL((GLADloadfunc)glfwGetProcAddress);
            glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
        } else {
            std::cerr << "Failed to create GLFW window: " << title << std::endl;
        }
    }

    ~Window() {
        auto it = std::remove(AllWindows.begin(), AllWindows.end(), this);
        AllWindows.erase(it, AllWindows.end());

        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }

        if (!AllWindows.empty() && AllWindows[0]->m_window) {
            glfwMakeContextCurrent(AllWindows[0]->m_window);
        }
    }
	
    int GetId() const { return id; }
    bool GLWindowShouldClose() const { return !m_window || glfwWindowShouldClose(m_window); }

    glm::ivec2 GetPosition() const {
        if (!m_window) return glm::ivec2(0, 0);
        int x, y;
        glfwGetWindowPos(m_window, &x, &y);
        return glm::ivec2(x, y);
    }
    glm::ivec2 GetSize() const {
        if (!m_window) return glm::ivec2(0, 0);
        int width, height;
        glfwGetWindowSize(m_window, &width, &height);
        return glm::ivec2(width, height);
    }
    std::string GetTitle() const {
        if (!m_window) return "";
        const char* title = glfwGetWindowTitle(m_window);
        return title ? std::string(title) : "";
    }

    void SetPosition(int x, int y) { if (m_window) glfwSetWindowPos(m_window, x, y); }
    void SetSize(int width = -1, int height = -1) {
        glm::ivec2 size = GetSize();
        if (width == -1) width = size.x;
        if (height == -1) height = size.y;
        glfwSetWindowSize(m_window, width, height);
    }
    void SetTitle(const std::string& title) { if (m_window) glfwSetWindowTitle(m_window, title.c_str()); }
    void SetAsPrimaryWindow() { 
        if (AllWindows.size() > 0 && id < (int)AllWindows.size()) {
            std::swap(AllWindows[0], AllWindows[id]);
        }
        if (m_window) glfwMakeContextCurrent(m_window);
    }

    void SwapBuffers() { if (m_window) glfwSwapBuffers(m_window); }
    static Window* Get(int id) { return (id >= 0 && id < (int)AllWindows.size()) ? AllWindows[id] : nullptr; }

private:
    inline static std::vector<Window*> AllWindows = std::vector<Window*>();
    inline static int nextId = 0;
    int id = 0;
	
    GLFWwindow* m_window = nullptr;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        if (glad_glViewport) glViewport(0, 0, width, height);
    }
};
