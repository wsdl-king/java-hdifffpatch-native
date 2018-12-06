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
        new Hdiff().hdiff("/home/qiwenshuai/zhoubao2","/home/qiwenshuai/zhoubao","/home/qiwenshuai/zhoubao3");  // invoke the native method
    }
}
