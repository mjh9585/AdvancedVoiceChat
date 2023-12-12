#include "mixingMatrix.hpp"


void MixingMatrix::update(){
    // transfer_in.clear();
    // transfer_out.clear();
    // for (uint16_t i = 0; i < num_clients; i++)
    // {
    //     clients[i].getReceiveBuffer()->getMany(transfer_in.data(),transfer_in.capacity());

    //     for (uint16_t j = 0; j < num_clients; j++)
    //     {
    //         mix(transfer_in,transfer_out, coefficients[i][j]);
    //     }
    // }
    // clients[j].getSendBuffer()->putMany(transfer_out.data(),transfer_out.capacity());
}

void inline MixingMatrix::mix(std::vector<float> in, std::vector<float> out, float c)
{
    for (u_int16_t k = 0; k < sampleSize; k++)
    {
        out[k] += in[k] * c;
    }
}

void MixingMatrix::addClient(Client user)
{
    clients.push_back(user);
    coefficients.emplace_back(new std::vector<float>);
    for (uint8_t j = 0; j < num_clients; j++)
    {
        coefficients[num_clients].push_back(1.0f);
    }
    num_clients += 1;
    for (uint8_t i = 0; i < num_clients; i++)
    {
        coefficients[i].push_back(1.0);
    }
    coefficients[num_clients - 1][num_clients - 1] = 0;
}

void MixingMatrix::modifyCoefficient(int row, int col, float val)
{
    coefficients[row][col] = val;
}