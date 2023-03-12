#include <jni.h>
#include <skity/skity.hpp>
#include <skity/gpu/gpu_context.hpp>


//
// Created by caichao on 2023/3/12.
//

//implements in example.cc
void draw_canvas(skity::Canvas* canvas);

class GLExample{
public:
    explicit GLExample(int wid, int hei){
        assert(wid > 0 && hei > 0);
        skity::GPUContext ctx{skity::GPUBackendType::kOpenGL,
                              nullptr};
        mCanvas = skity::Canvas::MakeHardwareAccelationCanvas(wid,hei,1, &ctx);
        mCanvas->setDefaultTypeface(
                skity::Typeface::MakeFromFile("/sdcard/SkityDemo/Roboto Mono Nerd Font Complete.ttf"));
    }

    void Render(){

//        skity::Paint paint;
//        paint.setFillColor({0.0,1.0,0.0,1.0});
//        paint.setAlphaF(1);
//        paint.setStyle(skity::Paint::kFill_Style);
//
//        skity::Path path;
//        path.addRect({00,00,300,300});
//        mCanvas->drawPath(path,paint);
//        mCanvas->flush();

        draw_canvas(mCanvas.get());
        mCanvas->flush();
    }

protected:
    std::unique_ptr<skity::Canvas> mCanvas;
};


extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_skitydemo_MainActivity_CreateGLExample(JNIEnv *env, jobject thiz,jint width,jint height) {
    auto glExample = new GLExample(width,height);
    return (long)(glExample);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_skitydemo_MainActivity_RunGLExample(JNIEnv *env, jobject thiz, jlong instance) {
    GLExample *example = (GLExample*)instance;
    example->Render();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_skitydemo_MainActivity_ReleaseGLExample(JNIEnv *env, jobject thiz,
                                                         jlong instance) {
    GLExample *example = (GLExample*)instance;
    delete example;
}