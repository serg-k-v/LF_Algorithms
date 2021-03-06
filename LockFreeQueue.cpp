#include "LockFreeQueue.hpp"
#include <iostream>
#include <thread>

#define TRH_SIZE 6

int main(int argc, char const *argv[])
{
    std::thread arr[TRH_SIZE];
    auto q_ptr = std::make_shared<concurrent::Queue<int>>();
    size_t half_size = (int)TRH_SIZE/2;

    for (size_t i = 0; i < 2; i++)
    {
        arr[i] = std::thread([q_ptr]() {
            for (size_t i = 0; i < 10; i++) {
                q_ptr->enqueue(i);
                std::cout << "elem : " << i << " was added!" << std::endl;
            }
        });
    }

    q_ptr.reset();
    // for (size_t i = half_size; i < TRH_SIZE; i++)
    // {
    //     arr[i] = std::thread([&q_ptr]() {
    //         int c = -1;
    //         for (size_t i = 0; i < 100; i++) {
    //             q_ptr->dequeue(c);
    //             std::cout << "data : " << c << std::endl;
    //         }
    //     });
    // }
    
    for (size_t i = 0; i < 2; i++)
    {
        arr[i].join();
    }
    
    return 0;
}



