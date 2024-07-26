#ifndef RISCV_SIMULATOR_QUEUE_HPP
#define RISCV_SIMULATOR_QUEUE_HPP

template<class T, int N>
class Queue{
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
    void clear() {head = tail;}
    T front() {return data[head];}
    T back() {return data[tail - 1];} // may over index
};

#endif //RISCV_SIMULATOR_QUEUE_HPP
