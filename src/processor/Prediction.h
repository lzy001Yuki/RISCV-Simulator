#ifndef RISCV_SIMULATOR_PREDICTION_H
#define RISCV_SIMULATOR_PREDICTION_H
#include"../utils/Type.hpp"
class Predictor{
private:
    u8 history[1<<6] = {0};
    u8 status[1<<6][16] = {0}; // 1-4 level
    int right = 0;
    int total = 0;
    u32 hash(u32 pc) {
        return pc * 13 % (1<<6);
    }
public:
    bool predict(u32 &pc);
    void update(u32 &pc, bool cur);
    double Accuracy() const ;
};

bool Predictor::predict(u32 &pc) {
    u32 index = hash(pc);
    total++;
    if (status[index][history[index]] >= 3) return true;
    else return false;
}

void Predictor::update(u32 &pc, bool correct) {
    u32 index = hash(pc);
    int now = status[index][history[index]];
    int upd = now;
    if (correct) right++;
    if (now == 1) {
        if (correct) upd++;
    } else if (now == 2 || now == 3) {
        if (correct) upd++;
        else upd--;
    } else if (now == 4) {
        if (!correct) upd--;
    }
    status[index][history[index]] = upd;
    history[index] = ((history[index] << 1) | correct) & 15;
}
double Predictor::Accuracy() const {return (double) right / total;}
#endif //RISCV_SIMULATOR_PREDICTION_H
