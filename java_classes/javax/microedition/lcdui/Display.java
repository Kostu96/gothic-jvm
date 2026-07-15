package javax.microedition.lcdui;

import javax.microedition.midlet.*;

public class Display {

    public static Display getDisplay(MIDlet midlet) {
        if (INSTANCE == null)
            INSTANCE = new Display();
        return INSTANCE;
    }

    public void setCurrent(Displayable disp) {
        current = disp;
    }

    private Display() {}

    private static Display INSTANCE;

    private Displayable current;
}
