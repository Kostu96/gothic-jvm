package javax.microedition.lcdui;

public abstract class Canvas extends Displayable {
    protected Canvas(boolean fullscreen) {}

    public native int getWidth();

    public native int getHeight();
}
