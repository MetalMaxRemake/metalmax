package com.park.metalmax.game;

import android.content.Context;
import android.graphics.Canvas;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import com.park.metalmax.MainActivity;

public class GameGLView extends GLSurfaceView {
    public GameGLView(Context context) {
        super(context);
    }

    public GameGLView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        buildDrawingCache();
        getDrawingCache();
        canvas.drawText("park_test_string", 0, 0, null);
    }
}
