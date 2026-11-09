#pragma once

#include "core/global/EngineTime.h"
#include "core/private_imports.h"

class Timer
{
    friend class Engine;

public:
    explicit Timer(
        float duration,
        bool repeating = false,
        std::function<void()> onComplete = nullptr
    )
        : duration(duration),
          repeating(repeating),
          onComplete(std::move(onComplete))
    {
        activeTimers.push_back(this);
    }

    ~Timer()
    {
        activeTimers.erase(
            std::remove(activeTimers.begin(), activeTimers.end(), this),
            activeTimers.end()
        );
    }

    const float GetElapsedTime() const
    {
        return elapsedTime;
    }
    const float GetProgress() const
    {
        return std::clamp(elapsedTime / duration, 0.0f, 1.0f);
    }
    const float GetDuration() const
    {
        return duration;
    }

    bool Finished() const
    {
        return elapsedTime >= duration;
    }

private:
    inline static std::vector<Timer*> activeTimers;

    float elapsedTime = 0.0f;
    float duration = 0.0f;

    bool repeating = false;

    std::function<void()> onComplete;

    static void ClearAll()
    {
        auto copy = activeTimers;
        activeTimers.clear();
        for (Timer* t : copy)
        {
            delete t;
        }
    }

    static void UpdateAll()
    {
        for (size_t i = 0; i < activeTimers.size(); )
        {
            Timer* timer = activeTimers[i];

            timer->elapsedTime += Time::deltaTime;

            if (timer->Finished())
            {
                auto callback = timer->onComplete;
                if (timer->repeating)
                {
                    // I likey
                    timer->elapsedTime -= timer->duration;
                    ++i;
                }
                else
                {
                    delete timer;
                }

                if (callback)
                {
                    callback();
                }
            }
            else
            {
                ++i;
            }
        }
    }
};
