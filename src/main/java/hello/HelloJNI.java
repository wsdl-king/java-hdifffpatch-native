package hello;

/**
 * @author qiwenshuai
 * @note
 * @since 18-12-4 17:55 by jdk 1.8
 */
public class HelloJNI {
    static {
        System.loadLibrary("hello"); // hello.dll (Windows) or libhello.so (Unixes)
    }
    // A native method that receives nothing and returns void
    private native void sayHello();

    public static void main(String[] args) {
        new HelloJNI().sayHello();  // invoke the native method
    }
}
