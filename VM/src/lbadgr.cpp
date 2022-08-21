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




static int luaB_badgr_write(lua_State* L)
{
    luaL_checkany(L, 1);
    size_t vl;
    const char* s = lua_tolstring(L, -1, &vl);
    printf("%s", s);
    return 1;
}

static int luaB_badgr_read(lua_State* L)
{

    std::ifstream f("example.json");
    json data = json::parse(f);

    lua_newtable(L);

    Table* t = hvalue(L->top - 1);

    StkId v = L->base + 1;
    // lua_setfield(lua_State* L, 1, data.data);
    // char* strr = const_cast<char*>(data.dump().c_str());
    for (auto& el : data.items()) {
        // std::cout << el.key() << " : " << el.value() << "\n";
        if (el.value().is_string()){
            lua_pushstring(L, const_cast<char*>(((std::string)el.value()).c_str()));
            lua_setfield(L, -2, const_cast<char*>(el.key().c_str()));
        } else if (el.value().is_number()) {
            lua_pushinteger(L, ((int64_t)el.value()));
            lua_setfield(L, -2, const_cast<char*>(el.key().c_str()));
        }
        
        // else if (el.value().is_boolean()) {
        //     lua_pushboolean(L, ((int)el.value()));
        //     lua_setfield(L, -2, const_cast<char*>(el.key().c_str()));
        // }
    }
    // lua_pushstring(L, strr);
    return 1;
}

static const luaL_Reg badgr_func[] = {
    {"write", luaB_badgr_write},
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
