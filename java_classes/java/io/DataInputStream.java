package java.io;

import com.kostu96.gjvm.UnimplementedError;

public class DataInputStream extends InputStream implements DataInput {

    public DataInputStream(InputStream in) {
        this.in = in;
    }

    public int available() throws IOException {
        throw new UnimplementedError();
    }

    public void close() throws IOException {
        in.close();
    }

    public void mark(int readLimit) {
        throw new UnimplementedError();
    }

    public boolean markSupported() {
        throw new UnimplementedError();
    }

    public int read() throws IOException {
        return in.read();
    }

    public int read(byte[] b) throws IOException {
        throw new UnimplementedError();
    }

    public int read(byte[] b, int off, int len) throws IOException {
        throw new UnimplementedError();
    }

    public final boolean readBoolean() throws IOException {
        int ch = read();
        if (ch < 0) {
            throw new EOFException();
        }
        return (ch != 0);
    }

    public final byte readByte() throws IOException {
        int ch = read();
        if (ch < 0) {
            throw new EOFException();
        }
        return (byte)(ch);
    }

    public final char readChar() throws IOException {
        return (char)readUnsignedShort();
    }

    public final double readDouble() throws IOException {
        return Double.longBitsToDouble(readLong());
    }

    public final float readFloat() throws IOException {
        return Float.intBitsToFloat(readInt());
    }

    public final void readFully(byte b[]) throws IOException {
        readFully(b, 0, b.length);
    }

    public final void readFully(byte b[], int off, int len) throws IOException {
        if (len < 0) {
            throw new IndexOutOfBoundsException();
        }
        int n = 0;
        while (n < len) {
            int count = read(b, off + n, len - n);
            if (count < 0) {
                throw new EOFException();
            }
            n += count;
        }
    }

    public final int readInt() throws IOException {
        int ch1 = read();
        int ch2 = read();
        int ch3 = read();
        int ch4 = read();
        if ((ch1 | ch2 | ch3 | ch4) < 0) {
             throw new EOFException();
        }
        return ((ch1 << 24) + (ch2 << 16) + (ch3 << 8) + (ch4 << 0));
    }

    public final long readLong() throws IOException {
        return ((long)(readInt()) << 32) + (readInt() & 0xFFFFFFFFL);
    }

    public final short readShort() throws IOException {
        return (short)readUnsignedShort();
    }

    public final int readUnsignedByte() throws IOException {
        int ch = read();
        if (ch < 0) {
            throw new EOFException();
        }
        return ch;
    }

    public final int readUnsignedShort() throws IOException {
        int ch1 = read();
        int ch2 = read();
        if ((ch1 | ch2) < 0) {
             throw new EOFException();
        }
        return (ch1 << 8) + (ch2 << 0);
    }

    public final String readUTF() throws IOException {
        return readUTF(this);
    }

    public final static String readUTF(DataInput in) throws IOException {
        int utflen = in.readUnsignedShort();
        char str[] = new char[utflen];
        byte bytearr [] = new byte[utflen];
        int c, char2, char3;
        int count = 0;
        int strlen = 0;

        in.readFully(bytearr, 0, utflen);

        while (count < utflen) {
            c = (int) bytearr[count] & 0xff;
            switch (c >> 4) {
                case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
                    /* 0xxxxxxx*/
                    count++;
                    str[strlen++] = (char)c;
                    break;
                case 12: case 13:
                    /* 110x xxxx   10xx xxxx*/
                    count += 2;
                    if (count > utflen)
                        throw new UTFDataFormatException();
                    char2 = (int) bytearr[count-1];
                    if ((char2 & 0xC0) != 0x80)
                        throw new UTFDataFormatException();
                    str[strlen++] = (char)(((c & 0x1F) << 6) | (char2 & 0x3F));
                    break;
                case 14:
                    /* 1110 xxxx  10xx xxxx  10xx xxxx */
                    count += 3;
                    if (count > utflen)
                        throw new UTFDataFormatException();
                    char2 = (int) bytearr[count-2];
                    char3 = (int) bytearr[count-1];
                    if (((char2 & 0xC0) != 0x80) || ((char3 & 0xC0) != 0x80))
                        throw new UTFDataFormatException();
                    str[strlen++] = (char)(((c     & 0x0F) << 12) |
                                           ((char2 & 0x3F) << 6)  |
                                           ((char3 & 0x3F) << 0));
                    break;
                default:
                    /* 10xx xxxx,  1111 xxxx */
                    throw new UTFDataFormatException();
                }
        }
        // The number of chars produced may be less than utflen
        return new String(str, 0, strlen);
    }

    public void reset() throws IOException {
        throw new UnimplementedError();
    }

    public long skip(long n) throws IOException {
        throw new UnimplementedError();
    }

    public final int skipBytes(int n) throws IOException {
        int total = 0;
        int cur = 0;

        while ((total<n) && ((cur = (int) skip(n-total)) > 0)) {
            total += cur;
        }
        return total;
    }

    protected InputStream in;
}
