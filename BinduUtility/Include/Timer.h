#pragma once
#include <chrono>

class GameTimer
{
public:
    GameTimer();
    ~GameTimer();

    void Reset();             // Must be reset before using/ before gameloop to update the base time

    void Stop();               // to pause the timer.
    void Start();               // to start the timer.

    void Tick();                // Must be called every frame/loop

    double TotalTime();           // Total time since start of this application in seconds
    double DeltaTime() const;          // Delta time between two frames in seconds



private:

    int64_t m_baseTime{ 0 };
    int64_t m_currTime{ 0 };
    int64_t m_prevTime{ 0 };

    int64_t m_stopTime{ 0 };
    int64_t m_pausedTime{ 0 };

    double m_deltaTime{ 0.0 };

    bool    m_paused{ false };

};
