package java.lang;

public class Throwable {

    public Throwable() {
        // TODO(Kostu): impl
    }

    public Throwable(String message) {
        this.message = message;
    }

    public String getMessage() {
        return message;
    }

    private String message;

}
