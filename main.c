/*
 * Copyright (c) 2021 Muvik Labs, LLC
 * Licensed under the MIT license.
 */

#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int lua_main(int argc, char **argv, void (*loader)(lua_State*));
int sg_lua_video(lua_State *L);

static void loader(lua_State *L)
{
    luaL_requiref(L, "sgvideo", sg_lua_video, 1);
}

int main(int argc, char *argv[])
{
    return lua_main(argc, argv, loader);
}
