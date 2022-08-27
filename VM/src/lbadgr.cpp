#include "lualib.h"

#include "lstate.h"
#include "lapi.h"
#include "ldo.h"
#include "ludata.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;




static int luaB_badgr_log(lua_State* L)
{
    luaL_checkany(L, 1);
    size_t vl;
    const char* s = lua_tolstring(L, -1, &vl);
    jmethodID m = L->global->env->GetStaticMethodID(*L->global->bridge_class, "nativeLog", "(Ljava/lang/String;)V");
    L->global->env->CallStaticVoidMethod(*L->global->bridge_class, m, L->global->env->NewStringUTF(s));
    return 1;
}

static int luaB_badgr_read(lua_State* L)
{
    std::ifstream f("example.json");
    json data = json::parse(f);

    lua_newtable(L);
    for (auto& el : data.items()) {
        if (el.value().is_string()){
            lua_pushstring(L, const_cast<char*>(((std::string)el.value()).c_str()));
            lua_setfield(L, -2, const_cast<char*>(el.key().c_str()));
        } else if (el.value().is_number()) {
            lua_pushinteger(L, ((int64_t)el.value()));
            lua_setfield(L, -2, const_cast<char*>(el.key().c_str()));
        }
        
    }
    return 1;
}


static const luaL_Reg badgr_func[] = {
    {"log", luaB_badgr_log},
    {"read", luaB_badgr_read},
    {NULL, NULL},
};

/*
** Open badgr library
*/
int luaopen_badgr(lua_State* L)
{
    luaL_register(L, LUA_BADGRLIBNAME, badgr_func);
    return 1;
}

