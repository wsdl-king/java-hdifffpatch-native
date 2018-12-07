# java-hdifffpatch-native
java 调用本地方法,基于hdiffpatch完成二进制文件拆分的代码

---- 以下命令均在 ../src/main/java/cpp 目录下完成

<h2>1.helloworld</h2>

---- hello_HelloJNI.h cmd

javac -h .   ../hello/HelloJNI.java 

---- libhello.so cmd

gcc -fPIC -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" -shared -o libhello.so  HelloJNI.c

--文件 HelloJNI.c

然后生成libhello.so,将此文件加入到java文件的lib下
 



<h2>2.hdiff/hpatch</h2>

研究过程中的两个坑---
第一 需要写文件全路径
第二 cpp之间要显示的指定


---- hdiff_Hdiff.h cmd

javac -h jni  ../hdiff/Hdiff.java 
不过需要修改生成的 hdiff_Hdiff.h,java生成的.h是不带入参的,需要修改一下
原来:
JNIEXPORT jint JNICALL Java_hdiff_Hdiff_hdiff
  (JNIEnv *, jobject, jstring, jstring, jstring);
  
修改为:
JNIEXPORT jint JNICALL Java_hdiff_Hdiff_hdiff
(JNIEnv *env, jobject thiz, jstring oldFilePath, jstring diffFilePath,
                                        jstring newFilePath);

---- diff  -> libhdiff.so cmd

 g++    -fPIC -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux"    -shared -o libhdiff.so      ../cpp/jni/hdiffz.cpp ../cpp/jni/HDiff/diff.cpp  jni/HDiff/private_diff/bytes_rle.cpp  jni/HDiff/private_diff/compress_detect.cpp  jni/HDiff/private_diff/suffix_string.cpp   jni/HDiff/private_diff/libdivsufsort/divsufsort.c jni/HDiff/private_diff/libdivsufsort/divsufsort64.c  jni/HDiff/private_diff/limit_mem_diff/adler_roll.c jni/HDiff/private_diff/limit_mem_diff/digest_matcher.cpp  jni/HDiff/private_diff/limit_mem_diff/stream_serialize.cpp   jni/HPatch/patch.c 



---- hpatch_Hpatch.h cmd

javac -h jni  ../hpatch/Hpatch.java 

不过需要修改生成的 hpatch_Hpatch.h,java生成的.h是不带入参的,需要修改一下

原来:
JNIEXPORT jint JNICALL Java_hpatch_Hpatch_hpatch
  (JNIEnv *, jobject, jstring, jstring, jstring);
  
修改为:
JNIEXPORT jint JNICALL Java_hpatch_Hpatch_hpatch
    (JNIEnv *env, jobject thiz, jstring oldFilePath, jstring diffFilePath,
                                        jstring newFilePath); 

---- patch -> libpatch.so cmd

g++    -fPIC -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux"    -shared -o libhpatch.so      ../cpp/jni/hpatchz.c   jni/HPatch/patch.c 



鸣谢 
housisong@gmail.com 
yanghui1986527@gmail.com
  
