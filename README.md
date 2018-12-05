# java-hdifffpatch-native
java 调用本地方法,基于hdiffpatch完成二进制文件拆分的代码

先写一个helloworld

----命令
~/IdeaProjects/java-hdifffpatch-native/src/main/java$ javac -h cpp  hello/HelloJNI.java

~/IdeaProjects/java-hdifffpatch-native/src/main/j`ava/hello/cpp$ gcc -fPIC -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" -shared -o libhello.so  HelloJNI.c

--文件 HelloJNI.c

然后生成libhello.so,将此文件加入到java文件的lib下

  
