#include <stdint.h>
#include <vector>
#include "client.hpp"

class MixingMatrix
{
private:
    std::vector<Client> clients;
    std::vector<float> transfer_in;
    std::vector<float> transfer_out;
    std::vector<std::vector<float>> coefficients;
    const int sampleSize;
    int num_clients;
public:
    MixingMatrix(int num_clients, int sampleSize):sampleSize(sampleSize)
    {
        transfer_in.reserve(sampleSize);
        transfer_out.reserve(sampleSize);
    };
    ~MixingMatrix(){};
    void update();
    void inline mix(std::vector<float> in, std::vector<float> out, float c);
    void addClient(Client user);
    //void removeClient(Client user);
    void modifyCoefficient(int row, int col, float val);
};

// mixingMatrix::mixingMatrix(int sampleSize):sampleSize(sampleSize)
// {
//     // for (uint8_t i = 0; i < num_users; i++) //intialize coefficients matrix to be of identity matrix
//     // {
//     //     for (uint8_t j = 0; j < num_users; j++)
//     //     {
//     //         if (i == j)
//     //         {
//     //             coefficients[i][j] = 0.0;
//     //         }
//     //         else
//     //         {
//     //             coefficients[i][j] = 1.0;
//     //         }
//     //     }
        
//     // }
    

// }

// mixingMatrix::~mixingMatrix()
// {
// }