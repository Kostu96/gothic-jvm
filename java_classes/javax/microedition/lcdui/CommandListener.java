package javax.microedition.lcdui;

/*
 * This interface is used by applications which need to receive high-level events from the implementation.
 * An application will provide an implementation of a CommandListener (typically by using a nested class
 * or an inner class) and will then provide the instance to the addCommand method on a Displayable
 * in order to receive high-level events on that screen.
 *
 * The specification does not require the platform to create several threads for the event delivery.
 * Thus, if a CommandListener method does not return or the return is not delayed, the system may be blocked.
 * So, there is the following note to application developers:
 * - the CommandListener method should return immediately.
 */
public interface CommandListener {

    public void commandAction(Command c, Displayable d);

}
