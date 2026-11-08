# Engine Lifecycle

The `Engine` class ([Engine.h](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/Engine.h)) manages the high-level application lifecycle, window coordination, event polling, and main loop execution.

---

## Class Overview

```cpp
class Engine {
public:
    static void run();
    static void terminate();

private:
    inline static bool terminated = false;
    static void SetupEngine();
    static void MainLoop();
};
```

---

## Public Methods

### `Engine::run()`
```cpp
static void run();
```
Initializes subsystem dependencies (such as GLFW and OpenGL context configurations) via `SetupEngine()` and enters `MainLoop()`. This function blocks until `Engine::terminate()` is invoked or the application is instructed to close.

### `Engine::terminate()`
```cpp
static void terminate();
```
Signals the engine to cleanly stop running. On termination:
1. Sets the internal `terminated` flag to `true`, ending `MainLoop()`.
2. Destroys GLFW context allocations and cleans up internal resources via `glfwTerminate()`.
3. Safe to call from any system, input callback, or scene logic to initiate a shutdown.

---

## The Execution Flow

```
+-------------------------------------------------+
|                   main()                        |
|   (Optional: create custom Window instance)     |
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
|  - Initializes GLFW subsystem                   |
|  - Sets OpenGL Core profile version hints (4.0) |
|  - Evaluates existing windows / creates default |
+-------------------------------------------------+
                        |
                        v
+-------------------------------------------------+
|            Engine::MainLoop()                   |
|  While (!terminated):                           |
|    1. glfwPollEvents()                          |
|    2. Update systems & logic                    |
|    3. If Window exists:                         |
|         - Check if window requested close       |
|         - glClearColor & glClear                |
|         - primary->SwapBuffers()                |
|       Else (Headless / Windowless):             |
|         - Sleep 16ms to throttle CPU usage      |
+-------------------------------------------------+
                        |
                        v
+-------------------------------------------------+
|            Engine::terminate()                  |
|  - Destroys windows & cleans up GLFW            |
+-------------------------------------------------+
```

---

## Headless Mode & Running Without a Window

The engine is engineered to continue running even when no primary window is currently open:

- **Server & Background Applications**: If you do not instantiate a `Window`, the engine stays alive in `MainLoop()`, polling system events and executing logic loops.
- **CPU Throttling**: When `Window::GetPrimaryWindow() == nullptr`, `MainLoop()` sleeps for `16ms` (`std::this_thread::sleep_for(std::chrono::milliseconds(16))`), keeping CPU utilization near `0%` rather than spinning unbound.
- **Dynamic Window Creation**: Windows can be opened, closed, and reopened at any point during engine runtime via `new Window(...)` or deleted with `delete windowPtr`.
- **Safe Rendering Checks**: OpenGL commands (`glClearColor`, `glClear`, `SwapBuffers`) are strictly guarded and only executed when an active window and OpenGL context are present.
