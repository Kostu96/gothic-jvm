package java.lang;

public class Throwable {

    public Throwable() {
        message = null;
    }

    public Throwable(String message) {
        this.message = message;
    }

    public String getMessage() {
        return message;
    }

    public void printStackTrace() {
        // TODO(Kostu): impl
    }

    public String toString() {
        String s = getClass().getName();
        return (message != null) ? (s + ": " + message) : s;
    }

    private String message;

}
