#include "lua.h"
#include "lualib.h"
#include "luacode.h"
#include "lgc.h"
#include "lstate.h"
#include <jni.h>
#include <cstdlib>
#include <memory>
#include <string>
#include <lfunc.h>
#include <thread>

lua_State* L;

void attachThreadToVM(JNIEnv* env) {
    JavaVM* g_vm;
    env->GetJavaVM(&g_vm);
    JNIEnv * g_env;

    int getEnvStat = g_vm->GetEnv((void **)&g_env, JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED) {
        if (g_vm->AttachCurrentThread( &g_env, NULL) != 0) {
            abort();
        }
    }
}

int runCode(lua_State* L, char* source)
{
    std::string str2((char *)source);
    size_t bytecodeSize = 0;
    char* vv = luau_compile(str2.data(), str2.length(), nullptr, &bytecodeSize);

    int result = luau_load(L, "=stdin", vv, bytecodeSize, 0);

    if (result != 0)
    {
        size_t len;

    
        lua_pop(L, 1);

        return 1;
    }

    lua_State* T = lua_newthread(L);

    lua_pushvalue(L, -2);
    lua_remove(L, -3);
    lua_xmove(L, T, 1);

    int status = lua_resume(T, NULL, 0);

    if (status == 0)
    {
        int n = lua_gettop(T);

        if (n)
        {
            luaL_checkstack(T, LUA_MINSTACK, "too many results to print");
            lua_getglobal(T, "print");
            lua_insert(T, 1);
            lua_pcall(T, n, 0, 0);
        }

        lua_pop(L, 1); // pop T
        return 0;
    }
    else
    {
        lua_pop(L, 1); // pop T
        return 1;
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_badgr_native_LuaBridge_startVMandAttachBridge(
    JNIEnv* env, jobject thiz, jobject bridge, jbyteArray bytecode)
{
    
    jclass thisClass = env->GetObjectClass(bridge);
    env->NewGlobalRef(bridge);

    jbyte* carr;
    carr = env->GetByteArrayElements( bytecode, NULL);
    if (carr == NULL)
    {
        abort();
    }


    std::unique_ptr<lua_State, void (*)(lua_State*)> globalState(luaL_newstate(), lua_close);
    lua_State* GL = globalState.get();
    luaL_openlibs(GL);
    luaL_sandbox(GL);
    size_t bytecodeSize = env->GetArrayLength(bytecode );
    L = lua_newthread(GL);
    JavaVM  *jvm;

    env->GetJavaVM(&jvm);
    L->global->jvm =jvm;
    L->global->bridge_class = &thisClass;
    L->global->bridge = &bridge;

    // new thread needs to have the globals sandboxed
    luaL_sandboxthread(L);
    runCode( L, (char*)carr);
    // lua_resume(L, NULL, 0);
    env->ReleaseByteArrayElements(bytecode, carr, 0);
}



extern "C" JNIEXPORT void JNICALL Java_com_badgr_native_LuaBridge_triggerOnClickFunctionForView(JNIEnv *env, jobject thiz, jstring view_id) {


    const char *viewId = env->GetStringUTFChars(view_id, 0);
    global_State* G = L->global;
    lua_State* r = G->callback_p;
    attachThreadToVM(env);
    if (r == NULL) return;

    lua_State* Ln = lua_newthread(L);
    lua_pushstring(Ln, viewId);
    auxresume(Ln, r, 1);

//     env->ReleaseStringUTFChars(view_id, viewId);
}

