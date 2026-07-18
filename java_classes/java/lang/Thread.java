package java.lang;

public class Thread implements Runnable {

    public Thread(Runnable target) {
        this.target = target;
    }

    public void run() {
        if (target != null) {
            target.run();
        }
    }

    public native void start();

    private Runnable target;

}
