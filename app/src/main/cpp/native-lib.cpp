#include <jni.h>
#include <string>

extern "C"
JNIEXPORT jstring

JNICALL
Java_media_ushow_webrtcdemo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++ By xiaokai";
    return env->NewStringUTF(hello.c_str());
}
