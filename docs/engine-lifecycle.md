# Engine Lifecycle

The `Engine` class ([Engine.h](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/Engine.h), [Engine.cpp](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/Engine.cpp)) manages the high-level application lifecycle, window coordination, event polling, timing updates, timer/tween dispatching, scene updates, and multi-window rendering.

---

## Class Overview

```cpp
class Engine {
public:
    static void init();
    static void run();
    static void terminate();

private:
    inline static bool initialized = false;
    inline static bool terminated = false;
    inline static float deltaTime = 0.0f;

    static void SetupEngine();
    static void MainLoop();
    static void UpdateScenes();
    static void RenderWindows();
};
```

---

## Public Methods

### `Engine::init()`
```cpp
static void init();
```
Initializes subsystem dependencies (such as GLFW and OpenGL hints).
- Safe to call multiple times; returns immediately if already initialized.
- Initializes GLFW (`glfwInit()`).
- Sets OpenGL window hints for version 4.0 Core Profile.
- If an OpenGL context is currently active, initializes GLAD function pointers.
- Resets the `terminated` flag to `false` and sets `initialized = true`.
- Called automatically by `Window` constructors and `Engine::run()`.

### `Engine::run()`
```cpp
static void run();
```
Prepares the engine via `SetupEngine()` and enters the blocking `MainLoop()`.
- Runs continuously until all windows close or `Engine::terminate()` is invoked.
- Drives frame timing, OS event polling, timer updates, tween interpolation, scene updates, and window rendering.

### `Engine::terminate()`
```cpp
static void terminate();
```
Gracefully shuts down the engine and releases all allocated resources:
1. Sets `terminated = true`, breaking out of `MainLoop()`.
2. Unloads all active scenes via [`Scene::Unload()`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/scenes/Scene.h) while OpenGL contexts remain valid.
3. Closes all open windows via [`Window::Close()`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/window/Window.h).
4. Clears all active timers via [`Timer::ClearAll()`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/anim/Timer.h).
5. Clears all active tweens via [`Tween::ClearAll()`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/anim/Tween.h).
6. Terminates GLFW via `glfwTerminate()`.
7. Sets `initialized = false`.

---

## The Execution Flow

```
+-------------------------------------------------+
|                   main()                        |
|   - Instantiate Window(s) and Scene(s)          |
|   - Bind scenes to windows: win.SetScene(&scene)|
+-------------------------------------------------+
                        |
                        v
+-------------------------------------------------+
|               Engine::run()                     |
+-------------------------------------------------+
                        |
                        v
+-------------------------------------------------+
|           Engine::SetupEngine()                 |
|  - Calls Engine::init()                         |
|  - Ensures GLFW & OpenGL 4.0 Core hints ready   |
+-------------------------------------------------+
                        |
                        v
+-------------------------------------------------+
|            Engine::MainLoop()                   |
|  While (!terminated):                           |
|    1. Time::Update(&lastFrameTime, currentTime) |
|    2. glfwPollEvents()                          |
|    3. Poll windows requesting close -> win->Close()
|    4. If windows existed & all closed -> break  |
|                                                 |
|    [ UPDATE PHASE ]                             |
|    5. Timer::UpdateAll()                        |
|    6. Tween::UpdateAll()                        |
|    7. UpdateScenes()                            |
|         - Collect unique scenes from windows    |
|           (or all scenes if in headless mode)   |
|         - If scene unloaded: scene->Load()      |
|         - scene->Update()                       |
|                                                 |
|    [ RENDER PHASE ]                             |
|    8. RenderWindows()                           |
|         - For each open window:                 |
|             * Set OpenGL context                |
|             * Update glViewport & glClear       |
|             * scene->Render()                   |
|             * glfwSwapInterval & SwapBuffers    |
|         - If no window presented: sleep 16ms    |
+-------------------------------------------------+
                        |
                        v
+-------------------------------------------------+
|            Engine::terminate()                  |
|  - scene->Unload() for all loaded scenes        |
|  - win->Close() for all open windows            |
|  - Timer::ClearAll() & Tween::ClearAll()        |
|  - glfwTerminate()                              |
+-------------------------------------------------+
```

---

## Update & Render Mechanics

### Unique Scene Updates
Windows reference target scenes via [`Window::SetScene()`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/window/Window.h). During `Engine::UpdateScenes()`:
- The engine collects unique scenes across all currently open windows into a `std::unordered_set<Scene*>`.
- If multiple windows display the same scene (e.g., dual viewports or split-screen), that scene's logic is updated **only once per frame**.
- If a scene is in `SceneState::Unloaded`, the engine automatically invokes `scene->Load()` before calling `scene->Update()`.

### Multi-Window Frame Pacing (VSync)
In `Engine::RenderWindows()`:
- OpenGL context is switched per window (`win->MakeContextCurrent()`).
- Viewports and color/depth buffers are cleared for each active window.
- The target scene's `Render()` is invoked.
- **VSync Pacing**: To avoid frame rate dividing by $N$ across $N$ windows, `glfwSwapInterval(1)` is set only on the first presented window; subsequent windows use `glfwSwapInterval(0)`.

---

## Headless Mode & CPU Throttling

The engine fully supports running without any open windows:

- **Server & Background Applications**: If no `Window` is created, `UpdateScenes()` automatically iterates and updates all instantiated `Scene` objects.
- **CPU Throttling**: When running headless or when all windows are minimized (zero framebuffer area), `RenderWindows()` sleeps for `16ms` (`std::this_thread::sleep_for(std::chrono::milliseconds(16))`), maintaining ~60 ticks/sec and keeping CPU usage near `0%`.
- **Dynamic Window Lifetimes**: Windows can be opened and closed dynamically during execution. If all open windows are closed by the user, the main loop exits and `Engine::terminate()` runs automatically.
