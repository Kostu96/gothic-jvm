package com.kostu96.gjvm;

public class UnimplementedError extends VirtualMachineError {

    public UnimplementedError() {
        super();
    }

    public UnimplementedError(String s) {
        super(s);
    }

}
