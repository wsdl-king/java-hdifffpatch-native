# java-hdifffpatch-native
java 调用本地方法,基于hdiffpatch完成二进制文件拆分的代码

先写一个helloworld

----命令
~/IdeaProjects/java-hdifffpatch-native/src/main/java$ javac -h cpp  hello/HelloJNI.java

~/IdeaProjects/java-hdifffpatch-native/src/main/j`ava/hello/cpp$ gcc -fPIC -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" -shared -o libhello.so  HelloJNI.c

--文件 HelloJNI.c

然后生成libhello.so,将此文件加入到java文件的lib下

wait  thank u


------
研究过程中的两个坑---
第一 需要写文件全路径
第二 cpp之间要显示的指定
---编译命令
g++    -fPIC -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux"    -shared -o libhdiff.so      ../cpp/jni/Hdiff.cpp ../cpp/jni/HDiff/diff.cpp  jni/HDiff/private_diff/bytes_rle.cpp  jni/HDiff/private_diff/suffix_string.cpp  jni/HPatch/patch.c 



鸣谢 	housisong@gmail.com  yanghui1986527@gmail.com
  
