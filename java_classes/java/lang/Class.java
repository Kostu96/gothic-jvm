package java.lang;

import com.kostu96.gjvm.ResourceInputStream;

import java.io.*;

public final class Class {

    public String toString() {
        return (isInterface() ? "interface " :  "class ") + getName();
    }

    public static native Class forName(String className) throws ClassNotFoundException;

    public native Object newInstance() throws InstantiationException, IllegalAccessException;

    public native boolean isInstance(Object obj);

    public native boolean isAssignableFrom(Class cls);

    public native boolean isInterface();

    public native boolean isArray();

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

    private Class() {}

}
