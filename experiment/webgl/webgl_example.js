
/**
 * 
 * @param {WebGL2RenderingContext} gl 
 * @param {*} skity 
 */
function init_canvas(gl, skity, name) {
        gl.enable(gl.STENCIL_TEST);
        gl.enable(gl.BLEND);
        gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
        gl.clearColor(1.0, 1.0, 1.0, 1.0);
        gl.clearStencil(0x0);
        gl.stencilMask(0xFF);

        console.log('Canvas = ', skity.Canvas);

        let canvas = skity.Canvas.Make(name, 800, 600, 1);

        console.log('canvas = ', canvas);

        return canvas;
}

function draw_basic_example(skity, canvas) {
        let paint = new skity.Paint();
        paint.setColor(skity.ColorSetARGB(255, 0x42, 0x85, 0xF4));

        let rect = skity.Rect.MakeXYWH(10, 10, 100, 160);

        canvas.drawRect(rect, paint);

        let oval = new skity.RRect();
        oval.setOval(rect);
        oval.offset(40, 80);

        paint.setColor(skity.ColorSetARGB(255, 0xDB, 0x44, 0x37));
        canvas.drawRRect(oval, paint);

        paint.setColor(skity.ColorSetARGB(255, 0x0F, 0x9D, 0x58));
        canvas.drawCircle(180, 50, 25, paint);

        rect.offset(80, 50);
        paint.setStrokeWidth(4.0);
        paint.setStyle(skity.Style.Stroke);
        paint.setColor(skity.ColorSetARGB(255, 0xF4, 0xB4, 0));


        canvas.drawRoundRect(rect, 10, 10, paint);
}

function draw_path_effect(skity, canvas) {
        let R = 115.2;
        let C = 128.0;
        let path = new skity.Path();
        path.moveTo(C + R, C);
        for (let i = 1; i < 8; i++) {
                let a = 2.6927937 * i;
                path.lineTo(C + R * Math.cos(a), C + R * Math.sin(a));
        }

        let paint = new skity.Paint();
        let effect = skity.PathEffect.MakeDiscretePathEffect(10, 4, 0);
        paint.setPathEffect(effect);
        paint.setStyle(skity.Style.Stroke);
        paint.setStrokeWidth(2);
        paint.setColor(skity.ColorSetARGB(255, 0x42, 0x85, 0xF4));

        canvas.drawPath(path, paint);

        // free memory
        effect.delete();
}

function draw_simple_text(skity, canvas, typeface) {
        let paint = new skity.Paint();
        paint.setTextSize(64);
        paint.setColor(skity.ColorSetARGB(255, 0x42, 0x85, 0xF4));
        paint.setStyle(skity.Style.Fill);
        paint.setTypeface(typeface);

        let builder = new skity.TextBlobBuilder();

        let blob = builder.buildTextBlob("Skity", paint);

        canvas.drawTextBlob(blob, 20, 64, paint);

        
        paint.setStyle(skity.Style.Stroke);
        paint.setColor(skity.ColorSetARGB(255, 0xDB, 0x44, 0x37));
        paint.setStrokeWidth(2.0);

        canvas.drawTextBlob(blob, 20, 144, paint);


        blob.delete();
}

/**
 * 
 * @param {WebGL2RenderingContext} gl 
 * @param {*} canvas 
 */
function render(gl, skity, canvas, typeface) {
        gl.clear(gl.COLOR_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);

        draw_basic_example(skity, canvas);
        
        canvas.save();

        canvas.translate(300, 0);
        draw_path_effect(skity, canvas);
        canvas.restore();

        canvas.save();
        canvas.translate(520, 0);
        draw_simple_text(skity, canvas, typeface);
        canvas.restore();

        canvas.flush();
}

