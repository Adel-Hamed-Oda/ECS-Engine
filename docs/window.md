# Window Management

The `Window` class ([core/window/Window.h](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/window/Window.h), [core/window/Window.cpp](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/window/Window.cpp)) provides complete windowing capabilities via GLFW, managing OpenGL context lifecycle, resource sharing, framebuffer sizing, scene binding, and close events.

---

## Class API Reference

### Constructor & Destructor

```cpp
Window(
    const char* title = "New window",
    glm::ivec2 position = glm::ivec2(0, 0),
    glm::ivec2 size = glm::ivec2(1920, 1080),
    Window* shareWindow = nullptr
);

~Window();
```

- **Parameters**:
  - `title`: The window title displayed in the OS title bar.
  - `position`: Initial screen coordinates `(x, y)` for the top-left corner.
  - `size`: Initial width and height `(width, height)` in screen pixels.
  - `shareWindow`: Optional pointer to another window with which to share OpenGL resources (textures, shaders, buffers). If omitted and windows already exist, the new window automatically shares resources with the primary window (`AllWindows[0]`).
- **Constructor Lifecycle**:
  1. Ensures `Engine::init()` is called.
  2. Allocates a unique incremental integer `id`.
  3. Resolves OpenGL context sharing.
  4. Creates the GLFW window handle.
  5. Positions the window on screen.
  6. Registers the instance in `Window::AllWindows`.
  7. Binds the context current, enables VSync (`glfwSwapInterval(1)`), loads GLAD pointers, sets default clear color (`0.1f, 0.1f, 0.1f, 1.0f`), and registers the framebuffer resizing callback.
- **Destructor**:
  - Calls `Close()`, triggering `onWindowClose` and freeing the GLFW window handle safely.

---

## Public Events

### `onWindowClose`
```cpp
Event<Window> onWindowClose;
```
Fired when the window is closed via [`Close()`](#window-lifecycle) or during window destruction. External subscribers can attach callbacks:
```cpp
Window window("Game View");
window.onWindowClose += [&]() {
    std::cout << "Window is closing! Saving game state..." << std::endl;
};
```

---

## Methods

### Window Lifecycle

| Method | Return Type | Description |
| :--- | :--- | :--- |
| `Close()` | `void` | Removes the window from `AllWindows`, invokes `onWindowClose`, and destroys the GLFW handle. Rebinds the remaining primary context if available. |
| `IsOpen()` | `bool` | Returns `true` if the window has an active GLFW window handle. |
| `GLWindowShouldClose()` | `bool` | Returns `true` if the user has requested to close the window (e.g. clicking the close 'X' button) or if the window handle is null. |

### Window State & Dimensions

| Method | Return Type | Description |
| :--- | :--- | :--- |
| `GetId()` | `int` | Returns the unique numeric ID assigned to this window instance. |
| `GetPosition()` | `glm::ivec2` | Returns the current screen coordinates of the window's top-left corner. |
| `GetSize()` | `glm::ivec2` | Returns the window width and height. |
| `GetTitle()` | `std::string` | Returns the current window title string. |
| `SetPosition(int x, int y)` | `void` | Moves the window to the specified screen coordinates. |
| `SetSize(int width, int height)` | `void` | Resizes the window. Pass `-1` for width or height to preserve the current dimension. |
| `SetTitle(const std::string&)` | `void` | Updates the window's title in the OS title bar. |
| `SetAsPrimaryWindow()` | `void` | Swaps this window to the first position (`AllWindows[0]`), makes its OpenGL context current, and enables VSync. |

### Scene & Camera Assignment

| Method | Parameters / Return | Description |
| :--- | :--- | :--- |
| `GetScene()` | `Scene*` | Returns the scene currently bound to this window (or `nullptr`). |
| `SetScene(Scene* scene)` | `Scene* scene` | Binds a scene pointer to be rendered into this window's viewport. |
| `SetScene(const std::string&)` | `const std::string& name` | Looks up a scene by name via [`Scene::Get()`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/scenes/Scene.h) and assigns it. |
| `GetCamera()` | `Entity` | Returns the camera entity assigned to this window viewport. |
| `SetCamera(Entity camera)` | `Entity camera` | Associates an ECS camera entity with this window. |

### Static Queries

| Static Method | Return Type | Description |
| :--- | :--- | :--- |
| `Window::GetPrimaryWindow()` | `Window*` | Returns the primary active window (`AllWindows[0]`), or `nullptr` if no windows exist. |
| `Window::Get(int id)` | `Window*` | Returns the window matching the given integer `id`, or `nullptr`. |
| `Window::GetAllWindows()` | `const std::vector<Window*>&` | Returns a const reference to all currently active windows. |

---

## Multi-Window & Shared Context Example

Windows in the engine automatically share OpenGL resources. Here is an example of running two windows displaying separate scenes and observing window close events:

```cpp
#include <Engine.h>
#include <iostream>

int main() {
    // 1. Create two windows side by side
    Window win1("Main View", glm::ivec2(100, 100), glm::ivec2(640, 480));
    Window win2("Inspector", glm::ivec2(760, 100), glm::ivec2(400, 480));

    // 2. Create scenes
    Scene gameScene("GameScene");
    Scene toolScene("ToolScene");

    // 3. Bind scenes to windows
    win1.SetScene(&gameScene);
    win2.SetScene(&toolScene);

    // 4. Respond to window close events
    win1.onWindowClose += [&]() {
        std::cout << "Primary window closed. Unloading game scene..." << std::endl;
        gameScene.Unload();
    };

    win2.onWindowClose += [&]() {
        std::cout << "Inspector window closed." << std::endl;
        toolScene.Unload();
    };

    // 5. Run the engine loop
    Engine::run();

    return 0;
}
```
