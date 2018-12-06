package hpatch;

/**
 * @author qiwenshuai
 * @note
 * @since 18-12-5 15:02 by jdk 1.8
 */
public class Hpatch {
    static {
        System.loadLibrary("hpatch");
    }

    public native int hpatch(String oldFilePath, String diffFilePath, String newFilePath);

    public static void main(String[] args) {
        new Hpatch().hpatch("123", "123", "123");
    }


}
