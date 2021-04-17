#ifndef LOCKFREEQUEUE_HPP
#define LOCKFREEQUEUE_HPP

#include <atomic>
#include <list>

template <typename T>
struct Node
{
    T data;
    std::atomic<Node<T>*> next;

    Node<T>() {}
    Node<T>(const T& data) : data(data) {
        next.store(nullptr);
    }
};

/**
 * CAS - CompareAndSwap
 * compare_exchange - _weak and _strong has difference if run on x64
 * use weak in loops
 */

namespace concurrent
{
    /**
     * Class stored almost allocated memory by OS
     * for reuse it on pop/dequeue operation.
     */
    template <class T>
    class DoubledMemoryList
    {
        Node<T>* tail_handle;
        Node<T>* head_handle;
        std::list<Node<T>*> _d_mem;
    public:
        DoubledMemoryList();
        ~DoubledMemoryList();
    };
    
    template <typename T>
    DoubledMemoryList<T>::DoubledMemoryList(){}
    
    template <typename T>
    DoubledMemoryList<T>::~DoubledMemoryList(){}
    

    /**
     * Non-blocking queue class
     */    
    template <class T>
    class Queue
    {
        std::atomic<Node<T>*> _head;
        std::atomic<Node<T>*> _tail;

    public:
        Queue();
        ~Queue();

        void dequeue(T&);
        void enqueue(const T&);

        /**
         * Appends an element to the end of the queue.
         * Returns false when the queue is full.
         */
        bool try_push(const T&);
        /**
         * Removes an element from the front of the queue.
         * Returns false when the queue is empty.
         */
        bool try_pop(T&);
    };

    template <typename T>
    Queue<T>::Queue(){
        auto dummy = new Node<T>();
        _head.store(dummy, std::memory_order_relaxed);
        _tail.store(dummy, std::memory_order_release);
    }

    template <typename T>
    Queue<T>::~Queue(){}
/*
    template <typename T>
    bool Queue<T>::try_push(const T& data)
    {
        auto new_tail = Node_t(data);
        auto next = _tail->next.load(std::memory_order_relaxed);
        auto curr_tail = _tail.load(std::memory_order_relaxed);

        while (true)
        {
            if (_tail->next.compare_exechange_weak(next, new_tail_next,
                                        std::memory_order_release,
                                        std::memory_order_relaxed))
            {
                _tail.compare_exchange_strong(curr_tail, new_tail,
                                        std::memory_order_release,
                                        std::memory_order_relaxed);
                return true;
            } else {
                _tail.compare_exchange_weak(curr_tail, new_tail,
                                        std::memory_order_release,
                                        std::memory_order_relaxed);
            }
        }
    }

    template <typename T>
    bool Queue<T>::try_pop(T& item)
    {
        // std::atomic::compare_exachnge
        auto curr_top = _head.load(std::memory_order_relaxed);
        while (true)
        {
            if (!curr_top)
                return false;

            item = curr_top.load(std::memory_order_relaxed);

            if (!_head.compare_exchange_weak(curr_top, curr_top->next,
                                        std::memory_order_release,
                                        std::memory_order_relaxed))
            {
                return true;
            }
        }
        
    }
*/
    template <typename T>
    void Queue<T>::dequeue(T& item)
    {
        // while (true)
        // {
        //     auto head_next = _head.load(std::memory_order_relaxed)->next.load(std::memory_order_relaxed);
        //     if (head_next->next)
        //         return;

        //     if (!_head.next.compare_exchange_weak(head_next, head_next->next,
        //                             std::memory_order_release,
        //                             std::memory_order_relaxed))
        //     {
        //         head_next.store(item);
        //         return;
        //     }
        // }
    }

    template <typename T>
    void Queue<T>::enqueue(const T& item)
    {
        auto new_tail = new Node<T>(item);
        auto curr_tail = _tail.load(std::memory_order_relaxed);
        // auto tail_next = curr_tail->next.load(std::memory_order_relaxed);

        while (true)
        {
            if (!curr_tail->next) {
                Node<T>* null_node = nullptr;
                if (curr_tail->next.compare_exchange_weak(null_node, new_tail,
                                        std::memory_order_release,
                                        std::memory_order_relaxed)
                    )
                    break;
            } else {
                _tail.compare_exchange_weak(curr_tail, curr_tail->next);
            }
        }
        
        _tail.compare_exchange_strong(curr_tail, new_tail);
    }
}

#endif // LOCKFREEQUEUE_HPP