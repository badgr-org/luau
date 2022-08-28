#include "lualib.h"

#include "lstate.h"
#include "lapi.h"
#include "ldo.h"
#include "ludata.h"
#include "ltable.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

jobject StlStringStringMapToJavaHashMap(JNIEnv* env, const std::map<std::string, std::string>& map)
{
    jclass mapClass = env->FindClass("java/util/HashMap");
    if (mapClass == NULL)
        return NULL;

    jmethodID init = env->GetMethodID(mapClass, "<init>", "()V");
    jobject hashMap = env->NewObject(mapClass, init);
    jmethodID put = env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    std::map<std::string, std::string>::const_iterator citr = map.begin();
    for (; citr != map.end(); ++citr)
    {
        jstring keyJava = env->NewStringUTF(citr->first.c_str());
        jstring valueJava = env->NewStringUTF(citr->second.c_str());

        env->CallObjectMethod(hashMap, put, keyJava, valueJava);

        env->DeleteLocalRef(keyJava);
        env->DeleteLocalRef(valueJava);
    }

    jobject hashMapGobal = static_cast<jobject>(env->NewGlobalRef(hashMap));
    env->DeleteLocalRef(hashMap);
    env->DeleteLocalRef(mapClass);

    return hashMapGobal;
}





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
    for (auto& el : data.items())
    {
        if (el.value().is_string())
        {
            lua_pushstring(L, const_cast<char*>(((std::string)el.value()).c_str()));
            lua_setfield(L, -2, const_cast<char*>(el.key().c_str()));
        }
        else if (el.value().is_number())
        {
            lua_pushinteger(L, ((int64_t)el.value()));
            lua_setfield(L, -2, const_cast<char*>(el.key().c_str()));
        }
    }
    return 1;
}


static int luaB_badgr_updateView(lua_State* L)
{
    luaL_checkany(L, 2);
    size_t vl;
    const char* id = lua_tolstring(L, -2, &vl);
    Table* t = hvalue(L->top - 1);
    int i = sizenode(t);

    std::map<std::string, std::string> params;
    while (i--)
    {
        LuaNode* n = &t->node[i];
        if (!ttisnil(gval(n)))
        {
            if (ttisstring(gkey(n)) && ttisstring(gval(n))){
                std::string key = getstr(tsvalue(gkey(n)));
                std::string value = getstr(tsvalue(gval(n)));
                params.insert(std::pair<std::string, std::string>(key, value));
            }
        }
    }



 

    jobject jmap = StlStringStringMapToJavaHashMap(L->global->env, params);
    jmethodID m = L->global->env->GetStaticMethodID(*L->global->bridge_class, "updateViewObjectParams", "(Ljava/lang/String;Ljava/util/Map;)V");
    L->global->env->CallStaticVoidMethod(*L->global->bridge_class, m, L->global->env->NewStringUTF(id), jmap);
    return 1;
}


static const luaL_Reg badgr_func[] = {
    {"log", luaB_badgr_log},
    {"read", luaB_badgr_read},
    {"updateView", luaB_badgr_updateView},
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

