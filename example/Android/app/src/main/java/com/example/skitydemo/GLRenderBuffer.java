package com.example.skitydemo;

import android.opengl.GLES32;

public class GLRenderBuffer {
    public GLRenderBuffer(int width,int height,boolean mutipleSample)
    {
        int buf[] = new int[1];
        GLES32.glGenRenderbuffers(1,buf,0);
        GLES32.glBindRenderbuffer(GLES32.GL_RENDERBUFFER,buf[0]);
        if (mutipleSample)
        {
            int tmp[] = new int[1];
            GLES32.glGetIntegerv(GLES32.GL_MAX_SAMPLES, tmp,0);
            GLES32.glRenderbufferStorageMultisample(GLES32.GL_RENDERBUFFER, tmp[0], GLES32.GL_DEPTH24_STENCIL8, width, height);
        }
        else
        {
            GLES32.glRenderbufferStorage(GLES32.GL_RENDERBUFFER,GLES32.GL_DEPTH24_STENCIL8,width,height);
        }
        GLES32.glBindRenderbuffer(GLES32.GL_RENDERBUFFER,0);
        mName = buf[0];
        mWidth = width;
        mHeight = height;
    }

    public void Release(){
        int buf[] = new int[1];
        buf[0] = mName;
        GLES32.glDeleteRenderbuffers(1,buf,0);
    }

    public int mName;
    public int mWidth;
    public int mHeight;
}
