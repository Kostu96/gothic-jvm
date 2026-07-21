package javax.microedition.lcdui;

public abstract class Canvas extends Displayable {

    protected Canvas() {}

    public native int getWidth();

    public native int getHeight();

    public final void repaint() {
        repaint(0, 0, getWidth(), getHeight());
    }

    public final native void repaint(int x, int y, int width, int height);

    public final native void serviceRepaints();

    public void setFullScreenMode(boolean mode) {}
    
}
