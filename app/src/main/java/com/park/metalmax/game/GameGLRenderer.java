package com.park.metalmax.game;

import android.opengl.GLSurfaceView;

import com.park.metalmax.NativeBridge;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class GameGLRenderer implements GLSurfaceView.Renderer {
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        NativeBridge.init();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        NativeBridge.onChange(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        NativeBridge.onDrawFrame();
    }
}
