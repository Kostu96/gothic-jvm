package javax.microedition.lcdui;

public abstract class Canvas extends Displayable {

    protected Canvas() {}

    public native int getWidth();

    public native int getHeight();

    protected abstract void paint(Graphics g);

    public final void repaint() {
        repaint(0, 0, getWidth(), getHeight());
    }

    public final void repaint(int x, int y, int width, int height) {
        repaintPending = true;
    }

    public final void serviceRepaints() {
        if (repaintPending) {
            repaintPending = false;
            paint(Display.getGraphics());
        }
    }

    public void setFullScreenMode(boolean mode) {}
    
    private boolean repaintPending = false;
}
