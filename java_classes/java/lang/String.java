package java.lang;

public final class String {

    //public String() {
    //    init();
    //}

    public String(StringBuffer buffer) {
        synchronized(buffer) {
            size = buffer.length();
            init(buffer);
        }
    }

    public String(char value[], int offset, int size) {
        this.size = size;
        init(value, offset, size);
    }

    public native char charAt(int index);

    public native int lastIndexOf(int ch);

    public int length() {
        return size;
    }

    public native String replace(char oldChar, char newChar);

    public String substring(int beginIndex) {
        return substring(beginIndex, length());
    }

    public native String substring(int beginIndex, int endIndex);

    public static String valueOf(int i) {
        return Integer.toString(i, 10);
    }

    private native void init(StringBuffer buffer);
    private native void init(char value[], int offset, int size);

    private int size;
}
