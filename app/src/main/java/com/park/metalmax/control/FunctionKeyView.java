package com.park.metalmax.control;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.RectF;
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

    private RectF TARect, TBRect, ARect, BRect, XRect, YRect;
    private boolean TA, TB, A, B, X, Y;
    private int colorA, colorB, colorTA, colorTB, colorX, colorY;

    int key = 0;
    final int aVal = 0b0001;
    final int bVal = 0b0010;
    final int taVal = 0b0100;
    final int tbVal = 0b1000;
    final int xVal = 0b0001_0000;
    final int yVal = 0b0010_0000;

    @SuppressLint("DrawAllocation")
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        size = MeasureSpec.getSize(widthMeasureSpec);
        setMeasuredDimension(size, size * 3 / 2);
        XRect = new RectF(margin, margin, (size / 2) - margin, (size / 2) - margin);
        YRect = new RectF((size / 2) + margin, margin, size - margin, (size / 2) - margin);
        TARect = new RectF(margin, (size / 2) + margin, (size / 2) - margin, size - margin);
        TBRect = new RectF((size / 2) + margin, (size / 2) + margin, size - margin, size - margin);
        ARect = new RectF(margin, (size) + margin, (size / 2) - margin, size * 3 / 2 - margin);
        BRect = new RectF((size / 2) + margin, (size) + margin, size - margin, size * 3 / 2 - margin);

        colorA = Color.parseColor("#AA7DBC5F");
        colorB = Color.parseColor("#AAE75F5B");
        colorX = Color.parseColor("#AA4BA7EA");
        colorY = Color.parseColor("#AAEADB54");
        colorTA = Color.parseColor("#AA622E72");
        colorTB = Color.argb(170, 230, 230, 230);
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
                if (XRect.contains(x, y)) {
                    X = true;
                }
                if (YRect.contains(x, y)) {
                    Y = true;
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
                if (XRect.contains(x, y)) {
                    X = false;
                }
                if (YRect.contains(x, y)) {
                    Y = false;
                }
                break;
        }
        key = 0;
        if (A) {
            key |= aVal;
        }
        if (B) {
            key |= bVal;
        }
        if (TA) {
            key |= taVal;
        }
        if (TB) {
            key |= tbVal;
        }
        if (X) {
            key |= xVal;
        }
        if (Y) {
            key |= yVal;
        }
        NativeBridge.onFuncKeyEvent(key);
        invalidate();
        return true;
    }

    private static final int margin = 5;

    @Override
    protected void onDraw(Canvas canvas) {
        drawKey(canvas, TARect, TA, "TR A", colorTA);
        drawKey(canvas, TBRect, TB, "TR B", colorTB);
        drawKey(canvas, ARect, A, "A", colorA);
        drawKey(canvas, BRect, B, "B", colorB);
        drawKey(canvas, XRect, X, "X", colorX);
        drawKey(canvas, YRect, Y, "Y", colorY);
    }
}
