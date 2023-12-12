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

    int getMany(float *data, int length){
        int retrieved = RingBuffer::getMany(data, length);
        if(length - retrieved > 0){
            memset(&data[retrieved], 0, length - retrieved);
        }
        return retrieved;
    }
};

// AudioBuffer::AudioBuffer(size_t size)
// {
// }

// AudioBuffer::~AudioBuffer()
// {
// }


#endif //AUDIO_BUFFER_HPP