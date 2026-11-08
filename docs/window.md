# Window Management

The `Window` class ([core/window/Window.h](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/window/Window.h)) encapsulates the GLFW native window, OpenGL context binding, framebuffer callbacks, and multi-window registration.

---

## Class API Reference

### Constructor & Destructor

```cpp
Window(
    const char* title = "New window",
    glm::ivec2 position = glm::ivec2(0, 0),
    glm::ivec2 size = glm::ivec2(1920, 1080)
);

~Window();
```

- **`title`**: Window title displayed on the title bar.
- **`position`**: Initial screen coordinates `(x, y)` for the top-left corner of the window.
- **`size`**: Width and height `(width, height)` in screen coordinates.
- **Behavior**:
  - Automatically initializes GLFW if not already initialized.
  - Creates a windowed GLFW window (`nullptr` monitor).
  - Automatically sets itself as the primary active window and binds the OpenGL context current.
  - Automatically loads and initializes GLAD function pointers.
  - Registers resizing callbacks (`framebuffer_size_callback`) to update `glViewport`.
  - Pushes the window into the static `AllWindows` registry.
- **Destructor**:
  - Automatically removes itself from `AllWindows`.
  - Destroys the underlying `GLFWwindow*`.
  - Re-binds the next available window context if one exists.

---

## Methods

### Window State & Information

| Method | Return Type | Description |
| :--- | :--- | :--- |
| `GetId()` | `int` | Returns the unique numeric ID assigned to this window instance. |
| `GetPosition()` | `glm::ivec2` | Retrieves the current screen coordinates of the window's top-left corner. |
| `GetSize()` | `glm::ivec2` | Retrieves the current width and height of the window's content area. |
| `GetTitle()` | `std::string` | Returns the window's current title text. |
| `GLWindowShouldClose()` | `bool` | Returns `true` if the user clicked the window's close (X) button. |
| `GetGLFWWindow()` | `GLFWwindow*` | Returns raw pointer to the underlying GLFW window handle. |

### Window Modification

| Method | Parameters | Description |
| :--- | :--- | :--- |
| `SetPosition(int x, int y)` | `int x, int y` | Moves the window to the specified screen coordinates. |
| `SetSize(int width, int height)`| `int width = -1, int height = -1` | Resizes the window. Pass `-1` to retain current axis size. |
| `SetTitle(const std::string&)` | `const std::string& title` | Updates the window's title string. |
| `SetAsPrimaryWindow()` | *None* | Makes this window the primary window (`AllWindows[0]`) and binds its OpenGL context. |
| `SwapBuffers()` | *None* | Swaps front and back color buffers to present the rendered image. |

### Static Helpers

| Static Method | Return Type | Description |
| :--- | :--- | :--- |
| `Window::GetPrimaryWindow()` | `Window*` | Returns the current primary active window, or `nullptr` if no windows exist. |
| `Window::Get(int id)` | `Window*` | Looks up a window pointer by its unique ID. Returns `nullptr` if not found. |

---

## Multi-Window Support

Multiple windows can be created and managed simultaneously:

```cpp
// Create primary view
Window* mainWin = new Window("Main View", glm::ivec2(50, 50), glm::ivec2(800, 600));

// Create secondary debug view
Window* debugWin = new Window("Debug Inspector", glm::ivec2(860, 50), glm::ivec2(400, 600));

// Switch rendering context to debug window
debugWin->SetAsPrimaryWindow();
glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT);
debugWin->SwapBuffers();

// Switch back to main window
mainWin->SetAsPrimaryWindow();
```
