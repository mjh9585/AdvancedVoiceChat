#include <stdint.h>
#include <vector>
#include "client.hpp"

class MixingMatrix
{
    struct AudioPair
    {
        int index;
        std::string id;
        std::shared_ptr<AudioBuffer> src;
        std::shared_ptr<AudioBuffer> dst;
        float *mixingBuffer;
    };

private:
    std::vector<AudioPair> streams;
    float *workingBuffer;
    // std::vector<float> transfer_out;
    std::vector<std::vector<float>> coefficients;
    const int sampleSize;
    int numStreams = 0;

public:
    MixingMatrix(int sampleSize):sampleSize(sampleSize)
    {
        workingBuffer = new float[sampleSize];
        // workingBuffer.reserve(sampleSize);
        // transfer_out.reserve(sampleSize);
    };
    ~MixingMatrix(){
        delete workingBuffer;
        for(auto v : streams){
            delete v.mixingBuffer;
        }
        streams.erase(streams.begin(), streams.end());
    };
    void update();
    void inline mix(const float *in, float *out, float c);
    void addClient(Client user);
    void addAudioPair(std::string id, std::shared_ptr<AudioBuffer> src, std::shared_ptr<AudioBuffer> dst);
    //void removeClient(Client user);
    void modifyCoefficient(int row, int col, float val);

    void printCoefficients(){
        std::cout << "[" << std::endl;
        for(int row = 0; row < numStreams; row++){
            std::cout << "  [";
            for(int col = 0; col < numStreams; col++){
                printf("%f, ", coefficients[row][col]);
            }
            std::cout << "]," << std::endl;
        }
        std::cout << "]" << std::endl;
    };
};