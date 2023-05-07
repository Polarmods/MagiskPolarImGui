#include <cstring>
#include <jni.h>
#include <pthread.h>
#include <cstdio>
#include "main.h"
#include "zygisk.hpp"
#include "Obfuscation/Obfuscate.h"
#include "Misc/Logging.h"
#include "ByNameModding/BNM.hpp"
#include "Misc/JNIStuff.h"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;
class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        env_ = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) {
            LOGE("Skip unknown process");
            return;
        }
        shouldInject = is_package(env_, args->app_data_dir);
    }

    void postAppSpecialize(const AppSpecializeArgs *) override {
        if (shouldInject) {
            env_->GetJavaVM(&jvm);
            //BNM::HardBypass(env_);
            initialize();
        }
    }

private:
    JNIEnv *env_{};
    bool shouldInject;
    int is_package(JNIEnv *env, jstring appDataDir)
    {
        if (!appDataDir)
            return 0;
        const char *app_data_dir = env->GetStringUTFChars(appDataDir, nullptr);
        int user = 0;
        static char package_name[256];
        if (sscanf(app_data_dir, "/data/%*[^/]/%d/%s", &user, package_name) != 2) {
            if (sscanf(app_data_dir, "/data/%*[^/]/%s", package_name) != 1) {
                package_name[0] = '\0';
                LOGW(OBFUSCATE("can't parse %s"), app_data_dir);
                return 0;
            }
        }
        if (strcmp(package_name, "com.blayzegames.iosfps") == 0) {
            LOGI(OBFUSCATE("we have found: %s"), package_name);
            env->ReleaseStringUTFChars(appDataDir, app_data_dir);
            return 1;
        } else {
            env->ReleaseStringUTFChars(appDataDir, app_data_dir);
            return 0;
        }
    }
};

REGISTER_ZYGISK_MODULE(MyModule)