package java.lang;

import com.kostu96.gjvm.ResourceInputStream;

import java.io.*;

/*
 * Instances of the class Class represent classes and interfaces in a running Java application.
 * Every array also belongs to a class that is reflected as a Class object that is shared by all arrays
 * with the same element type and number of dimensions.
 * 
 * Class has no public constructor. Instead Class objects are constructed automatically by the Java Virtual Machine
 * as classes are loaded.
 */
public final class Class {

    public static native Class forName(String name) throws ClassNotFoundException;

    public native String getName();

    public InputStream getResourceAsStream(String name) {
        //System.out.println(this.get);
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
