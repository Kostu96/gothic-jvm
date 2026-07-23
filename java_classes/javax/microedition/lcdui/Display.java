package javax.microedition.lcdui;

import javax.microedition.midlet.*;

public class Display {

    public static Display getDisplay(MIDlet midlet) {
        if (INSTANCE == null)
            INSTANCE = new Display();
        return INSTANCE;
    }

    public int numAlphaLevels() {
        return 256; // 8-bits for alpha
    }

    public void setCurrent(Displayable disp) {
        current = disp;
    }

    static Graphics getGraphics() {
        if (GRAPHICS == null)
            GRAPHICS = new Graphics();
        return GRAPHICS;
    }

    private Display() {}

    private static Display INSTANCE;
    private static Graphics GRAPHICS;

    private Displayable current;
}
