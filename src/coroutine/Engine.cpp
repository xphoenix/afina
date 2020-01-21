#include <afina/coroutine/Engine.h>

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

namespace Afina {
namespace Coroutine {

void Engine::Store(context &ctx) {
    char address;
    ctx.Low = &address < StackBottom ? &address : StackBottom;
    ctx.Hight = &address < StackBottom ? StackBottom : &address;
    auto &buf = std::get<0>(ctx.Stack);
    auto &size = std::get<1>(ctx.Stack);
    auto cur_size = ctx.Hight - ctx.Low;
    if (size < cur_size) {
        delete[] buf;
        size = cur_size;
        buf = new char[size];
    }
    memcpy(buf, ctx.Low, cur_size);
    // ctx.Stack = std::make_tuple(buf, cur_size);
}

void Engine::Restore(context &ctx) {
    char address;
    if ((ctx.Low <= &address) && (ctx.Hight > &address)) {
        Restore(ctx);
    }
    auto &buf = std::get<0>(ctx.Stack);
    auto &size = std::get<1>(ctx.Stack);
    memcpy(ctx.Low, buf, size);
    cur_routine = &ctx;
    longjmp(ctx.Environment, 1);
}

void Engine::yield() {
    if (alive == nullptr) {
        return;
    }
    auto routine = alive;
    if (routine == cur_routine) {
        if (routine->next != nullptr) {
            routine = routine->next;
        } else {
            return;
        }
    }

    Enter(*routine);
}

void Engine::sched(void *routine_) {
    if (routine_ == nullptr) {
        yield();
    } else if (routine_ == cur_routine) {
        return;
    }

    Enter(*(static_cast<context *>(routine_)));
}

void Engine::Enter(context &ctx) {
    if (cur_routine != nullptr && cur_routine != idle_ctx) {
        if (setjmp(cur_routine->Environment) > 0) {
            return;
        }
        Store(*cur_routine);
    }
    cur_routine = &ctx;
    Restore(ctx);
}

} // namespace Coroutine
} // namespace Afina
