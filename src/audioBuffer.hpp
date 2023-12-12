#ifndef AUDIO_BUFFER_HPP
#define AUDIO_BUFFER_HPP

#include "RingBuffer.hpp"

class AudioBuffer : public RingBuffer<float>
{
private:
    /* data */
public:
    AudioBuffer(size_t size) : RingBuffer(size){};
    ~AudioBuffer(){};
};

// AudioBuffer::AudioBuffer(size_t size)
// {
// }

// AudioBuffer::~AudioBuffer()
// {
// }


#endif //AUDIO_BUFFER_HPP