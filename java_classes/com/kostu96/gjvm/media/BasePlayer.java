package com.kostu96.gjvm.media;

import javax.microedition.media.*;

public class BasePlayer implements Player {

    public BasePlayer() {}

    public native void addPlayerListener(PlayerListener playerListener);

	public native void close();

	public native void deallocate();

	public native String getContentType();

    public native Control getControl(String controlType);

	public native Control[] getControls();

	public native long getDuration();

	public native long getMediaTime();

	public native int getState();
	
	public native void prefetch() throws MediaException;
	
	public native void realize() throws MediaException;

	public native void removePlayerListener(PlayerListener playerListener);

	public native void setLoopCount(int count);
	
	public native long setMediaTime(long now) throws MediaException;
	
	public native void start() throws MediaException;

	public native void stop() throws MediaException;

}
