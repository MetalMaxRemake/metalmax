package com.park.metalmax;

import android.view.Surface;

public class NativeBridge {

    static {
        System.loadLibrary("metalmax");
    }

    public static native void initNativeWindow(Surface surface);
    public static native void releaseNativeWindow();
    public static native void slInit();
    public static native void slRelease();
    public static native void initLogic();
    public static native void onKeyEvent(int key);
    public static native void onFuncKeyEvent(int key);

}
