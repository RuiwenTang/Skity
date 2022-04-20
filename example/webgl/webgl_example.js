
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

        rect.offset(80, 50);

        paint.setStrokeWidth(4.0);
        paint.setStyle(skity.Style.Stroke);
        paint.setColor(skity.ColorSetARGB(255, 0xF4, 0xB4, 0));

        console.log('stroke = ', skity.Style.Stroke);

        canvas.drawRoundRect(rect, 10, 10, paint);
}

/**
 * 
 * @param {WebGL2RenderingContext} gl 
 * @param {*} canvas 
 */
function render(gl, skity, canvas) {
        gl.clear(gl.COLOR_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
        
        draw_basic_example(skity, canvas);
        

        canvas.flush();
}

