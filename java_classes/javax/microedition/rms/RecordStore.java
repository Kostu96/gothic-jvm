package javax.microedition.rms;

public class RecordStore {

    public native int
    addRecord(byte[] data, int offset, int size) throws
        RecordStoreNotOpenException, RecordStoreException, RecordStoreFullException;

    public void closeRecordStore() throws RecordStoreNotOpenException, RecordStoreException {
        closed = true;
    }

    public native int getNumRecords() throws RecordStoreNotOpenException;

    public int getSizeAvailable() throws RecordStoreNotOpenException {
        return 1024 * 1024;
    }

    public static native RecordStore
    openRecordStore(String recordStoreName, boolean createIfNecessary) throws
        RecordStoreException, RecordStoreFullException, RecordStoreNotFoundException;

    public native void
    setRecord(int recordId, byte[] newData, int offset, int size) throws
        RecordStoreNotOpenException, InvalidRecordIDException, RecordStoreException, RecordStoreFullException;
    
    private boolean closed = false;
    
}
