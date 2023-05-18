package com.park.metalmax;

import android.graphics.Bitmap;
import android.view.Surface;

import dalvik.annotation.optimization.FastNative;

public class NativeBridge {

    static {
        System.loadLibrary("metalmax");
    }

    private static final NativeBridge sInstance = new NativeBridge();

    public static NativeBridge getInstance() {
        return sInstance;
    }

    public static Bitmap getPxImg(String text) {
        Bitmap bitmap = Bitmap.createBitmap(8 * text.length(), 8, Bitmap.Config.ARGB_8888);
        NativeBridge.getCharImg(bitmap, text);
        return bitmap;
    }

    public static native void initNativeWindow(Surface surface);

    public static native void releaseNativeWindow();

    @FastNative
    public static native void slInit();
    @FastNative
    public static native void slRelease();

    @FastNative
    public static native void getAudioBuffer(short[] buffer);

    @FastNative
    public static native boolean getCharImg(Bitmap bitmap, String text);

    @FastNative
    public static native void initNativeMethod();

    @FastNative
    public static native void commonTest();

    @FastNative
    public static native void onKeyEvent(int key);
    @FastNative
    public static native void onFuncKeyEvent(int key);

}
