// =================================================================================
// Filename:     timerclass.h
// Description:  this class is a high precision timer that measures
//               the exact time between frames of execution. It's 
//               primary use if for synchronizing objects that 
//               require a standard time frame for movement.
// Revising:     01.08.22
// =================================================================================
#pragma once
#include <chrono>

namespace Core
{
//---------------------------------------------------------
// some typedefs to get timings
//---------------------------------------------------------
using TimeDurationMs = std::chrono::duration<float, std::milli>;
using TimePoint      = std::chrono::steady_clock::time_point;


//---------------------------------------------------------
// class:  GameTimer
//---------------------------------------------------------
class GameTimer
{
public:
    GameTimer();

    // get time in seconds
    float GetGameTime()  const;
    float GetDeltaTime() const;

    TimePoint GetTimePoint() const;

    void Reset();  // is called before message loop
    void Start();  // is called when unpaused
    void Stop();   // is called when paused
    void Tick();   // is called every frame

private:
    bool isStopped_ = false;

    __int64 currTime_ = 0;
    __int64 pausedTime_ = 0;
    __int64 baseTime_ = 0;
    __int64 prevTime_ = 0;
    __int64 stopTime_ = 0;

    double secondsPerCount_ = 0.0f;  // 1.0 / counts_per_sec
    double deltaTime_       = -1.0f;
};

//---------------------------------------------------------
// Desc:  return a time passed since the last frame 
//---------------------------------------------------------
inline float GameTimer::GetDeltaTime() const
{
    return (float)deltaTime_;
}

//---------------------------------------------------------
// Desc:  get current point of time (aka timestamp)
//---------------------------------------------------------
inline TimePoint GameTimer::GetTimePoint() const
{
    return std::chrono::steady_clock::now();
}

}
