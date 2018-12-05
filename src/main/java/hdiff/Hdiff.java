package hdiff;

/**
 * @author qiwenshuai
 * @note
 * @since 18-12-5 14:29 by jdk 1.8
 */
public class Hdiff {

    static {
        System.loadLibrary("hdiff");
    }

    public native int hdiff(String oldFilePath, String newFilePath, String diffFilePath);

    public static void main(String[] args) {
        new Hdiff().hdiff("123","323","~/home/qiwenshuai/chafen/A2/we.apk");  // invoke the native method
    }
}
