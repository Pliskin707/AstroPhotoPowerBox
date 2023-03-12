#include "mavg.hpp"

movingAverage::movingAverage (const size_t size)
{
    _buffer = new pliskin_ringbuffer::rb_base(sizeof(_avgValue), size);
}

movingAverage::~movingAverage ()
{
    if (_buffer != nullptr)
    {
        delete _buffer;
        _buffer = nullptr;
    }
}

void movingAverage::addValue (const float value)
{
    if (_buffer != nullptr)
    {
        if (_buffer->full())
        {
            float oldVal;
            _buffer->read(&oldVal);

            _Sum -= oldVal;
        }

        _buffer->write(&value);
        _Sum += value;
        _avgValue = _Sum / ((float) _buffer->remaining_elm());
    }
}

float movingAverage::getAvgValue (void) const
{
    return _avgValue;
}

float movingAverage::getOldestValue(void) const
{
    float oldestVal = 0.0f;

    if (_buffer != nullptr)
        _buffer->peak(&oldestVal);

    return oldestVal;
}

void movingAverage::reset (const float startValue)
{
    if (_buffer != nullptr)
    {
        _Sum = 0.0f;
        _avgValue = startValue;

        _buffer->clear();
        while(_buffer->write(&startValue))
            _Sum += startValue;
    }
}