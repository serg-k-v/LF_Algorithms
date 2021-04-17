#include "LockFreeQueue.hpp"
#include <iostream>
#include <thread>

#define TRH_SIZE 6

int main(int argc, char const *argv[])
{
    std::thread arr[TRH_SIZE];
    auto q_ptr = std::make_shared<concurrent::Queue<int>>();
    size_t half_size = (int)TRH_SIZE/2;

    for (size_t i = 0; i < 4; i++)
    {
        arr[i] = std::thread([q_ptr]() {
            for (size_t i = 0; i < 5; i++) {
                q_ptr->enqueue(i);
                std::cout << "elem : " << i << " was added!" << std::endl;
            }
        });
    }

    q_ptr->print();
    
    for (size_t i = 0; i < 4; i++)
    {
        arr[i].join();
    }

    for (size_t i = half_size; i < 2; i++)
    {
        arr[i] = std::thread([q_ptr]() {
            int c = -1;
            for (size_t i = 0; i < 6; i++) {
                q_ptr->dequeue(c);
                std::cout << "get data : " << c << std::endl;
            }
        });
    }

    q_ptr->print();
    
    
    for (size_t i = 4; i < 6; i++)
    {
        arr[i].join();
    }

    q_ptr->print();
    q_ptr.reset();

    return 0;
}



