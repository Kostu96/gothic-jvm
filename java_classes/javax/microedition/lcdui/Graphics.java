package javax.microedition.lcdui;

public class Graphics {

    public void fillRect(int x, int y, int width, int height) {
        fillRect(x, y, width, height, color);
    }

    public void setColor(int rgb) {
        color = 0xFF000000 | (rgb & 0x00FFFFFF);
    }

    Graphics(Image image) {
        init(image);
    }

    private native void fillRect(int x, int y, int width, int height, int color);

    private native void init(Image image);

    private int color = -1;
}
