#include <iostream>
#include <thread>
#include <unistd.h>
#include <vector>

#include "count_down_latch.h"

void fun(countdownlatch *cl) {
    std::cout << std::this_thread::get_id() << ' ' << "Wait... " << std::endl;
    cl->await();
    std::cout << std::this_thread::get_id() << ' ' << "Wait is over " << std::endl;
}

int main() {
    auto cl = new countdownlatch(10);
    int i = 0;
    std::vector<std::thread *> ts;
    while (i++ < 2) {
        std::thread *t = new std::thread(fun, cl);
        ts.push_back(t);
    }

    i = 0;
    while (i++ < 10) {
        sleep(1);
        std::cout << std::this_thread::get_id() << ' ' << "[" << i << "] Decrement latch " << std::endl;
        cl->count_down();
    }

    i = 0;
    while (i < 2) {
        ts[i++]->join();
    }

    i = 0;
    while (i < 2) {
        delete ts[i++];
    }
    delete cl;
    return 0;
}
