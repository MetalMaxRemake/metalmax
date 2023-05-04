package com.park.metalmax;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

public class NativeBridge {

    static {
        System.loadLibrary("metalmax");
    }

    private static final NativeBridge sInstance = new NativeBridge();

    public static NativeBridge getInstance() {
        return sInstance;
    }

    @FastNative
    public static native void changeBuffer();

    @FastNative
    public static native void onKeyEvent(int key);
    @FastNative
    public static native void onFuncKeyEvent(int key);

    @CriticalNative
    public static native void init();

    @FastNative
    public static native void onChange(int width, int height);

    @CriticalNative
    public static native void onDrawFrame();

}
