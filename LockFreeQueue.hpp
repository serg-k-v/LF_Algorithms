#ifndef LOCKFREEQUEUE_HPP
#define LOCKFREEQUEUE_HPP

#include <atomic>
#include <iostream>
#include <list>

#define DEF_SIZE 256
#define MAX_SIZE 1024

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
    template <class T>
    struct MemNode
    {
        bool isUsed;
        Node<T>* node;
    };

    /**
     * Class stored almost allocated memory by OS
     * for reuse it on pop/dequeue operation.
     * Must use insted operator new for avoid errors allocation memory.
     */
    template <class T>
    class DoubledMemoryList
    {
        
        Node<T>* tail_handle;
        Node<T>* head_handle;
        std::list<MemNode<T>*> _d_mem;
        size_t _capacity;
    public:
        DoubledMemoryList();
        ~DoubledMemoryList();

        Node<T>* get_node();
        void free_node(const Node<T>*);
        void resize();
    };
    
    template <typename T>
    DoubledMemoryList<T>::DoubledMemoryList(){
        _capacity = DEF_SIZE;
        tail_handle = new Node<T>;
        head_handle = new Node<T>;

        for (size_t i = 0; i < _capacity; i++)
            _d_mem.insert({false, new Node<T>});
    }

    template <typename T>
    DoubledMemoryList<T>::~DoubledMemoryList(){}
    
    template <typename T>
    Node<T>* DoubledMemoryList<T>::get_node()
    {
        auto it = _d_mem.begin();
        while (it != _d_mem.end())
            if (!it->isUsed)
                return it->Node;
        resize();
    }

    template <typename T>
    void DoubledMemoryList<T>::resize()
    {
        _capacity += DEF_SIZE;
        if (_capacity > MAX_SIZE) {
            std::cerr << "Not enought memory\n";
            return;
        }

        for (size_t i = 0; i < DEF_SIZE; i++)
            _d_mem.insert({false, new Node<T>});
    }

    template <typename T>
    void DoubledMemoryList<T>::free_node(const Node<T>* node)
    {
        auto it = _d_mem.cbegin();
        while (it != _d_mem.cend())
        {
            if (*it == node)
                it->isused = false;
            it++;
        }
    }

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
        void print();

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
        while (true)
        {
            auto head = _head.load(std::memory_order_relaxed);
            auto head_next = head->next.load(std::memory_order_relaxed);

            if (head_next)
                return;

            if (!head->next.compare_exchange_weak(head_next, head_next->next,
                                    std::memory_order_release,
                                    std::memory_order_relaxed))
            {
                auto tmp = new Node<T>;
                head->next.store(tmp);
                item = tmp->data;

                delete tmp;
                return;
            }
        }
    }

    template <typename T>
    void Queue<T>::enqueue(const T& item)
    {
        auto new_tail = new Node<T>(item);
        auto curr_tail = _tail.load(std::memory_order_relaxed);

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

    template <typename T>
    void Queue<T>::print()
    {
        auto head = _head.load(std::memory_order_relaxed);
        Node<T>* head_next = nullptr;

        while (head)
        {
            head_next = head->next.load(std::memory_order_relaxed);
            std::cout << head->data << " ";
            head = head_next;
        }
        std::cout << std::endl;
    }
}

#endif // LOCKFREEQUEUE_HPP