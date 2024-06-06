#include "../Include/Timer.h"

GameTimer::GameTimer()
{

}

GameTimer::~GameTimer()
{

}

void GameTimer::Reset()
{
    m_baseTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    m_currTime = m_baseTime;

}

void GameTimer::Stop()
{
    if (!m_paused)
    {
        m_paused = true;
        m_stopTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
}

void GameTimer::Start()
{
    if (m_paused)
    {
        m_paused = false;

        int64_t currTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        m_pausedTime += currTime - m_stopTime;

        m_stopTime = 0;
    }
}

void GameTimer::Tick()
{
    if (m_paused)
    {
        m_deltaTime = 0.0;
        return;
    }

    m_currTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

    // Time difference between this frame and previous frame
    m_deltaTime = (m_currTime - m_prevTime) * 0.000001;

    // current time is now previous time
    m_prevTime = m_currTime;

    // force non-negative

    if (m_deltaTime < 0.0)
        m_deltaTime = 0.0;

}

double GameTimer::TotalTime()
{

    this->Tick();

    if (m_paused)
    {
        int64_t time = (m_stopTime - m_baseTime) - m_pausedTime;
        return time * 0.000001;
    }
    else
        return ((m_currTime - m_baseTime) - m_pausedTime) * 0.000001;
}

double GameTimer::DeltaTime() const
{
    return m_deltaTime;
}
