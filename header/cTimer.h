/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */
#ifndef CTIMER_H_
#define CTIMER_H_

#include <time.h>

// timer class based on https://github.com/NVIDIA/NvPipe/blob/master/examples/utils.h
class cTimer
{
public:
    cTimer()
    {
        reset();
    }

    void reset()
    {
        time  = std::chrono::high_resolution_clock::now();
    }

    double getElapsedSeconds() const
    {
        return 1.0e-6 * std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - time).count();
    }

    double getElapsedMilliseconds() const
    {
        return 1.0e-3 * std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - time).count();
    }

private:
    std::chrono::high_resolution_clock::time_point time;
};


#endif /* CTIMER_H_ */
