package com.park.metalmax.game.gl;

import android.opengl.GLSurfaceView;
import android.os.SystemClock;
import android.util.Log;

import com.park.metalmax.NativeBridge;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class GameGLRenderer implements GLSurfaceView.Renderer {
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        NativeBridge.glInit();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        NativeBridge.glOnChange(width, height);
    }

    long startTime;

    @Override
    public void onDrawFrame(GL10 gl) {
//        startTime = SystemClock.elapsedRealtime();
        NativeBridge.glOnDrawFrame();
//        Log.d("onDrawFrameGL", "time cost:" + (SystemClock.elapsedRealtime() - startTime) + "ms");
    }
}
