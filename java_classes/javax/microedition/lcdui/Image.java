package javax.microedition.lcdui;

import java.io.*;

public class Image {

    public static native Image createImage(InputStream stream) throws IOException;

    public static Image createImage(int width, int height) {
        return new Image(width, height);
    }

    public static Image createImage(String name) throws IOException {
        // NOTE(Kostu): not exactly right but works because there is one class loader with shared classpaths
        InputStream stream = Image.class.getResourceAsStream(name);
        return createImage(stream);
    }

    public Graphics getGraphics() {
        return new Graphics(this);
    }

    public native void getRGB(int[] rgbData,
                              int offset, int scanLength,
                              int x, int y,
                              int width, int height);

    private Image(int width, int height) {
        init(width, height);
    }

    private native void init(int width, int height);
}
