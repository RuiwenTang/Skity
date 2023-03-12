package com.example.skitydemo;

import android.graphics.Bitmap;
import android.opengl.GLES20;
import android.opengl.GLES32;

import java.nio.ByteBuffer;

public class GLFrameBuffer {
    public GLFrameBuffer(){
        int fbo[] = new int[1];
        GLES32.glGenFramebuffers(1,fbo,0);
        mName = fbo[0];
    }

    public void AttachTexture(GLTexture texture){
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER,mName);
        GLES32.glFramebufferTexture2D(GLES32.GL_FRAMEBUFFER,GLES32.GL_COLOR_ATTACHMENT0,GLES32.GL_TEXTURE_2D,texture.mName,0);
        int st = GLES32.glCheckFramebufferStatus(GLES32.GL_FRAMEBUFFER);
        assert(st == GLES32.GL_FRAMEBUFFER_COMPLETE);
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, 0);
        mTexture = texture;
        assert GLES32.glGetError() == GLES32.GL_NO_ERROR;
    }

    public void AttachDepthStencilBuffer(GLRenderBuffer renderBuffer){
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER,mName);
        GLES32.glFramebufferRenderbuffer(GLES32.GL_FRAMEBUFFER, GLES32.GL_DEPTH_ATTACHMENT, GLES32.GL_RENDERBUFFER, renderBuffer.mName);
        GLES32.glFramebufferRenderbuffer(GLES32.GL_FRAMEBUFFER, GLES32.GL_STENCIL_ATTACHMENT, GLES32.GL_RENDERBUFFER, renderBuffer.mName);
        int st = GLES32.glCheckFramebufferStatus(GLES32.GL_FRAMEBUFFER);
        assert(st == GLES32.GL_FRAMEBUFFER_COMPLETE);
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER, 0);
        mRenderBuffer =  renderBuffer;
        assert GLES32.glGetError() == GLES32.GL_NO_ERROR;
    }

    public void Release(){
        if (mTexture != null)
            mTexture.Release();;
        if (mRenderBuffer != null)
            mRenderBuffer.Release();;
    }

    public Bitmap CreateBitmap(){
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER,mName);
        ByteBuffer buffer = ByteBuffer.allocateDirect(mTexture.mWidth * mTexture.mHeight * 4);
        GLES20.glReadPixels(0,0,mTexture.mWidth,mTexture.mHeight,GLES20.GL_RGBA,GLES20.GL_UNSIGNED_BYTE,buffer);
        Bitmap bitmap = Bitmap.createBitmap(mTexture.mWidth, mTexture.mHeight, Bitmap.Config.ARGB_8888);
        bitmap.copyPixelsFromBuffer(buffer);
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER,0);
        return bitmap;
    }

    public void Bind(){
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER,mName);
        GLES32.glViewport(0,0,mTexture.mWidth,mTexture.mHeight);
    }

    public void Unbind(){
        GLES32.glBindFramebuffer(GLES32.GL_FRAMEBUFFER,0);
    }

    public int mName;
    public GLTexture mTexture;
    public GLRenderBuffer mRenderBuffer;
}
