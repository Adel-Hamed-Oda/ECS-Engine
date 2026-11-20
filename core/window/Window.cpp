#include "core/window/Window.h"
#include "Engine.h"
#include "core/private_imports.h"

std::vector<Window*> Window::AllWindows;
int Window::nextId = 0;

Window* Window::GetPrimaryWindow() {
    for (Window* win : AllWindows) {
        if (win && !win->m_isDebugWindow && win->IsOpen()) {
            return win;
        }
    }
    return nullptr;
}

Window* Window::Get(int id) {
    for (Window* win : AllWindows) {
        if (win && win->id == id) return win;
    }
    return nullptr;
}

const std::vector<Window*>& Window::GetAllWindows() {
    return AllWindows;
}

Window::Window(const char* title, glm::ivec2 position, glm::ivec2 size, Window* shareWindow, bool isDebug) {
    Engine::init();

    id = nextId++;
    m_isDebugWindow = isDebug;

    // Share OpenGL resources (textures, buffers, shaders) across windows
    GLFWwindow* shareContext = nullptr;
    if (shareWindow && shareWindow->m_window) {
        shareContext = shareWindow->m_window;
    } else if (Window* primary = GetPrimaryWindow()) {
        shareContext = primary->m_window;
    } else if (!AllWindows.empty() && AllWindows[0]->m_window) {
        shareContext = AllWindows[0]->m_window;
    }

    m_window = glfwCreateWindow(size.x, size.y, title, nullptr, shareContext);
    if (m_window) {
        glfwSetWindowPos(m_window, position.x, position.y);
        AllWindows.push_back(this);

        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1);
        gladLoadGL((GLADloadfunc)glfwGetProcAddress);
        Renderer::SetupRendering();
        if (glad_glClearColor) {
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        }
        glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
    } else {
        std::cerr << "Failed to create GLFW window: " << title << std::endl;
    }
}

Window::~Window() {
    Close();
}

void Window::Close() {
    if (!m_window) return;

    auto it = std::remove(AllWindows.begin(), AllWindows.end(), this);
    AllWindows.erase(it, AllWindows.end());

    GLFWwindow* windowToDestroy = m_window;
    m_window = nullptr;

    onWindowClose.Invoke();

    if (glfwGetCurrentContext() == windowToDestroy) {
        glfwMakeContextCurrent(nullptr);
    }
    glfwDestroyWindow(windowToDestroy);

    if (Window* primary = GetPrimaryWindow()) {
        if (primary->m_window) {
            glfwMakeContextCurrent(primary->m_window);
        }
    } else if (!AllWindows.empty() && AllWindows[0]->m_window) {
        glfwMakeContextCurrent(AllWindows[0]->m_window);
    }

    // When the other (game) windows are closed, ensure the debug window is closed as well
    if (!m_isDebugWindow) {
        bool anyGameWindowOpen = false;
        for (Window* win : AllWindows) {
            if (win && win->IsOpen() && !win->m_isDebugWindow) {
                anyGameWindowOpen = true;
                break;
            }
        }
        if (!anyGameWindowOpen) {
            DebugWindow::CloseAll();
        }
    }
}

bool Window::IsOpen() const {
    return m_window != nullptr;
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

glm::ivec2 Window::GetFramebufferSize() const {
    if (!m_window) return glm::ivec2(0, 0);
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
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
    if (m_isDebugWindow) return; // Debug window cannot become primary

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
    if (m_window && glfwGetCurrentContext() != m_window) {
        glfwMakeContextCurrent(m_window);
    }
}

void Window::SwapBuffers() {
    if (m_window) glfwSwapBuffers(m_window);
}

Scene* Window::GetScene() const {
    return m_scene;
}

void Window::SetScene(Scene* scene) {
    m_scene = scene;
}

void Window::SetScene(const std::string& sceneName) {
    m_scene = Scene::Get(sceneName);
}

Entity Window::GetCamera() const {
    return m_assignedCamera;
}

void Window::SetCamera(Entity camera) {
    m_assignedCamera = camera;
}

bool Window::IsDebugWindow() const {
    return m_isDebugWindow;
}

void Window::Render() {
    glm::ivec2 fbSize = GetFramebufferSize();
    if (fbSize.x <= 0 || fbSize.y <= 0) return;

    if (glad_glViewport) {
        glViewport(0, 0, fbSize.x, fbSize.y);
    }

    if (glad_glClear) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if (Scene* targetScene = GetScene()) {
        if (targetScene->GetState() == SceneState::Unloaded) {
            targetScene->Load();
        }
        Renderer::RenderScene(targetScene, fbSize.x, fbSize.y, GetCamera());
    }
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (!glad_glViewport || width <= 0 || height <= 0) return;

    GLFWwindow* previousContext = glfwGetCurrentContext();
    if (previousContext != window) {
        glfwMakeContextCurrent(window);
    }

    glViewport(0, 0, width, height);

    if (previousContext != window) {
        glfwMakeContextCurrent(previousContext);
    }
}
