package com.example.skitydemo;

import android.opengl.GLES32;

public class GLTexture {

    public GLTexture(int wid,int hei){
        int tex[] = new int[1];
        GLES32.glGenTextures(1,tex,0);
        GLES32.glActiveTexture(GLES32.GL_TEXTURE1);
        GLES32.glBindTexture(GLES32.GL_TEXTURE_2D,tex[0]);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_WRAP_S, GLES32.GL_CLAMP_TO_EDGE);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_WRAP_T, GLES32.GL_CLAMP_TO_EDGE);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_WRAP_R, GLES32.GL_CLAMP_TO_EDGE);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MAG_FILTER, GLES32.GL_LINEAR);
        GLES32.glTexParameteri(GLES32.GL_TEXTURE_2D, GLES32.GL_TEXTURE_MIN_FILTER, GLES32.GL_LINEAR);
        GLES32.glTexImage2D(GLES32.GL_TEXTURE_2D,0,GLES32.GL_RGBA,wid,hei,0,GLES32.GL_RGBA,GLES32.GL_UNSIGNED_BYTE,null);
        mName = tex[0];
        mWidth = wid;
        mHeight = hei;
    }

    public void Release(){
        int buf[] = new int[1];
        buf[0] = mName;
        GLES32.glDeleteTextures(1,buf,0);
    }

    public int mName;
    public int mWidth;
    public int mHeight;
}
