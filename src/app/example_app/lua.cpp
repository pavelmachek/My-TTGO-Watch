#include <stdio.h>

void lua_test(void) {}

#if 0
extern "C" {
#if 0
#include "lua-on-arduino/src/lua/lua.h"
#include "lua-on-arduino/src/lua/lualib.h"
#include "lua-on-arduino/src/lua/lauxlib.h"
#else
#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>
#endif  
}

void lua_test(void) {
  printf("Should test lua\n");
  
  lua_State *L = luaL_newstate();
  luaopen_base(L);
  //  luaopen_io(L);
  luaopen_string(L);
  luaopen_math(L);

  printf("Lua ready?\n");
#if 0    
  if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0))
    printf("cannot run configuration file: %s",
	  lua_tostring(L, -1));
#else
  if (luaL_loadstring(L, "width = 42; height = 123") || lua_pcall(L, 0, 0, 0))
    printf("cannot run configuration file: %s",
	  lua_tostring(L, -1));
#endif
    
  lua_getglobal(L, "width");
  lua_getglobal(L, "height");
  if (!lua_isnumber(L, -2))
    printf("`width' should be a number\n");
  if (!lua_isnumber(L, -1))
    printf("`height' should be a number\n");

  printf("Got width = %d\n", (int)lua_tonumber(L, -2));
  printf("Got height = %d\n", (int)lua_tonumber(L, -1));
    
  lua_close(L);

  printf("Done testing lua\n");
  fflush(stdout);
}

#if 0
void load (char *filename, int *width, int *height) {
  lua_State *L = lua_open();
  luaopen_base(L);
  //  luaopen_io(L);
  luaopen_string(L);
  luaopen_math(L);

  printf("Lua ready?\n");
    
  if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0))
    printf("cannot run configuration file: %s",
	  lua_tostring(L, -1));
    
  lua_getglobal(L, "width");
  lua_getglobal(L, "height");
  if (!lua_isnumber(L, -2))
    printf("`width' should be a number\n");
  if (!lua_isnumber(L, -1))
    printf("`height' should be a number\n");
  *width = (int)lua_tonumber(L, -2);
  *height = (int)lua_tonumber(L, -1);
    
  lua_close(L);
}
#endif
#endif

#if 0
//#include "mbed.h"
extern "C" {
#include "runtime.h"

#include "micropython/py/runtime.h"
#include "micropython/py/mphal.h"
#include "micropython/lib/utils/pyexec.h"
}


// Serial communication to host PC via USB
Serial pc(USBTX, USBRX);

// Implement the micropython HAL I/O functions
extern "C" void mp_hal_stdout_tx_chr(int c);
void mp_hal_stdout_tx_chr(int c) {
    pc.putc(c);
}
extern "C" int mp_hal_stdin_rx_chr(void);
int mp_hal_stdin_rx_chr(void) {
    int c = pc.getc();
    return c;
}

// Now the main program - run the REPL.
int main() {
    mp_init();
    pyexec_friendly_repl();
    mp_deinit();
    return 0;
}
#endif
