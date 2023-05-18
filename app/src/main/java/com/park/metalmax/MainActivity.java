package com.park.metalmax;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

import com.park.metalmax.control.DirectKeyView;
import com.park.metalmax.control.FunctionKeyView;
import com.park.metalmax.game.surface.GameSurfaceView;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        super.onCreate(savedInstanceState);
        setContentView(initView());
        NativeBridge.initNativeMethod();
        NativeBridge.commonTest();
    }

    @Override
    protected void onResume() {
        super.onResume();
        NativeBridge.slInit();
    }

    @Override
    protected void onPause() {
        super.onPause();
        NativeBridge.slRelease();
    }

    private View initView() {
        //root
        FrameLayout rootLayout = new FrameLayout(MainActivity.this);
        ViewGroup.LayoutParams layoutParams = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT);
        rootLayout.setLayoutParams(layoutParams);

        //SurfaceView
        GameSurfaceView surfaceView = new GameSurfaceView(MainActivity.this);
        surfaceView.setLayoutParams(layoutParams);
        rootLayout.addView(surfaceView);

        //ControlView
        LinearLayout keyViewLayout = new LinearLayout(MainActivity.this);
        keyViewLayout.setLayoutParams(layoutParams);
        keyViewLayout.setOrientation(LinearLayout.HORIZONTAL);
        keyViewLayout.setGravity(Gravity.CENTER);

        LinearLayout.LayoutParams directKeyParams = new LinearLayout.LayoutParams(dpToPx(this, 170), ViewGroup.LayoutParams.WRAP_CONTENT);
        directKeyParams.gravity = Gravity.LEFT | Gravity.BOTTOM;
        DirectKeyView directKeyView = new DirectKeyView(MainActivity.this);
        directKeyView.setLayoutParams(directKeyParams);

        LinearLayout.LayoutParams functionKeyParams = new LinearLayout.LayoutParams(dpToPx(this, 140), ViewGroup.LayoutParams.WRAP_CONTENT);
        functionKeyParams.gravity = Gravity.RIGHT | Gravity.BOTTOM;
        FunctionKeyView functionKeyView = new FunctionKeyView(MainActivity.this);
        functionKeyView.setLayoutParams(functionKeyParams);

        LinearLayout.LayoutParams startKeyParams = new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT);
        startKeyParams.gravity = Gravity.CENTER | Gravity.BOTTOM;
        startKeyParams.weight = 1;
        View startSelectKeyView = new View(MainActivity.this);
        startSelectKeyView.setLayoutParams(startKeyParams);
        //todo impl start/select key
        keyViewLayout.addView(directKeyView);
        keyViewLayout.addView(startSelectKeyView);
        keyViewLayout.addView(functionKeyView);

        rootLayout.addView(keyViewLayout);
        return rootLayout;
    }

    public static int dpToPx(Context context, float dpValue) {
        // 获取屏幕密度
        final float scale = context.getResources().getDisplayMetrics().density;
        // 结果+0.5是为了int取整时更接近
        return (int) (dpValue * scale + 0.5f);
    }
}
