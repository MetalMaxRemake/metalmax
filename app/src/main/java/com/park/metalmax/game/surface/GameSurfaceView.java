package com.park.metalmax.game.surface;

import android.content.Context;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.park.metalmax.NativeBridge;

public class GameSurfaceView extends SurfaceView implements SurfaceHolder.Callback {
    private static final String TAG = "GameSurfaceView";

    public GameSurfaceView(Context context) {
        super(context);
        getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.d(TAG, "surfaceCreated");
        NativeBridge.initNativeWindow(holder.getSurface());
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.d(TAG, "surfaceChanged,width=" + width + ",height=" + height);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d(TAG, "surfaceDestroyed");
        NativeBridge.releaseNativeWindow();
    }
}
