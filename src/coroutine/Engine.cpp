#include <afina/coroutine/Engine.h>

#include <csetjmp>
#include <cstring>

namespace Afina {
namespace Coroutine {

void Engine::Store(context &ctx) {
    char storeBeginAddress;
    ctx.Low = ctx.Hight = StackBottom;

    // this condition used for different architectures,
    // where stack low addresses can be greater than high addresses
    if (&storeBeginAddress > StackBottom) {
        ctx.Hight = &storeBeginAddress;
    } else {
        ctx.Low = &storeBeginAddress;
    }

    std::cout << "Store func here, ctx.Low = " << std::hex <<  (long) ctx.Low
              << " ctx.Higth = " << std::hex << (long) ctx.Hight << std::endl;

    auto stackSize = ctx.Hight - ctx.Low;
    char *&buffer = std::get<0>(ctx.Stack);
    uint32_t &prevSize = std::get<1>(ctx.Stack);

    // we should allocate memory for new stack copy if it was't allocated yet
    // or current stack size isn't big enough or current stack size is too big
    if (stackSize > prevSize || stackSize * 2 < prevSize) {
        delete[] buffer;
        buffer = new char[stackSize];
        prevSize = stackSize;
    }
    std::cout << "buf address = " << std::hex << (long) buffer << std::endl;
    memcpy(buffer, ctx.Low, stackSize);
}

void Engine::Restore(context &ctx) {
    char restoreBeginAddress;

    while (&restoreBeginAddress <= ctx.Hight && &restoreBeginAddress >= ctx.Low) {
        Restore(ctx);
    }
    // now we can restore coroutine's stack without changing our stack
    std::memcpy(ctx.Low, std::get<0>(ctx.Stack), ctx.Hight - ctx.Low);
    cur_routine = &ctx;
    // run coroutine from the point where it was stopped
    longjmp(ctx.Environment, 1);
}

void Engine::yield() {
    // we have no alive coroutines or we have only one alive coroutine
    if (alive == nullptr || alive->next == nullptr) return;

    // run the next alive routine
    context *nextCoro;
    if (cur_routine == alive) {
        nextCoro = alive->next;
    } else {
        nextCoro = alive;
    }

    if (cur_routine != nullptr && cur_routine != idle_ctx) {
        if (setjmp(cur_routine->Environment) > 0) {
            return;
        }

        Store(*cur_routine);
    }
    Restore(*nextCoro);
}

void Engine::sched(void *coro) {
    auto nextCoro = static_cast<context *>(coro);
    if (nextCoro == cur_routine) return;
    if (nextCoro == nullptr) yield();

    // if coro is blocked???

    if (cur_routine != nullptr && cur_routine != idle_ctx) {
        if (setjmp(cur_routine->Environment) > 0) {
            return;
        }
        Store(*cur_routine);
    }
    Restore(*nextCoro);
}

void Engine::block(void *coro) {
    context *blockedCoro;
    if (coro == nullptr) {
        blockedCoro = cur_routine;
    } else {
        blockedCoro = static_cast<context *>(coro);
    }

    if (blocked == blockedCoro) return;

    // delete coroutine from the list of alive coroutines
    if (blockedCoro->prev != nullptr) {
        blockedCoro->prev->next = blockedCoro->next;
    }
    if (blockedCoro->next != nullptr) {
        blockedCoro->next->prev = blockedCoro->prev;
    }

    // add coroutine to the list of blocked coroutines
    blockedCoro->prev = nullptr;
    blockedCoro->next = blocked;
    blocked = blockedCoro;
    if (blocked->next != nullptr) {
        blocked->next->prev = blockedCoro;
    }
    if (blockedCoro == cur_routine) {
//        yield();
        if (cur_routine != nullptr && cur_routine != idle_ctx) {
            if (setjmp(cur_routine->Environment) > 0) {
                return;
            }
            Store(*cur_routine);
        }
        cur_routine = nullptr;
        Restore(*idle_ctx);
    }
}

void Engine::unblock(void *coro) {
    auto unblockedCoro = static_cast<context *>(coro);
    if (unblockedCoro == cur_routine || unblockedCoro == alive) return;

    // delete coroutine from the list of blocked coroutines
    if (unblockedCoro->prev != nullptr) {
        unblockedCoro->prev->next = unblockedCoro->next;
    }
    if (unblockedCoro->next != nullptr) {
        unblockedCoro->next->prev = unblockedCoro->prev;
    }

    // add coroutine to the list of alive coroutines
    unblockedCoro->prev = nullptr;
    unblockedCoro->next = alive;
    alive = unblockedCoro;
    if (alive->next != nullptr) {
        alive->next->prev = unblockedCoro;
    }
}

} // namespace Coroutine
} // namespace Afina
