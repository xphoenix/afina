#include <afina/coroutine/Engine.h>

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

namespace Afina {
namespace Coroutine {
void Engine::Store(context &ctx) {
  volatile char stackStart;
  //Должен сохранить регистры и стэк
  ctx.Low = ctx.Height = this->StackBottom;
  if (&stackStart>ctx.Low){
    ctx.Height = const_cast<char*>(&stackStart);
  }
  else{
    ctx.Low = const_cast<char*>(&stackStart);
  }
  int size = ctx.Height - ctx.Low;

  // If dst and src overlap, behavior is undefined. To avoid this, we make:
  if (std::get<1>(ctx.Stack)<size){
    delete std::get<0>(ctx.Stack);
    std::get<0>(ctx.Stack) = new char[size];
    std::get<1>(ctx.Stack) = size;
  }

  memcpy(std::get<0>(ctx.Stack), ctx.Low, size);
}

void Engine::Restore(context &ctx) {
  volatile char stackStart;
  char *StackAddr = const_cast<char*>(&stackStart);
  if(ctx.Low<=StackAddr && StackAddr<=ctx.Height) Restore(ctx);

  // memcpy(void *restrict dst, const void *restrict src, size_t n);
  // The memcpy() function copies n bytes from memory area src to memory area dst.
  memcpy(ctx.Low, std::get<0>(ctx.Stack), std::get<1>(ctx.Stack));
  longjmp(ctx.Environment, 1);
}

void Engine::yield() {
  if (cur_routine == nullptr and alive == nullptr){
    return;
  }

context* to_call = nullptr;
 for (context* p = alive; p != nullptr; p = p->next) {
      if (p != cur_routine) { // find any routine != cur_routine
          to_call = p;
          break;
      }
  }
  
  sched(to_call);
}


void Engine::sched(void *routine_) {
  context *to_call = (context *)routine_;

    if (to_call == cur_routine) {
      return;
    }


  // these lines are required to exit last run() correctly
  if (to_call == nullptr) {
      if (alive == nullptr) {
          return;   // no coroutines remain
      }
      else {
          to_call = alive;
      }
  }

  if (cur_routine){
      Store(*cur_routine); //so there would be a place to return to
      if (setjmp(cur_routine -> Environment)){
        return;
      }
  }
  // throw current frame away
  cur_routine = to_call;
  Restore(*to_call);

}

} // namespace Coroutine
} // namespace Afina
