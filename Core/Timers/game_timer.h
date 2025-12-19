// =================================================================================
// Filename:     timerclass.h
// Description:  this class is a high precision timer that measures
//               the exact time between frames of execution. It's 
//               primary use if for synchronizing objects that 
//               require a standard time frame for movement.
// Revising:     01.08.22
// =================================================================================
#pragma once

namespace Core
{

class GameTimer
{
public:
    GameTimer();

    // get time in seconds
    float        GetGameTime()  const;

    inline float GetDeltaTime() const
    {
        //constexpr float maxDeltaTime = (1.0f / 60.0f);
        //return (deltaTime_ > maxDeltaTime) ? maxDeltaTime : (float)deltaTime_;

        return (float)deltaTime_;
    }

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

}
