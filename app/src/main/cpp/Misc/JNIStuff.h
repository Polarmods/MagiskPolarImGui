#ifndef JNISTUFF
#define JNISTUFF
extern JavaVM *jvm;
extern jclass UnityPlayer_cls;
extern jfieldID UnityPlayer_CurrentActivity_fid;
extern JNIEnv* getEnv();
extern jobject getGlobalContext(JNIEnv *env);
extern void displayKeyboard(bool pShow);
#endif JNISTUFF