package com.example.skitydemo;


import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.opengl.GLES32;
import android.opengl.GLSurfaceView;
import android.os.Bundle;

import android.util.Log;
import android.view.SurfaceHolder;
import android.view.View;


import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import androidx.navigation.ui.AppBarConfiguration;
import androidx.navigation.ui.NavigationUI;

import com.example.skitydemo.databinding.ActivityMainBinding;

import android.view.Menu;
import android.view.MenuItem;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.logging.Logger;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends Activity implements GLSurfaceView.Renderer {

    private AppBarConfiguration appBarConfiguration;
    private ActivityMainBinding binding;
    private GLSurfaceView mGlSurfaceView;
    private GLFrameBuffer mFBO;
    private long mGLExampleInstance = 0;
    private int mVerCode = 100;
    private boolean mAuthorOK = false;

    static {
        System.loadLibrary("GLDemoJni");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);
        mAuthorOK =  AuthorizationHelper.VerifyStoragePermissions(this,mVerCode);
        if (mAuthorOK)
            CopyFiles();

        mGlSurfaceView = findViewById(R.id.glview);
        mGlSurfaceView.setPreserveEGLContextOnPause(false);
        mGlSurfaceView.setEGLContextClientVersion(3);
        mGlSurfaceView.setRenderer(this);
    }

    @Override
    public void onSurfaceCreated(GL10 var1, EGLConfig var2) {
        Log.e("tag", "onSurfaceCreated");
    }

    @Override
    public void onSurfaceChanged(GL10 var1, int var2, int var3) {
        Log.e("tag", "onSurfaceChanged");
        PrepareGLExample(var2,var3);
    }

    @Override
    public void onDrawFrame(GL10 var1) {
        GLES32.glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        GLES32.glClearStencil(0);
        GLES32.glClear(GLES32.GL_COLOR_BUFFER_BIT | GLES32.GL_STENCIL_BUFFER_BIT);
        GLES32.glEnable(GLES32.GL_BLEND);
        GLES32.glBlendFunc(GLES32.GL_SRC_ALPHA, GLES32.GL_ONE_MINUS_SRC_ALPHA);
        GLES32.glBlendEquation(GLES32.GL_FUNC_ADD);
        GLES32.glEnable(GLES32.GL_STENCIL_TEST);
        GLES32.glViewport(0, 0, mGlSurfaceView.getWidth(), mGlSurfaceView.getHeight());
        if (mGLExampleInstance != 0) {
            RunGLExample(mGLExampleInstance);
        }
        GLES32.glDisable(GLES32.GL_STENCIL_TEST);
        GLES32.glDisable(GLES32.GL_BLEND);
        Log.e("tag", "onDrawFrame");
    }

    @Override
    protected void onPause() {
        mGlSurfaceView.queueEvent(new Runnable() {
            @Override
            public void run() {
                if (mGLExampleInstance != 0)
                    ReleaseGLExample(mGLExampleInstance);
                if (mFBO != null)
                    mFBO.Release();
                mFBO = null;
                mGLExampleInstance = 0;
            }
        });
        super.onPause();
        Log.e("tag", "onPause");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.e("tag", "onResume");
    }

    //gl
    public void PrepareGLExample(int wid,int hei){
        if (mGLExampleInstance == 0 && mAuthorOK) {
            mGLExampleInstance = CreateGLExample(wid, hei);
            mFBO = new GLFrameBuffer();
            GLTexture texture = new GLTexture(wid, hei);
            GLRenderBuffer renderBuffer = new GLRenderBuffer(wid, hei, false);
            mFBO.AttachTexture(texture);
            mFBO.AttachDepthStencilBuffer(renderBuffer);
            assert GLES32.glGetError() == GLES32.GL_NO_ERROR;
        }
    }

    //file
    @Override
    public void onRequestPermissionsResult(int var1,  String[] var2,  int[] var3)
    {
        super.onRequestPermissionsResult(var1,var2,var3);
        CopyFiles();;
        mAuthorOK = true;
        mGlSurfaceView.queueEvent(new Runnable() {
            @Override
            public void run() {
                PrepareGLExample(mGlSurfaceView.getWidth(),mGlSurfaceView.getHeight());
            }
        });
    }

    public void CopyFiles(){
        FileHelper.copyFilesFromAssets(getApplicationContext(),"","/sdcard/SkityDemo/",false);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }



    private native long CreateGLExample(int width, int height);

    private native void RunGLExample(long instance);

    private native void ReleaseGLExample(long instance);
}