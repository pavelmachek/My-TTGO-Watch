#include <stdio.h>

extern "C" {
#include "../../lua-on-arduino/src/lua/lua.h"
#include "../../lua-on-arduino/src/lua/lualib.h"
#include "../../lua-on-arduino/src/lua/lauxlib.h"
}

void lua_test(void) {
  printf("Should test lua\n");
}
    
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
