 #include <jni.h>
 #include <stdio.h>
 #include "hello_HelloJNI.h"

 JNIEXPORT void JNICALL Java_hello_HelloJNI_sayHello(JNIEnv *env, jobject thisObj) {
    printf("Hello World!\n");
    return;
 }