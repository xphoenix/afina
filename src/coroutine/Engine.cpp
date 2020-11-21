#include <afina/coroutine/Engine.h>

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

namespace Afina {
namespace Coroutine {
ATTRIBUTE_NO_SANITIZE_ADDRESS
void Engine::Store(context &ctx) {
    volatile char current_addr;
    ctx.Hight =ctx.Low=StackBottom;
    if(&current_addr<ctx.Low)
    {
        ctx.Low=(char*)&current_addr;
    }else{
        ctx.Low=(char*)&current_addr;
    }
    auto size=ctx.Hight-ctx.Low;
    auto &buf=std::get<char*>(ctx.Stack);
    auto &new_size=std::get<uint32_t>(ctx.Stack);
    if(std::get<uint32_t>(ctx.Stack)<size)
    {
        delete[] std::get<char*>(ctx.Stack);
        new_size = size;
        buf=new char[size];
    }
    memcpy(buf,ctx.Low,size);
}
ATTRIBUTE_NO_SANITIZE_ADDRESS
void Engine::Restore(context &ctx) {
    volatile char current_addr;
    if(ctx.Low<=&current_addr && ctx.Hight>&current_addr)
    {
        Restore(ctx);
    }
    memcpy(ctx.Low, std::get<char*>(ctx.Stack), std::get<uint32_t>(ctx.Stack));
    longjmp(ctx.Environment, 1);
}
ATTRIBUTE_NO_SANITIZE_ADDRESS
void Engine::yield() {
    if(cur_routine!=nullptr || alive !=nullptr)
    {
        context *ptr;
        for(ptr=alive; ptr!=cur_routine;ptr=ptr->next){
            if(ptr)
                break;
        }
        sched(ptr);
    }
}
ATTRIBUTE_NO_SANITIZE_ADDRESS
void Engine::sched(void *routine_) {
    if (routine_ == nullptr) {
        yield();
    }
    if (cur_routine == routine_) {
        return;
    }
    context *r = (context*)routine_;
    if(r!=cur_routine){
        if(!r){
            if(!alive){
                return;
            }else{
                r=alive;
            }
        }
        if(cur_routine)
        {
            Store(*cur_routine);
            if(setjmp(cur_routine->Environment)){
                return;
            }
        }
    }
    cur_routine=r;
    Restore(*r);
}

} // namespace Coroutine
} // namespace Afina