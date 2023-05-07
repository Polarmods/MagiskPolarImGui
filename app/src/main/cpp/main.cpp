#include <EGL/egl.h> // need to make a common.h that contains all these headers cuz this is nasty
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include <fstream>
#include <sstream>
#include "nlohmann/json.hpp"
#include "http/cpr/cpr.h"
#include "Misc/Logging.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_android.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "Obfuscation/Obfuscate.h"
#include <stdio.h>
#include "Memory/KittyMemory.h"
#include <android/native_window_jni.h>
#include <jni.h>
#include <string>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>
#include "Misc/ByOpen/byopen.h"
#include "Misc/JNIStuff.h"
#include "Misc/FileWrapper.h"
#include "Misc/Utils.h"
#include "ByNameModding/BNM.hpp"
#include "Obfuscation/Custom_Obfuscate.h"
#include "Unity/Unity.h"
#include "Misc/FunctionPointers.h"
#include "Hooking/Hooks.h"
#include "Misc/ImGuiStuff.h"
#include "Menu.h"
#include "Hooking/JNIHooks.h"
#include "Unity/Screen.h"
#include "Unity/Input.h"
#include "main.h"
// the private version held by certain polarmodders has image loading and a lot more

EGLBoolean (*old_eglSwapBuffers)(...);
EGLBoolean new_eglSwapBuffers(EGLDisplay _display, EGLSurface _surface) {
    SetupImGui();
    Menu::DrawImGui();

    return old_eglSwapBuffers(_display, _surface);
}
//HOOKED(void, Input, void *thiz, void *ex_ab, void *ex_ac)
//{
////    auto io = ImGui::GetIO();
////    if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
////        return;
////    }
//    origInput(thiz, ex_ab, ex_ac);
//
//    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *)thiz);
//}

void *hack_thread(void *)
{
    using namespace BNM;
    KittyMemory::ProcMap il2cpp_base_map;
    do {
        sleep(1);
        il2cpp_base_map = KittyMemory::getLibraryMap("libil2cpp.so");
    } while (!il2cpp_base_map.isValid());

    LOGD("IL2CPP LOADED");
    auto eglhandle = dlopen("libEGL.so", RTLD_LAZY);
    const char *dlopen_error = dlerror();
    if (dlopen_error)
    {
        LOGE(OBFUSCATE("Cannot load dl 'egl': %s"), dlopen_error);
        eglhandle = dlopen("libunity.so", RTLD_LAZY); // I have no idea if this works it was just to me that it would fix crashes so I did it really quickly
    }
    auto eglSwapBuffers = dlsym(eglhandle, "eglSwapBuffers");
    const char *dlsym_error = dlerror();
    if (dlsym_error)
    {
        LOGE(OBFUSCATE("Cannot load symbol 'eglSwapBuffers': %s"), dlsym_error);
    } else
    {
        hook(eglSwapBuffers, (void *) new_eglSwapBuffers, (void **) &old_eglSwapBuffers);
    }
//    void *sym_input = DobbySymbolResolver(("/system/lib/libinput.so"), ("_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE"));
//    if (NULL != sym_input) {
//        DobbyHook(sym_input,(void*)myInput,(void**)&origInput);
//    }
    void *il2cppHandle = by_dlopen("libil2cpp.so", BY_RTLD_LAZY);
    const char *il2cpp_error = dlerror();
    if (il2cpp_error || !il2cppHandle)
    {
        LOGE(OBFUSCATE("Cannot load dl 'il2cpp': %s"), il2cpp_error);
        return NULL;
    }
    BNM::External::LoadBNM(il2cppHandle);

    AttachIl2Cpp(); // this is required when you use bynamemodding functions
    Unity::Screen::Setup();
    Pointers::LoadPointers();
    DetachIl2Cpp(); // remember to detach when you are done using bynamemodding functions
    return NULL;
}

void initialize()
{
    hook((void *) getEnv()->functions->RegisterNatives, (void *) hook_RegisterNatives,
         (void **) &old_RegisterNatives);
    int ret;
    pthread_t ntid;
    if ((ret = pthread_create(&ntid, nullptr, hack_thread, nullptr))) {
        LOGE("can't create thread: %s\n", strerror(ret));
    }
}
