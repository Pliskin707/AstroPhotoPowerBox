#ifndef __MAVG_HPP__
#define __MAVG_HPP__

#include "non_copy_class.hpp"
#include "rb.h"

class movingAverage : private nonCopyable
{
    private:
        pliskin_ringbuffer::rb_base * _buffer = nullptr;
        float _Sum = 0.0f;
        float _avgValue = 0.0f;

    public:
        movingAverage (const size_t size);
        ~movingAverage();

        void addValue (const float value);
        float getAvgValue (void) const;
        float getOldestValue (void) const;
        void reset (const float startValue = 0.0f);
};

#endif