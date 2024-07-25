cmake_minimum_required(VERSION 3.22)
project(RISCV_Simulator)

set(CMAKE_CXX_STANDARD 17)

add_executable(RISCV_Simulator src/main.cpp
        src/processor/LoadStoreBuffer.h
        src/processor/Prediction.h
        src/processor/Reorder.h
        src/processor/ReservationStation.h
        src/processor/ALU.h
        src/utils/queue.hpp
        src/processor/cpu.hpp
        src/utils/Type.hpp
        src/processor/decode.h
        src/processor/Memory.hpp
        src/processor/CommonDataBus.hpp
        src/processor/opQueue.h
        src/processor/Register.hpp
)
