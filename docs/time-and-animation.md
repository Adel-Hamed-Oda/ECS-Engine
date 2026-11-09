# Time, Timers & Tweens

The engine provides a timing and animation suite comprising global time tracking ([`Time`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/core/global/EngineTime.h)), automated timers ([`Timer`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/anim/Timer.h)), and interpolated property tweens ([`Tween`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/anim/Tween.h)).

---

## 1. Engine Time (`core/global/EngineTime.h`)

The static `Time` class provides frame delta and runtime measurement across all systems and scenes.

### Public Properties

All time values are read-only public static references, with `timeScale` being publicly configurable:

```cpp
class Time {
public:
    static const int& frames;          // Total elapsed frame count
    static const double& time;         // Total accumulated engine time (scaled)
    static const double& deltaTime;    // Elapsed time for the current frame (scaled)
    static const double& FPS;          // Smoothed FPS (moving average over 600 frames)
    static const double& immediateFPS; // Instantaneous FPS (1.0 / deltaTime)

    static double timeScale;           // Time dilation multiplier (default: 1.0f)
};
```

### Time Scaling
Adjusting `Time::timeScale` automatically scales both `Time::deltaTime` and `Time::time`:
```cpp
Time::timeScale = 0.5f; // Slow motion (half speed)
Time::timeScale = 2.0f; // Fast forward (double speed)
Time::timeScale = 0.0f; // Pause gameplay logic, timers, and tweens
```

---

## 2. Timers (`anim/Timer.h`)

[`Timer`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/anim/Timer.h) executes callbacks after a specified duration. It supports one-shot and repeating execution.

### Constructor & Destructor

```cpp
explicit Timer(
    float duration,
    bool repeating = false,
    std::function<void()> onComplete = nullptr
);

~Timer();
```

- **Duration**: Duration in seconds.
- **Repeating**:
  - `false` (One-shot): Automatically deletes itself and triggers `onComplete` when finished.
  - `true` (Repeating): Triggers `onComplete` every time `duration` expires, resetting elapsed time, and continues running until explicitly deleted.

### Methods

| Method | Return Type | Description |
| :--- | :--- | :--- |
| `GetElapsedTime()` | `float` | Returns total elapsed seconds for the current cycle. |
| `GetProgress()` | `float` | Returns normalized progress clamped between `0.0f` and `1.0f`. |
| `GetDuration()` | `float` | Returns the timer duration in seconds. |
| `Finished()` | `bool` | Returns `true` if `elapsedTime >= duration`. |

### Lifecycle & Memory Management
- **Automatic Registration**: Instantiating a `Timer` automatically registers it with the engine's internal update list.
- **One-Shot Timers**: When a non-repeating timer completes, the engine automatically executes `delete this;`. You can simply fire and forget:
  ```cpp
  // Fires once after 3 seconds, then deletes itself automatically
  new Timer(3.0f, false, []() {
      std::cout << "3 seconds elapsed!" << std::endl;
  });
  ```
- **Repeating Timers**: Must be stored and explicitly deleted when no longer needed (e.g. during scene unload):
  ```cpp
  Timer* repeatingTimer = new Timer(1.0f, true, []() {
      std::cout << "Tick every second" << std::endl;
  });

  // Later, when unloading the scene:
  delete repeatingTimer;
  ```
- **Engine Cleanup**: When the engine shuts down, `Engine::terminate()` invokes `Timer::ClearAll()`, safely deallocating all active timers.

---

## 3. Tweens (`anim/Tween.h`)

[`Tween`](file:///c:/Users/adelh/OneDrive/Desktop/ECS%20Test%20Game/engine/anim/Tween.h) smoothly interpolates a target variable from a start value to an end value over time using linear interpolation (`Lerp`) or custom easing functions.

### Constructor

```cpp
template <typename T>
Tween(
    T* value,
    const T& start,
    const T& end,
    float duration,
    std::function<void()> onComplete = nullptr,
    std::function<float(float)> easingFunction = nullptr
);
```

- **Supported Types (`T`)**:
  - Arithmetic scalars: `float`, `double`, `int`, `short`.
  - GLM vector types: `glm::vec2`, `glm::vec3`, `glm::vec4`, `glm::ivec2`, etc.
- **`value`**: Pointer to the variable to mutate each frame.
- **`start`**: Initial value at $t = 0$.
- **`end`**: Target value at $t = 1$.
- **`duration`**: Duration in seconds.
- **`onComplete`**: Optional callback executed when the tween reaches its target.
- **`easingFunction`**: Optional function mapping normalized time $t \in [0.0, 1.0] \rightarrow t'$.

### Automatic Lifecycle & Deletion
- A `Tween` registers itself into the engine's active tween list upon construction.
- Each frame, `Engine::MainLoop()` updates elapsed time using `Time::deltaTime` and applies the interpolated value.
- When `Finished()`, the tween invokes `onComplete` (if provided) and automatically executes `delete this;`.
- On engine termination, `Tween::ClearAll()` cleans up any remaining active tweens.

---

## Usage Examples

### 1. Interpolating Entity Position (GLM Vector)
```cpp
#include <Engine.h>

void AnimateEntity(Scene& scene, Entity entity) {
    auto& transform = scene.GetComponent<Transform>(entity);

    // Smoothly move position from (0, 0) to (500, 300) over 2.5 seconds
    new Tween<glm::vec2>(
        &transform.position,
        glm::vec2(0.0f, 0.0f),
        glm::vec2(500.0f, 300.0f),
        2.5f,
        []() {
            std::cout << "Movement tween completed!" << std::endl;
        }
    );
}
```

### 2. Custom Easing Function (Ease-In Quadratic)
```cpp
// Quadratic ease-in: f(t) = t^2
auto easeInQuad = [](float t) -> float {
    return t * t;
};

float opacity = 0.0f;
new Tween<float>(
    &opacity,
    0.0f,
    1.0f,
    1.0f,
    [&opacity]() {
        std::cout << "Fade in complete: " << opacity << std::endl;
    },
    easeInQuad
);
```

### 3. Combining Timers and Events in a Scene
```cpp
class LevelScene : public Scene {
public:
    Timer* spawnTimer = nullptr;

    LevelScene() : Scene("LevelScene") {
        onLoad += [this]() {
            // Spawn an entity every 2 seconds
            spawnTimer = new Timer(2.0f, true, [this]() {
                Entity enemy = Instantiate();
                auto& tf = AddComponent<Transform>(enemy, glm::vec2(100.0f, 100.0f));

                // Tween enemy position downward over 1 second
                new Tween<glm::vec2>(&tf.position, tf.position, tf.position + glm::vec2(0.0f, 200.0f), 1.0f);
            });
        };

        onUnload += [this]() {
            if (spawnTimer) {
                delete spawnTimer;
                spawnTimer = nullptr;
            }
        };
    }

    ~LevelScene() override {
        if (spawnTimer) {
            delete spawnTimer;
            spawnTimer = nullptr;
        }
    }
};
```
