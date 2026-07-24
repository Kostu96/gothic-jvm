package java.io;

import com.kostu96.gjvm.UnimplementedError;

public class ByteArrayOutputStream extends OutputStream {

    public ByteArrayOutputStream() {
        throw new UnimplementedError();
    }

    public ByteArrayOutputStream(int size) {
        throw new UnimplementedError();
    }

    public void close() throws IOException {
        throw new UnimplementedError();
    }

    public void reset() {
        throw new UnimplementedError();
    }

    public int size() {
        throw new UnimplementedError();
    }

    public byte[] toByteArray() {
        throw new UnimplementedError();
    }

    public String toString() {
        throw new UnimplementedError();
    }

    public void write(byte[] b, int off, int len) {
        throw new UnimplementedError();
    }

    public void write(int b) {
        throw new UnimplementedError();
    }

    protected byte[] buf;
    protected int count;
}
