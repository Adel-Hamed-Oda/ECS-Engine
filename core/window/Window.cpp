#include "core/window/Window.h"
#include "core/private_imports.h"

std::vector<Window*> Window::AllWindows;
int Window::nextId = 0;

Window* Window::GetPrimaryWindow() {
    return AllWindows.empty() ? nullptr : AllWindows[0];
}

Window* Window::Get(int id) {
    for (Window* win : AllWindows) {
        if (win && win->id == id) return win;
    }
    return nullptr;
}

Window::Window(const char* title, glm::ivec2 position, glm::ivec2 size) {
    id = nextId++;

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

Window::~Window() {
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

int Window::GetId() const {
    return id;
}

bool Window::GLWindowShouldClose() const {
    return !m_window || glfwWindowShouldClose(m_window);
}

glm::ivec2 Window::GetPosition() const {
    if (!m_window) return glm::ivec2(0, 0);
    int x, y;
    glfwGetWindowPos(m_window, &x, &y);
    return glm::ivec2(x, y);
}

glm::ivec2 Window::GetSize() const {
    if (!m_window) return glm::ivec2(0, 0);
    int width, height;
    glfwGetWindowSize(m_window, &width, &height);
    return glm::ivec2(width, height);
}

std::string Window::GetTitle() const {
    if (!m_window) return "";
    const char* title = glfwGetWindowTitle(m_window);
    return title ? std::string(title) : "";
}

void Window::SetPosition(int x, int y) {
    if (m_window) glfwSetWindowPos(m_window, x, y);
}

void Window::SetSize(int width, int height) {
    if (!m_window) return;
    glm::ivec2 size = GetSize();
    if (width == -1) width = size.x;
    if (height == -1) height = size.y;
    glfwSetWindowSize(m_window, width, height);
}

void Window::SetTitle(const std::string& title) {
    if (m_window) glfwSetWindowTitle(m_window, title.c_str());
}

void Window::SetAsPrimaryWindow() {
    auto it = std::find(AllWindows.begin(), AllWindows.end(), this);
    if (it != AllWindows.end()) {
        std::iter_swap(AllWindows.begin(), it);
    }
    if (m_window) {
        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1);
    }
}

void Window::MakeContextCurrent() {
    if (m_window) {
        glfwMakeContextCurrent(m_window);
    }
}

void Window::SwapBuffers() {
    if (m_window) glfwSwapBuffers(m_window);
}

GLFWwindow* Window::GetGLFWWindow() const {
    return m_window;
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (glad_glViewport) glViewport(0, 0, width, height);
}
