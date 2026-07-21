package java.io;

public class ByteArrayInputStream extends InputStream {

    public ByteArrayInputStream(byte[] buf) {
        this.buf = buf;
        this.count = buf.length;
        this.pos = 0;
    }

    public int read() {
        return (pos < count) ? (buf[pos++] & 0xff) : -1;
    }

    protected byte[] buf;
    protected int count;
    protected int mark;
    protected int pos;
}
