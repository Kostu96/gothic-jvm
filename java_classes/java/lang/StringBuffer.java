package java.lang;

public final class StringBuffer {

    public StringBuffer() {
        size = 0;
        //capacity = 16;
        init();
    }

    public synchronized native StringBuffer append(String str);

    public StringBuffer append(int i) {
        return append(String.valueOf(i));
    }

    // public int capacity() {
    //     return capacity;
    // }

    public int length() {
        return size;
    }

    public String toString() {
        return new String(this);
    }

    private native void init();

    //private int capacity;
    private int size;

}
