package java.lang;

public final class String {

    public String() {
        init();
    }

    public native char charAt(int index);

    public native int lastIndexOf(int ch);

    public int length() {
        return count;
    }

    public native String replace(char oldChar, char newChar);

    public String substring(int beginIndex) {
        return substring(beginIndex, length());
    }

    public native String substring(int beginIndex, int endIndex);

    private native void init();

    private char[] value;
    private int offset;
    private int count;
    private int hash;
}
