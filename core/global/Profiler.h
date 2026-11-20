#pragma once

#include <string>
#include <vector>
#include <array>
#include <chrono>
#include <algorithm>
#include <glm/glm.hpp>

enum class ProfileID : uint8_t {
    EventsInput = 0,
    TimersTweens,
    SceneECS,
    Render3D,
    BufferSwap,
    DebugOverlay,
    Count
};

struct ProfileSection {
    std::string name;
    glm::vec4 color = glm::vec4(1.0f);
    double durationMs = 0.0;
    double smoothedMs = 0.0;
    double percentage = 0.0;

    std::chrono::high_resolution_clock::time_point startTime{};
    bool isRunning = false;
};

class Profiler {
public:
    static constexpr size_t MAX_SECTIONS = 16;
    inline static bool s_enabled = false;

    static bool IsEnabled() {
        return s_enabled;
    }

    static void SetEnabled(bool enabled) {
        s_enabled = enabled;
        if (enabled) {
            EnsureInitialized();
        } else {
            Clear();
        }
    }

    static void Clear() {
        for (size_t i = 0; i < s_activeCount; ++i) {
            s_sections[i].durationMs = 0.0;
            s_sections[i].smoothedMs = 0.0;
            s_sections[i].percentage = 0.0;
            s_sections[i].isRunning = false;
        }
        s_lastFrameTimeMs = 0.0;
        s_totalSmoothedMs = 0.0;
    }

    static inline void BeginFrame() {
        if (!s_enabled) return;

        s_frameStartTime = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < s_activeCount; ++i) {
            s_sections[i].durationMs = 0.0;
            s_sections[i].isRunning = false;
        }
    }

    // High-performance O(1) enum-indexed section start
    static inline void BeginSection(ProfileID id) {
        if (!s_enabled) return;

        const size_t idx = static_cast<size_t>(id);
        s_sections[idx].startTime = std::chrono::high_resolution_clock::now();
        s_sections[idx].isRunning = true;
    }

    // High-performance O(1) enum-indexed section end
    static inline void EndSection(ProfileID id) {
        if (!s_enabled) return;

        auto now = std::chrono::high_resolution_clock::now();
        const size_t idx = static_cast<size_t>(id);
        if (s_sections[idx].isRunning) {
            s_sections[idx].durationMs += std::chrono::duration<double, std::milli>(now - s_sections[idx].startTime).count();
            s_sections[idx].isRunning = false;
        }
    }

    // Overload for dynamic/custom user sections
    static void BeginSection(const std::string& name, const glm::vec4& color = glm::vec4(1.0f)) {
        if (!s_enabled) return;

        auto now = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < s_activeCount; ++i) {
            if (s_sections[i].name == name) {
                s_sections[i].color = color;
                s_sections[i].startTime = now;
                s_sections[i].isRunning = true;
                return;
            }
        }

        if (s_activeCount < MAX_SECTIONS) {
            size_t idx = s_activeCount++;
            s_sections[idx].name = name;
            s_sections[idx].color = color;
            s_sections[idx].startTime = now;
            s_sections[idx].isRunning = true;
        }
    }

    static void EndSection(const std::string& name) {
        if (!s_enabled) return;

        auto now = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < s_activeCount; ++i) {
            if (s_sections[i].name == name && s_sections[i].isRunning) {
                s_sections[i].durationMs += std::chrono::duration<double, std::milli>(now - s_sections[i].startTime).count();
                s_sections[i].isRunning = false;
                return;
            }
        }
    }

    static inline void EndFrame() {
        if (!s_enabled) return;

        auto now = std::chrono::high_resolution_clock::now();
        s_lastFrameTimeMs = std::chrono::duration<double, std::milli>(now - s_frameStartTime).count();

        double totalSectionTime = 0.0;
        for (size_t i = 0; i < s_activeCount; ++i) {
            totalSectionTime += s_sections[i].durationMs;
        }

        double total = (totalSectionTime > 0.001) ? totalSectionTime : s_lastFrameTimeMs;
        if (total < 0.001) total = 0.001;

        if (s_totalSmoothedMs <= 0.0) {
            s_totalSmoothedMs = total;
        } else {
            s_totalSmoothedMs = s_totalSmoothedMs * 0.90 + total * 0.10;
        }

        const double invTotal = 100.0 / (s_totalSmoothedMs > 0.0001 ? s_totalSmoothedMs : 1.0);
        for (size_t i = 0; i < s_activeCount; ++i) {
            auto& sec = s_sections[i];
            if (sec.smoothedMs <= 0.0) {
                sec.smoothedMs = sec.durationMs;
            } else {
                sec.smoothedMs = sec.smoothedMs * 0.90 + sec.durationMs * 0.10;
            }
            sec.percentage = sec.smoothedMs * invTotal;
        }
    }

    static const ProfileSection* GetSectionData() {
        return s_sections.data();
    }

    static size_t GetActiveCount() {
        return s_activeCount;
    }

    static double GetTotalFrameTimeMs() {
        return s_lastFrameTimeMs;
    }

    static double GetSmoothedTotalMs() {
        return s_totalSmoothedMs;
    }

    struct Scope {
        ProfileID id;
        bool active = false;

        explicit Scope(ProfileID id)
            : id(id), active(Profiler::IsEnabled()) {
            if (active) Profiler::BeginSection(id);
        }

        ~Scope() {
            if (active) Profiler::EndSection(id);
        }
    };

private:
    static void EnsureInitialized() {
        if (s_initialized) return;

        s_sections[0] = { "Events & Input", glm::vec4(0.25f, 0.60f, 0.95f, 1.0f) };
        s_sections[1] = { "Timers & Tweens", glm::vec4(0.95f, 0.82f, 0.20f, 1.0f) };
        s_sections[2] = { "Scene Update / ECS", glm::vec4(0.95f, 0.48f, 0.15f, 1.0f) };
        s_sections[3] = { "3D Scene Render", glm::vec4(0.65f, 0.35f, 0.92f, 1.0f) };
        s_sections[4] = { "Buffer Swap / VSync", glm::vec4(0.20f, 0.85f, 0.65f, 1.0f) };
        s_sections[5] = { "Debug Profiler", glm::vec4(0.92f, 0.30f, 0.60f, 1.0f) };

        s_activeCount = static_cast<size_t>(ProfileID::Count);
        s_initialized = true;
    }

    inline static std::array<ProfileSection, MAX_SECTIONS> s_sections{};
    inline static size_t s_activeCount = static_cast<size_t>(ProfileID::Count);
    inline static bool s_initialized = false;
    inline static std::chrono::high_resolution_clock::time_point s_frameStartTime{};
    inline static double s_lastFrameTimeMs = 16.6;
    inline static double s_totalSmoothedMs = 16.6;
};
