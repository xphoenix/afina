#include "gtest/gtest.h"

#include <iostream>

#include <afina/coroutine/Engine.h>

void _calculator_add(int &result, int left, int right) { result = left + right; }

TEST(CoroutineTest, SimpleStart) {
    Afina::Coroutine::Engine engine;

    int result;
    engine.start(_calculator_add, result, 1, 2);

    ASSERT_EQ(3, result);
}

void printa(Afina::Coroutine::Engine &pe, std::stringstream &out, void *&other) {
    out << "A1 ";
    pe.sched(other);

    out << "A2 ";
    pe.sched(other);

    out << "A3 ";
    pe.sched(other);
}

void printb(Afina::Coroutine::Engine &pe, std::stringstream &out, void *&other) {
    out << "B1 ";
    pe.sched(other);

    out << "B2 ";
    pe.sched(other);

    out << "B3 ";
}

std::stringstream out;
void *pa = nullptr, *pb = nullptr;
void _printer(Afina::Coroutine::Engine &pe, std::string &result) {
    // Create routines, note it doens't get control yet
    pa = pe.run(printa, pe, out, pb);
    pb = pe.run(printb, pe, out, pa);

    // Pass control to first routine, it will ping pong
    // between printa/printb greedely then we will get
    // control back
    pe.sched(pa);

    out << "END";

    // done
    result = out.str();
}

TEST(CoroutineTest, Printer) {
    Afina::Coroutine::Engine engine;

    std::string result;
    engine.start(_printer, engine, result);
    ASSERT_STREQ("A1 B1 A2 B2 A3 B3 END", result.c_str());
}

std::stringstream out1;
void *pc = nullptr, *pd = nullptr;

void printc(Afina::Coroutine::Engine &pe, std::stringstream &out, void *&other) {
    out << "C1 ";
    pe.block(nullptr);
    out << "C2 ";
    pe.unblock(other);
}

void printd(Afina::Coroutine::Engine &pe, std::stringstream &out, void *&other) {
    out << "D1 ";
    pe.unblock(other);
    pe.block(nullptr);
    out << "D2 ";
}

void block_printer(Afina::Coroutine::Engine &pe, std::string &result) {
    pc = pe.run(printc, pe, out1, pd);
    pd = pe.run(printd, pe, out1, pc);

    // Pass control to first routine, it will ping pong
    // between printa/printb greedely then we will get
    // control back
    pe.sched(pc);

    out1 << "END";

    // done
    result = out1.str();

}

TEST(CoroutineTest, Block) {
    Afina::Coroutine::Engine engine;

    std::string result;
    engine.start(block_printer, engine, result);

    ASSERT_STREQ("C1 D1 C2 D2 END", result.c_str());
}