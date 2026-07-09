package javax.microedition.lcdui;

public abstract class Canvas {
    protected Canvas(boolean fullscreen) {}

    public native int getWidth();

    public native int getHeight();
}
