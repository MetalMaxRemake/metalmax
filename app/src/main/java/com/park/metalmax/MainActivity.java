package com.park.metalmax;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.Toast;

import com.park.metalmax.control.DirectKeyView;
import com.park.metalmax.control.FunctionKeyView;
import com.park.metalmax.game.GameGLRenderer;
import com.park.metalmax.game.GameGLView;

public class MainActivity extends Activity {

    Handler mainHandler = new Handler(Looper.getMainLooper());

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        super.onCreate(savedInstanceState);
        setContentView(initView());
        NativeBridge.initNativeMethod();
        mainHandler.postDelayed(() -> {
            try {
                NativeBridge.commonTest();
            }catch (Throwable tr){
                tr.printStackTrace();
                Toast.makeText(MainActivity.this, "native test failed! exiting ...", Toast.LENGTH_SHORT).show();
                finish();
            }
        }, 1000);
    }

    private GameGLView mGameGlView;

    private View initView() {
        //root
        FrameLayout rootLayout = new FrameLayout(MainActivity.this);
        ViewGroup.LayoutParams layoutParams = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT);
        rootLayout.setLayoutParams(layoutParams);
        //GLView
        GameGLView gameGLView = new GameGLView(MainActivity.this);
        gameGLView.setLayoutParams(layoutParams);
        GLSurfaceView.Renderer renderer = new GameGLRenderer();
        gameGLView.setEGLContextClientVersion(2);
        gameGLView.setRenderer(renderer);
        gameGLView.setDrawingCacheEnabled(true);
        mGameGlView = gameGLView;
        rootLayout.addView(gameGLView);
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

    /**
     * 根据手机的分辨率从 px(像素) 的单位 转成为 dp(相对大小)
     */
    public static int pxToDp(Context context, float pxValue) {
        final float scale = context.getResources().getDisplayMetrics().density;
        return (int) (pxValue / scale + 0.5f);
    }
}
