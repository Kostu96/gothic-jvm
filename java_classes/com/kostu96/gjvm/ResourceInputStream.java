package com.kostu96.gjvm;

import java.io.*;

public class ResourceInputStream extends InputStream {
    public ResourceInputStream(String name) throws IOException {
        init(name);
    }

    public native int read() throws IOException;

    private native void init(String name) throws IOException;

}
