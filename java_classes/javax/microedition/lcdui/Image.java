package javax.microedition.lcdui;

public class Image {

    public static Image createImage(int width, int height) {
        return new Image(width, height);
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
