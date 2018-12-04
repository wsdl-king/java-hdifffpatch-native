/**
 * @author qiwenshuai
 * @note
 * @since 18-12-4 15:08 by jdk 1.8
 */
public class HelloJNI {

    static {
        System.loadLibrary("hello");
    }

    private native  void sayHello();


    public static void main(String[] args) {
        new HelloJNI().sayHello();  // Create an instance and invoke the native method
    }
}
