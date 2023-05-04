package com.park.metalmax.control;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.MotionEvent;

import com.park.metalmax.NativeBridge;

public class FunctionKeyView extends KeyView {
    public FunctionKeyView(Context context) {
        super(context);
    }

    public FunctionKeyView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public FunctionKeyView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    private Rect TARect, TBRect, ARect, BRect;
    private boolean TA, TB, A, B;

    int key = 0;
    final int aVal = 0b0001;
    final int bVal = 0b0010;
    final int taVal = 0b0100;
    final int tbVal = 0b1000;

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        TARect = new Rect(margin, margin, (size / 2) - margin, (size / 2) - margin);
        TBRect = new Rect((size / 2) + margin, margin, size - margin, (size / 2) - margin);
        ARect = new Rect(margin, (size / 2) + margin, (size / 2) - margin, size - margin);
        BRect = new Rect((size / 2) + margin, (size / 2) + margin, size - margin, size - margin);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int x = (int) event.getX();
        int y = (int) event.getY();
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                if (TARect.contains(x, y)) {
                    TA = true;
                }
                if (TBRect.contains(x, y)) {
                    TB = true;
                }
                if (ARect.contains(x, y)) {
                    A = true;
                }
                if (BRect.contains(x, y)) {
                    B = true;
                }
                break;
            case MotionEvent.ACTION_UP:
                if (TARect.contains(x, y)) {
                    TA = false;
                }
                if (TBRect.contains(x, y)) {
                    TB = false;
                }
                if (ARect.contains(x, y)) {
                    A = false;
                }
                if (BRect.contains(x, y)) {
                    B = false;
                }
                break;
        }
        key = 0;
        if(A) {
            key |= aVal;
        }
        if(B) {
            key |= bVal;
        }
        if(TA) {
            key |= taVal;
        }
        if(TB) {
            key |= tbVal;
        }
        NativeBridge.onFuncKeyEvent(key);
        invalidate();
        return true;
    }

    private static final int margin = 5;

    @Override
    protected void onDraw(Canvas canvas) {
        drawKey(canvas, TARect, TA, "turbo A");
        drawKey(canvas, TBRect, TB, "turbo B");
        drawKey(canvas, ARect, A, "A");
        drawKey(canvas, BRect, B, "B");
    }
}
