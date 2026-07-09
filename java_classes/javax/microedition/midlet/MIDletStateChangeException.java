package javax.microedition.midlet;

import java.lang.String;

public class MIDletStateChangeException extends Exception {
    public MIDletStateChangeException() {}

    public MIDletStateChangeException(String s) {
	    super(s);
    }
}
