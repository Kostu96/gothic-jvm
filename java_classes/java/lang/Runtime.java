package java.lang;

public class Runtime {

    public static Runtime getRuntime() { 
        return INSTANCE;
    }

    private Runtime() {}

    private static Runtime INSTANCE = new Runtime();

}
