#ifndef RISCV_SIMULATOR_STACK_HPP
#define RISCV_SIMULATOR_STACK_HPP
template<int N>
class stack{
private:
    int data[N] = 0;
    int top = 0;
public:
    stack() = default;
    void push(int i) {
        top++;
        data[top] = i;
    }
    bool full() {return top == N-1;}
    int pop() {
        int tmp = data[top];
        top--;
        return tmp;
    }

};
#endif //RISCV_SIMULATOR_STACK_HPP
