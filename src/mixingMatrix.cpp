#include "mixingMatrix.hpp"


void MixingMatrix::update(){
    // workingBuffer.clear()

    for(int col = 0; col<numStreams; col++){
        streams[col].src->getMany(workingBuffer, sampleSize);
        for (int row = 0; row < numStreams; row++)
        {
            if(col == 0){
                memset(streams[row].mixingBuffer, 0, sampleSize*sizeof(float));
            }
            mix(workingBuffer, streams[row].mixingBuffer, coefficients[row][col]);
        }
    }

    for (int row = 0; row < numStreams; row++){
        streams[row].dst->putMany(streams[row].mixingBuffer, sampleSize);
    }
}

void inline MixingMatrix::mix(const float *in, float *out, float c)
{
    for (u_int16_t k = 0; k < sampleSize; k++)
    {
        out[k] += in[k] * c;
    }
}

void MixingMatrix::addClient(Client user)
{   
    addAudioPair(user.getID(), user.getReceiveBuffer(), user.getSendBuffer());
}

void MixingMatrix::addAudioPair(std::string id, std::shared_ptr<AudioBuffer> src, std::shared_ptr<AudioBuffer> dst){
    AudioPair audioPair;
    audioPair.id = id;
    audioPair.src = src;
    audioPair.dst = dst;
    audioPair.index = numStreams;
    audioPair.mixingBuffer = new float[sampleSize];
    streams.push_back(audioPair);

    std::vector<float> newCoef;
    for (uint8_t j = 0; j < numStreams; j++)
    {
        newCoef.push_back(1.0f);
    }

    coefficients.push_back(newCoef);
    numStreams += 1;
    for (uint8_t i = 0; i < numStreams; i++)
    {
        coefficients[i].push_back(1.0);
    }
    coefficients[numStreams - 1][numStreams - 1] = 0;
}


void MixingMatrix::modifyCoefficient(int row, int col, float val)
{
    coefficients[row][col] = val;
}