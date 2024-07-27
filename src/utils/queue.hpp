#ifndef RISCV_SIMULATOR_QUEUE_HPP
#define RISCV_SIMULATOR_QUEUE_HPP

template<class T, int N>
class Queue{
    friend class ReorderBuffer;
private:
    T data[N];
    int head = 0;
    int tail = 0;
    int length = 0;
public:
    Queue() = default;
    bool empty() {return tail == head;}
    bool full() {return (tail + 1) % N == head;}
    void push(const T &obj) {
        data[tail] = obj;
        tail = (tail + 1) % N;
        length++;
    }
    void pop() {
        head = (head + 1) % N;
        length--;
    }
    T& operator[](int index) {
        index = (index - 1) % N;
        return data[index];
    }
    int size() {return length;}
    void clear() {
        head = tail = 0;
        length = 0;
    }
    T front() {return data[head];}
    T back() {return data[tail - 1];} // may over index
    void print() {
        int cur = head;
        while (cur != tail) {
            std::cout<<data[cur];
            cur = (cur + 1) % N;
        }
    }
};

#endif //RISCV_SIMULATOR_QUEUE_HPP
