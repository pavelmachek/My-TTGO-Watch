#include <stdio.h>

extern "C" {
#include "lua/src/lua.h"
#include "lua/src/lualib.h"
#include "lua/src/lauxlib.h"

  static int c_getversion(lua_State *L)
  {
    double arg1 = luaL_checknumber (L, 1);

    //push the results
    lua_pushnumber(L, arg1 + 42);

    //return number of results
    return 1;
  }
}

static void lua_add_callbacks(lua_State *L)
{
  lua_pushcfunction(L, c_getversion);
  lua_setglobal(L, "c_getversion");
}

void lua_test(void) {
  printf("Should test lua\n");
  
  lua_State *L = luaL_newstate();
  luaopen_base(L);
  //  luaopen_io(L);
  luaopen_string(L);
  luaopen_math(L);

  lua_add_callbacks(L);

  printf("Lua ready?\n");
#if 0    
  if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0))
    printf("cannot run configuration file: %s",
	  lua_tostring(L, -1));
#else
  if (luaL_loadstring(L, "width = c_getversion(0); height = c_getversion(-42); text = 'hello C'") || lua_pcall(L, 0, 0, 0))
    printf("cannot run configuration file: %s",
	  lua_tostring(L, -1));
#endif

  lua_getglobal(L, "text");
  lua_getglobal(L, "width");
  lua_getglobal(L, "height");
  if (!lua_isstring(L, -3))
    printf("`width' should be a number\n");
  if (!lua_isnumber(L, -2))
    printf("`width' should be a number\n");
  if (!lua_isnumber(L, -1))
    printf("`height' should be a number\n");

  printf("Got text = %s\n", (char *)lua_tostring(L, -3));
  printf("Got width = %d\n", (int)lua_tonumber(L, -2));
  printf("Got height = %d\n", (int)lua_tonumber(L, -1));
    
  lua_close(L);

  printf("Done testing lua\n");
  fflush(stdout);
}
