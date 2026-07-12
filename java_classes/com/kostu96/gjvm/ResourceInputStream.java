package com.kostu96.gjvm;

import java.io.*;

public class ResourceInputStream extends InputStream {
    public ResourceInputStream(String name) throws IOException {
        init(name);
    }

    public int read() throws IOException {
        return 0;
    }

    private native void init(String name);

}
