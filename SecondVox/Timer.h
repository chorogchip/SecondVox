#pragma once
#pragma once

#include "Macros.h"

#include <chrono>

namespace vox::utils
{
class Timer
{
private:
    std::chrono::microseconds additional_time = std::chrono::microseconds(0);
    bool is_stopped = false;
    std::chrono::system_clock::time_point started_time_;

public:

    M_FORCE_INLINE void Start()
    {
        is_stopped = false;
        this->started_time_ = std::chrono::system_clock::now();
    }

    M_FORCE_INLINE void Stop()
    {
        is_stopped = true;
        additional_time = ReStartAndGetElapsedMicroSec();
    }

    M_FORCE_INLINE std::chrono::microseconds GetElapsedMicroSec() const
    {
        if (is_stopped) return additional_time;
        auto current_time_ = std::chrono::system_clock::now();
        return additional_time + std::chrono::duration_cast<std::chrono::microseconds>(current_time_ - this->started_time_);
    }

    M_FORCE_INLINE std::chrono::microseconds ReStartAndGetElapsedMicroSec()
    {
        auto previous_time = this->started_time_;
        this->started_time_ = std::chrono::system_clock::now();
        if (is_stopped) {
            is_stopped = false;   
            return additional_time;
        }
        return additional_time + std::chrono::duration_cast<std::chrono::microseconds>(this->started_time_ - previous_time);
    }

    M_FORCE_INLINE void AddTimeMicroSec( std::chrono::microseconds microseconds )
    {
        this->started_time_ += microseconds;
    }
};

}