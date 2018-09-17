#pragma once

#include "cTimer.h"


/**
 * Author: Tim Biedert
 * biedert@cs.uni-kl.de
 * @brief Returns some measured value per second.
 *
 */
class RateStats
{
public:
    RateStats()
    {
        this->reset();
    }

    void reset()
    {
        this->x = 0.0f;
        this->timer.reset();
    }

    void add(float v)
    {
        this->x += v;
    }

    float getRate(float interval)
    {
        // Update rate
        float elapsed = this->timer.getElapsedMilliseconds();
        if (elapsed >= interval)
        {
            this->rate = this->x / (0.001f * elapsed);
            this->reset();
        }

        return this->rate;
    }

private:
    float x = 0.0f;
    float rate = 0.0f;
    cTimer timer;
};



class AverageStats
{
public:
    AverageStats()
    {
        this->reset();
    }

    void reset()
    {
        this->x = 0.0f;
        this->n = 0;
        this->timer.reset();
    }

    void add(float v)
    {
        this->x += v;
        ++this->n;
    }

    float getAverage(float interval)
    {
        // Update average
        float elapsed = this->timer.getElapsedMilliseconds();
        if (elapsed >= interval)
        {
            this->average = this->n > 0 ? (this->x / this->n) : 0.0f;
            this->reset();
        }
        return this->average;
    }

private:
    float x = 0.0f;
    uint32_t n = 0;
    float average = 0.0f;
    cTimer timer;
};


