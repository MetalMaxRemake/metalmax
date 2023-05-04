package com.park.metalmax.control;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.view.View;

import com.park.metalmax.MainActivity;
import com.park.metalmax.NativeBridge;

import java.util.HashMap;
import java.util.Map;


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

    protected void drawKey(Canvas canvas, Rect key, boolean color) {
        if (color) {
            mPaint.setColor(colorA);
        } else {
            mPaint.setColor(colorB);
        }
        canvas.drawRect(key, mPaint);
    }

    private Bitmap getKeyBmp(String text) {
        Bitmap bitmapScale = bitmapMap.get(text);
        if(bitmapScale == null) {
            Bitmap bitmap = Bitmap.createBitmap(8 * text.length(), 8, Bitmap.Config.ARGB_8888);
            NativeBridge.getCharImg(bitmap, text);
            int scale = MainActivity.dpToPx(getContext(), 40) / 30;
            bitmapScale = Bitmap.createScaledBitmap(bitmap, scale * 8 * text.length(), scale * 8, false);
            bitmapMap.put(text, bitmapScale);
        }
        return bitmapScale;
    }

    protected void drawKey(Canvas canvas, Rect key, boolean color, String text) {
        drawKey(canvas, key, color);
        mPaint.setColor(Color.BLACK);
        mPaint.setTextSize(30);
        mPaint.setTypeface(Typeface.MONOSPACE);
        Bitmap bitmap = getKeyBmp(text);
        canvas.drawBitmap(bitmap,
                (key.left + key.right) / 2 - bitmap.getWidth()/2,
                (key.top + key.bottom) / 2,
                null);
    }

}
