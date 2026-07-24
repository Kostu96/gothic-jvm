package java.lang;

import com.kostu96.gjvm.io.*;
import java.io.*;

public final class Class {

    public static native Class forName(String name) throws ClassNotFoundException;

    public native String getName();

    public InputStream getResourceAsStream(String name) {
        try {
            if (name.length() > 0 && name.charAt(0) == '/') {
                /* Absolute format */
                name = name.substring(1);
            } else {
                /* Relative format */
                String className = this.getName();
                int dotIndex = className.lastIndexOf('.');
                if (dotIndex >= 0) {
                    name = className.substring(0, dotIndex + 1).replace('.', '/')
                           + name;
                }
            }
            return new ResourceInputStream(name);
        } catch (IOException x) {
            return null;
        }
    }

    // public native boolean isArray();

    // public native boolean isAssignableFrom(Class cls);

    // public native boolean isInstance(Object obj);

    public native boolean isInterface();

    // public native Object newInstance() throws InstantiationException, IllegalAccessException;

    public String toString() {
        return (isInterface() ? "interface " :  "class ") + getName();
    }

    private Class() {}

}
