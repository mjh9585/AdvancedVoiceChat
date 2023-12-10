#include "RingBuffer.hpp"
#include <stdio.h>
#include <iostream>
//using namespace std;

int main()
{
    int size = 2;
    RingBuffer<uint8_t> channel1(size);
    RingBuffer<uint8_t> channel2(size);
    // channel1.reset();
    // channel2.reset();

    std::cout << std::boolalpha;

    std::cout << "Initial State:" << std::endl;
    std::cout << "\tChannel 1 empty " << channel1.empty() << "; Channel 2 empty " << channel2.empty() << "\r\n";
    std::cout << "\tChannel 1 capacity " << channel1.capacity() << "; Channel 2 capacity " << channel2.capacity() << "\r\n";
    std::cout << "\tChannel 1 size " << channel1.size() << "; Channel 2 size " << channel2.size() << "\r\n";
    std::cout << "\tChannel 1 full " << channel1.full() << "; Channel 2 full " << channel2.full() << "\r\n";

    std::cout << std::endl << "Adding Elements" << std::endl;
    for (uint8_t i = 0; i < 3; i++)
    {
        channel1.put(i);
        channel2.put(i);
        printf("After iteration %d: \r\n", i);
        std::cout << "\tChannel 1 empty " << channel1.empty() << "; Channel 2 empty " << channel2.empty() << "\r\n";
        std::cout << "\tChannel 1 capacity " << channel1.capacity() << "; Channel 2 capacity " << channel2.capacity() << "\r\n";
        std::cout << "\tChannel 1 size " << channel1.size() << "; Channel 2 size " << channel2.size() << "\r\n";
        std::cout << "\tChannel 1 full " << channel1.full() << "; Channel 2 full " << channel2.full() << "\r\n";
    }

    std::cout << "After Adding: \r\n";
    std::cout << "\tChannel 1 empty " << channel1.empty() << "; Channel 2 empty " << channel2.empty() << "\r\n";
    std::cout << "\tChannel 1 capacity " << channel1.capacity() << "; Channel 2 capacity " << channel2.capacity() << "\r\n";
    std::cout << "\tChannel 1 size " << channel1.size() << "; Channel 2 size " << channel2.size() << "\r\n";
    std::cout << "\tChannel 1 full " << channel1.full() << "; Channel 2 full " << channel2.full() << "\r\n";

    // std::cout << "Channel 1 full " << channel1.full() << "; Channel 2 full " << channel2.full() << "\r\n";
    for (uint8_t i = 0; i < 2; i++)
    {
        auto d1 = channel1.get();
        auto d2 = channel2.get();
        if (d1)
        {
            printf("Channel 1 [%d] = %d\r\n",i,d1.value());
            // std::cout << "Channel 1 [" << i << "] = " << d1.value() << "\r\n";
        }
        
        if (d2)
        {
            printf("Channel 2 [%d] = %d\r\n",i,d2.value());
            // std::cout << "Channel 2 [" << i << "] = " << d2.value() << "\r\n";
        }

        printf("After iteration %d: \r\n", i);
        std::cout << "\tChannel 1 empty " << channel1.empty() << "; Channel 2 empty " << channel2.empty() << "\r\n";
        std::cout << "\tChannel 1 capacity " << channel1.capacity() << "; Channel 2 capacity " << channel2.capacity() << "\r\n";
        std::cout << "\tChannel 1 size " << channel1.size() << "; Channel 2 size " << channel2.size() << "\r\n";
        std::cout << "\tChannel 1 full " << channel1.full() << "; Channel 2 full " << channel2.full() << "\r\n";
    }
    std::cout << "Channel 1 empty " << channel1.empty() << "; Channel 2 empty " << channel2.empty() << "\r\n";
    std::cout << "Channel 1 size " << channel1.size() << "; Channel 2 size " << channel2.size() << "\r\n";
    std::cout << "Channel 1 full " << channel1.full() << "; Channel 2 full " << channel2.full() << "\r\n";
    channel1.reset();
    channel2.reset();
    std::cout << "Channel 1 empty " << channel1.empty() << "; Channel 2 empty " << channel2.empty() << "\r\n";
    std::cout << "Channel 1 size " << channel1.size() << "; Channel 2 size " << channel2.size() << "\r\n";
    std::cout << "Channel 1 full " << channel1.full() << "; Channel 2 full " << channel2.full() << "\r\n";

    RingBuffer<uint8_t> channel3(5);
    uint8_t data_in[3];
    uint8_t data_out[5];
    for (int i = 0; i < 3; i++)
    {
        data_in[0] = i * 3;
        data_in[1] = i * 3 + 1;
        data_in[2] = i * 3 + 2;
        channel3.putMany(data_in, 3);
        channel3.printBuff();
    }

    printf("Got %d\r\n", channel3.getMany(data_out, 5));

    for (int i = 0; i < 5; i++)
    {
        printf("Data_out [%d] = %d\r\n", i, data_out[i]);
    }
    
    
}