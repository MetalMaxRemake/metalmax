package com.park.metalmax.control;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.view.View;

import java.util.HashMap;
import java.util.Map;


public class KeyView extends View {

    private static final int radio = 20;
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

    private final Map<String, Bitmap> bitmapMap = new HashMap<>();
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

    protected void drawKey(Canvas canvas, RectF key, boolean pushed) {
        if (pushed) {
            mPaint.setColor(colorA);
        } else {
            mPaint.setColor(colorB);
        }
        canvas.drawRoundRect(key,radio, radio, mPaint);
    }

    @SuppressLint("NewApi")
    protected void drawKey(Canvas canvas, RectF key, boolean pushed, int color) {
        if (pushed) {
            mPaint.setColor(colorA);
        } else {
            mPaint.setColor(color);
        }
        canvas.drawRoundRect(key,radio, radio, mPaint);
    }

    protected void drawKey(Canvas canvas, RectF key, boolean pushed, String text) {
        drawKey(canvas, key, pushed);
        mPaint.setColor(Color.BLACK);
        mPaint.setTextSize(30);
        mPaint.setTypeface(Typeface.MONOSPACE);
        canvas.drawText(text,
                (key.left + key.right) / 2 - text.length() * 20/2,
                (key.top + key.bottom) / 2 + 10,
                mPaint);
    }

    protected void drawKey(Canvas canvas, RectF key, boolean pushed, String text, int color) {
        drawKey(canvas, key, pushed, color);
        mPaint.setColor(Color.WHITE);
        mPaint.setTextSize(30);
        Typeface font = Typeface.create(Typeface.MONOSPACE, Typeface.BOLD);
        mPaint.setTypeface(font);
        mPaint.setStrokeWidth(10);
        canvas.drawText(text,
                (key.left + key.right) / 2 - text.length() * 20/2,
                (key.top + key.bottom) / 2 + 10,
                mPaint);
    }

}
