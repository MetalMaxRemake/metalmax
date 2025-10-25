package com.park.metalmax;

import android.view.Surface;

public class NativeBridge {

    static {
        System.loadLibrary("metalmax");
    }

    public static boolean initNativeWindow(Surface surface) {
        return initNativeWindowNative(surface);
    }
    public static void releaseNativeWindow() {
        releaseNativeWindowNative();
    }
    public static void slInit() {
        slInitNative();
    }
    public static void slRelease() {
        slReleaseNative();
    }
    public static void initLogic() {
        initLogicNative();
    }
    public static void onKeyEvent(int key) {
        onKeyEventNative(key);
    }
    public static void onFuncKeyEvent(int key) {
        onFuncKeyEventNative(key);
    }

    public static native boolean initNativeWindowNative(Surface surface);
    public static native void releaseNativeWindowNative();
    public static native void slInitNative();
    public static native void slReleaseNative();
    public static native void initLogicNative();
    public static native void onKeyEventNative(int key);
    public static native void onFuncKeyEventNative(int key);

}
