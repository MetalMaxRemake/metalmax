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

    public static native void initNativeMethod();

    @CriticalNative
    public static native void commonTest();

    @CriticalNative
    public static native void onKeyEvent(int key);
    @CriticalNative
    public static native void onFuncKeyEvent(int key);

    @CriticalNative
    public static native void glInit();

    @CriticalNative
    public static native void glOnChange(int width, int height);

    @CriticalNative
    public static native void glOnDrawFrame();

}
