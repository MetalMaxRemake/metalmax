package com.park.metalmax.control;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.View;


public class KeyView extends View {
    public KeyView(Context context) {
        super(context);
    }

    public KeyView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public KeyView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    protected int size;
    protected Paint mPaint = new Paint();
    protected int colorA = Color.argb(170, 170, 0, 0),
            colorB = Color.argb(200, 230, 230, 230);


    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        mPaint.setColor(colorB);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        size = MeasureSpec.getSize(widthMeasureSpec);
        setMeasuredDimension(size, size);
    }

    protected void drawKey(Canvas canvas, Rect key, boolean color) {
        if (color) {
            mPaint.setColor(colorA);
        } else {
            mPaint.setColor(colorB);
        }
        canvas.drawRect(key, mPaint);
    }

    protected void drawKey(Canvas canvas, Rect key, boolean color, String text) {
        drawKey(canvas, key, color);
        mPaint.setColor(Color.BLACK);
        mPaint.setTextSize(30);
        canvas.drawText(text, (key.left + key.right) / 2 - text.length() * 7, (key.top + key.bottom) / 2, mPaint);
    }

}
