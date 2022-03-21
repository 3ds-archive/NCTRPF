#include <3ds.h>
#include "plgldr.h"
#include "csvc.h"
#include "common.h"
#include "syscalls.h"

#include "ctr-led-brary.hpp"
#include "NCTRPF/Screen.hpp"
#include "NCTRPF/Key.hpp"

static Handle thread;
static u8     stack[STACK_SIZE] ALIGN(8);

void onExit()
{
  Screen::Exit();
  hidExit();
  fsExit();
  srvExit();
}

extern char* fake_heap_start;
extern char* fake_heap_end;
extern u32 __ctru_heap;
extern u32 __ctru_linear_heap;

u32 __ctru_heap_size        = 0;
u32 __ctru_linear_heap_size = 0;

void __system_initSyscallsEx()
{
  ThreadVars* tv = getThreadVars();
  tv->magic = THREADVARS_MAGIC;
  tv->thread_ptr = NULL;
  tv->reent = _impure_ptr;
  tv->tls_tp = fake_heap_start;
  tv->srv_blocking_policy = false;
}

void __system_allocateHeaps()
{
  PluginHeader *header = (PluginHeader*)(0x7000000);

  __ctru_heap = header->heapVA;
  __ctru_heap_size = header->heapSize;

  mappableInit(0x11000000, OS_MAP_AREA_END);

  // Set up newlib heap
  fake_heap_start = (char *)__ctru_heap;
  fake_heap_end = fake_heap_start + __ctru_heap_size;
}

// Plugin main thread entrypoint
void ThreadMain(void* arg)
{
  // Initialize systems
  __sync_init();
  __system_initSyscallsEx();
  __system_allocateHeaps();
  
  // Initialize services
  srvInit();
  hidInit();
  fsInit();
  plgLdrInit();

  // Initialize classes
  Screen::Initialize();

  // Plugin main loop
  while (1)
  {
    svcSleepThread(1000000);

    // check event
    s32 event = PLGLDR__FetchEvent();

    if (event == PLG_ABOUT_TO_EXIT)
      onExit();

    PLGLDR__Reply(event);

    Key::Update();

    if (Key::IsDown(KEY_SELECT))
    {
      Screen &bottom = Screen::GetBottom();
      bottom.DrawString("Hello!", 0, 0);

      // https://github.com/PabloMK7/ctr-led-brary/blob/master/example/source/main.cpp#L57
      LED::PlayLEDPattern(LED::GeneratePattern(LED_Color(0xFF, 0xFF, 0xFF), LED_PatType::WAVE_DESC, 4, 0, 0x10, 0, 2.0/3, 1.0/3));
    }
    else
    {
      LED::StopLEDPattern();
    }
  }
}

int main()
{
  // Create the plugin's main thread
  svcCreateThread(&thread, ThreadMain, 0, (u32*)(stack + STACK_SIZE), 30, -1);
}
