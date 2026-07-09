package java.lang;

public final class System {

    //public static InputStream in;
    //public static PrintStream out;
    //public static PrintStream err;

    public static native long currentTimeMillis();

    public static native void arraycopy(Object src, int srcOffset,
                                        Object dst, int dstOffset,
                                        int length);

    public static native int identityHashCode(Object x);

    public static native String getProperty(String key);

    public static native void exit(int status);

    public static native void gc();

    private System() {}
}
