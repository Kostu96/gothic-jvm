package java.io;

import com.kostu96.gjvm.UnimplementedError;

public class ByteArrayInputStream extends InputStream {

    public ByteArrayInputStream(byte[] buf) {
        this.buf = buf;
        this.count = buf.length;
        this.pos = 0;
        this.mark = 0;
    }

    public ByteArrayInputStream(byte[] buf, int offset, int length) {
        this.buf = buf;
        this.pos = offset;
        this.count = Math.min(offset + length, buf.length);
        this.mark = offset;
    }

    public int available() {
        throw new UnimplementedError();
    }

    public void close() throws IOException {
        throw new UnimplementedError();
    }

    public void mark(int readLimit) {
        throw new UnimplementedError();
    }

    public boolean markSupported() {
        throw new UnimplementedError();
    }

    public int read() {
        return (pos < count) ? (buf[pos++] & 0xff) : -1;
    }

    public int read(byte[] b, int off, int len) {
        throw new UnimplementedError();
    }

    public void reset() {
        throw new UnimplementedError();
    }

    public long skip(long n) {
        throw new UnimplementedError();
    }

    protected byte[] buf;
    protected int count;
    protected int mark;
    protected int pos;
}
