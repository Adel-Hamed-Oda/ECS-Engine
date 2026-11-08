#pragma once

#include <vector>

class Engine;

class Time
{
    friend class Engine;

private:
    inline static int _frames = 0;
    inline static double _time = 0.0f;
    inline static double _deltaTime = 0.0f;
    inline static double _FPS = 0.0f;
    inline static double _immediateFPS = 0.0f;

public:
    // Read-only access for users to query time information
    inline static const int& frames = _frames;
    inline static const double& time = _time;
    inline static const double& deltaTime = _deltaTime;
    inline static const double& FPS = _FPS;
    inline static const double& immediateFPS = _immediateFPS;

    inline static double timeScale = 1.0f;

private:
    // Only Engine is allowed to call Update
    static void Update(double* lastFrameTime, double currentTime)
    {
        const double currentFrameTime = static_cast<double>(currentTime);
        const double unscaledDelta = (currentFrameTime > *lastFrameTime)
            ? (currentFrameTime - *lastFrameTime)
            : 0.0f;
        _frames++;
        _deltaTime = unscaledDelta * timeScale;
        *lastFrameTime = currentFrameTime;
        _time += _deltaTime;
        if (_deltaTime > 0.0f)
            _immediateFPS = 1.0f / _deltaTime;
        else
            _immediateFPS = 0.0f;

        static std::vector<double> deltaBuffer;
        static double deltaSum = 0.0f;
        if (deltaBuffer.size() > 600)
        {
            deltaSum -= deltaBuffer.front();
            deltaBuffer.erase(deltaBuffer.begin());
        }
        deltaBuffer.push_back(unscaledDelta);
        deltaSum += unscaledDelta;

        if (_frames % 60 == 0 && deltaSum > 0.0f)
            _FPS = (static_cast<double>(deltaBuffer.size()) / deltaSum);
    }
};
