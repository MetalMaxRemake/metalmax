package com.park.metalmax;

import android.graphics.Bitmap;

import dalvik.annotation.optimization.FastNative;

public class NativeBridge {

    static {
        System.loadLibrary("metalmax");
    }

    private static final NativeBridge sInstance = new NativeBridge();

    public static NativeBridge getInstance() {
        return sInstance;
    }

    public static native boolean getCharImg(Bitmap bitmap, String text);

    public static native void initNativeMethod();

    @FastNative
    public static native void commonTest();

    @FastNative
    public static native void onKeyEvent(int key);
    @FastNative
    public static native void onFuncKeyEvent(int key);

    @FastNative
    public static native void glInit();

    @FastNative
    public static native void glOnChange(int width, int height);

    @FastNative
    public static native void glOnDrawFrame();

}
