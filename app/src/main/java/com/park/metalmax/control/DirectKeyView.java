package com.park.metalmax.control;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.MotionEvent;

import com.park.metalmax.NativeBridge;

public class DirectKeyView extends KeyView {
    public DirectKeyView(Context context) {
        super(context);
    }

    public DirectKeyView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public DirectKeyView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    private int keyWidth;

    private Rect leftRect, rightRect, upRect, downRect;

    @SuppressLint("DrawAllocation")
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        keyWidth = (size / 5);
        int keyLength = keyWidth * 2;
        leftRect = new Rect(0, keyLength, keyLength, keyLength + keyWidth);
        rightRect = new Rect(keyLength + keyWidth, keyLength, size, keyLength + keyWidth);
        upRect = new Rect(keyLength, 0, keyLength + keyWidth, keyLength);
        downRect = new Rect(keyLength, keyLength + keyWidth, keyLength + keyWidth, size);
    }
    
    int key = 0;
    final int upVal = 0b0001;
    final int downVal = 0b0010;
    final int leftVal = 0b0100;
    final int rightVal = 0b1000;

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int x = (int) event.getX();
        int y = (int) event.getY();
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                if (leftRect.contains(x, y)) {
                    key |= leftVal;
                }
                if (rightRect.contains(x, y)) {
                    key |= rightVal;
                }
                if (upRect.contains(x, y)) {
                    key |= upVal;
                }
                if (downRect.contains(x, y)) {
                    key |= downVal;
                }
                NativeBridge.onKeyEvent(key);
                break;
            case MotionEvent.ACTION_UP:
                if (leftRect.contains(x, y)) {
                    key &= ~leftVal;
                }
                if (rightRect.contains(x, y)) {
                    key &= ~rightVal;
                }
                if (upRect.contains(x, y)) {
                    key &= ~upVal;
                }
                if (downRect.contains(x, y)) {
                    key &= ~downVal;
                }
                NativeBridge.onKeyEvent(key);
                break;
            case MotionEvent.ACTION_MOVE:

                break;
        }
        invalidate();
        return true;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        drawKey(canvas, leftRect, (key & leftVal) != 0);
        drawKey(canvas, rightRect, (key & rightVal) != 0);
        drawKey(canvas, upRect, (key & upVal) != 0);
        drawKey(canvas, downRect, (key & downVal) != 0);
    }
}
