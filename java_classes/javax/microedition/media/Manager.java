package javax.microedition.media;

import com.kostu96.gjvm.media.*;
import java.io.*;

public final class Manager extends Object {

    public static Player createPlayer(InputStream stream, String type) throws IOException, MediaException {
        return new BasePlayer();
    }

}
