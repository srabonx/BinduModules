#include "../Include/Bindu_Timer.h"

namespace BINDU
{

    Timer::Timer()
    {

    }

    Timer::~Timer()
    {

    }

    void Timer::Reset()
    {
        m_baseTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        m_currTime = m_baseTime;

    }

    uint64_t Timer::Restart()
    {
        const uint64_t currTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        const uint64_t timeDelta = currTime - m_lastTimeToRestart;

        m_lastTimeToRestart = currTime;

        return timeDelta;
    }

    void Timer::Stop()
    {
        if (!m_paused)
        {
            m_paused = true;
            m_stopTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        }
    }

    void Timer::Start()
    {
        if (m_paused)
        {
            m_paused = false;

            const uint64_t currTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

            m_pausedTime += currTime - m_stopTime;

            m_stopTime = 0;
        }
    }

    void Timer::Tick()
    {
        if (m_paused)
        {
            m_deltaTime = 0.0;
            return;
        }

        m_currTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        // Time difference between this frame and previous frame
        m_deltaTime = static_cast<double>(m_currTime - m_prevTime) * CONVERT_TIME;

        // current time is now previous time
        m_prevTime = m_currTime;

        // force non-negative

        if (m_deltaTime < 0.0)
            m_deltaTime = 0.0;

    }

    double Timer::TotalTime()
    {

        this->Tick();

        if (m_paused)
        {
            const uint64_t time = (m_stopTime - m_baseTime) - m_pausedTime;
            return static_cast<double>(time) * CONVERT_TIME;
        }
        else
            return static_cast<double>((m_currTime - m_baseTime) - m_pausedTime) * CONVERT_TIME;
    }

    double Timer::DeltaTime() const
    {
        return m_deltaTime;
    }
}
