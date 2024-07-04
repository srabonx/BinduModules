#pragma once
#include <chrono>

namespace BINDU
{

    constexpr double MS_TO_SEC = 0.0001;

    constexpr double MICROSEC_TO_SEC = 0.000001;

    constexpr double CONVERT_TIME = MICROSEC_TO_SEC;

    class Timer
    {
    public:
        Timer();
        ~Timer();

        // Must be reset before using/ before gameloop to update the base time
        void Reset();            

        // Restarts the clock and returns the time in microseconds since last Restart() call.
        uint64_t Restart();

        // to pause the timer.
        void Stop();               

        // to start the timer.
        void Start();               

        // Must be called every frame/loop
        void Tick();                

        // Total time since start of this application in seconds
        double TotalTime();           

        // Delta time between two frames in seconds
        double DeltaTime() const;          


    private:

        uint64_t m_baseTime{ 0 };
        uint64_t m_currTime{ 0 };
        uint64_t m_prevTime{ 0 };

        uint64_t m_stopTime{ 0 };
        uint64_t m_pausedTime{ 0 };

        uint64_t m_lastTimeToRestart{ 0 };

        double m_deltaTime{ 0.0 };

        bool    m_paused{ false };

    };
}
