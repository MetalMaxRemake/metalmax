package com.park.metalmax;

import android.graphics.Bitmap;
import android.view.Surface;

import dalvik.annotation.optimization.FastNative;

public class NativeBridge {

    static {
        System.loadLibrary("metalmax");
    }

    public static native void initNativeWindow(Surface surface);
    public static native void releaseNativeWindow();
    @FastNative
    public static native void slInit();
    @FastNative
    public static native void slRelease();
    @FastNative
    public static native void initNativeMethod();
    @FastNative
    public static native void commonTest();
    @FastNative
    public static native void onKeyEvent(int key);
    @FastNative
    public static native void onFuncKeyEvent(int key);

}
