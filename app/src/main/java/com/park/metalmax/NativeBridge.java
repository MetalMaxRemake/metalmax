package com.park.metalmax;

import dalvik.annotation.optimization.CriticalNative;

public class NativeBridge {

    static {
        System.loadLibrary("metalmax");
    }

    private static final NativeBridge sInstance = new NativeBridge();

    public static NativeBridge getInstance() {
        return sInstance;
    }

    @CriticalNative
    public static native void changeBuffer();

    @CriticalNative
    public static native void onKeyEvent(int key);
    @CriticalNative
    public static native void onFuncKeyEvent(int key);

    @CriticalNative
    public static native void init();

    @CriticalNative
    public static native void onChange(int width, int height);

    @CriticalNative
    public static native void onDrawFrame();

}
