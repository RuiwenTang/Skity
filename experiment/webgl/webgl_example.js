
function degree_to_radius(deg) {
        return deg * 0.01745329251994329576923690768489;
}

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

        let x1 = y1 = y2 = 0;
        let x2 = 200;

        let colors = new skity.VectorUint32();
        colors.push_back(skity.ColorSetARGB(255, 0, 255, 255));
        colors.push_back(skity.ColorSetARGB(255, 0, 0, 255));
        colors.push_back(skity.ColorSetARGB(255, 255, 0, 0));
        let shader = skity.Shader.MakeLinear(x1, y1, x2, y2, colors);


        paint.setShader(shader);
        paint.setStyle(skity.Style.Fill);

        canvas.drawTextBlob(blob, 20, 224, paint);

        shader.delete();
        blob.delete();
}

function draw_dash_start_example(skity, canvas) {
        let path = new skity.Path();
        path.moveTo(199, 34);
        path.lineTo(253, 143);
        path.lineTo(374, 160);
        path.lineTo(287, 244);
        path.lineTo(307, 365);
        path.lineTo(199, 309);
        path.lineTo(97, 365);
        path.lineTo(112, 245);
        path.lineTo(26, 161);
        path.lineTo(146, 143);
        path.close();

        let paint = new skity.Paint();
        paint.setStyle(skity.Style.Fill);
        paint.setColor(skity.ColorSetARGB(255, 150, 150, 255));

        canvas.drawPath(path, paint);

        let pattern = new skity.VectorFloat();
        pattern.push_back(10);
        pattern.push_back(10);

        let effect = skity.PathEffect.MakeDashEffect(pattern);

        paint.setStrokeCap(skity.LineCap.Round);
        paint.setStrokeJoin(skity.LineJoin.Round);
        paint.setColor(skity.ColorSetARGB(255, 0, 0, 255));
        paint.setStrokeWidth(3.0);
        paint.setStyle(skity.Style.Stroke);
        paint.setPathEffect(effect);

        canvas.drawPath(path, paint);

        effect.delete();
}

function draw_linear_gradient_example(skity, canvas) {
        let paint = new skity.Paint();
        paint.setStyle(skity.Style.Fill);

        let colors = new skity.VectorUint32();

        colors.push_back(skity.ColorSetARGB(0, 0, 255, 255));
        colors.push_back(skity.ColorSetARGB(255, 0, 0, 255));
        colors.push_back(skity.ColorSetARGB(255, 255, 0, 0));

        let pos = new skity.VectorFloat();
        pos.push_back(0);
        pos.push_back(0.65);
        pos.push_back(1);

        for (let i = 0; i < 4; i++) {
                let block_x = (i % 2) * 100.0;
                // Fixme to solev float division
                let block_y = parseInt(i / 2) * 100.0;

                let matrix = new skity.Matrix();

                if (i / 2 == 1) {
                        matrix = skity.Matrix.Multiply(matrix, skity.Matrix.Translate(new skity.Matrix(), block_x, block_y));
                        matrix = skity.Matrix.Multiply(matrix, skity.Matrix.Rotate(new skity.Matrix(), degree_to_radius(45.0), 0, 0, 1));
                        matrix = skity.Matrix.Multiply(matrix, skity.Matrix.Translate(new skity.Matrix(), -block_x, -block_y));
                }

                let lgs = skity.Shader.MakeLinearWithPos(block_x, block_y, block_x + 50, block_y + 100, colors, pos);
                lgs.setLocalMatrix(matrix);
                paint.setShader(lgs);

                let rect = skity.Rect.MakeXYWH(block_x, block_y, 100, 100);

                canvas.drawRect(rect, paint);

                lgs.delete();
        }

        let radial_colors = new skity.VectorUint32();
        radial_colors.push_back(skity.ColorSetARGB(255, 255, 255, 255));
        radial_colors.push_back(skity.ColorSetARGB(255, 0, 0, 0));

        let path = new skity.Path();
        path.addCircle(220, 350, 100, skity.PathDirection.CW);

        let radial_shader = skity.Shader.MakeRadial(220, 350, 150, radial_colors);
        paint.setShader(radial_shader);

        canvas.drawPath(path, paint);

        radial_shader.delete();
}

function draw_even_odd_fill(skity, canvas) {
        let paint = new skity.Paint();
        paint.setStyle(skity.Style.Fill);
        paint.setColor(skity.ColorSetARGB(64, 255, 0, 0));

        let path = new skity.Path();

        path.moveTo(100, 10);
        path.lineTo(40, 180);
        path.lineTo(190, 60);
        path.lineTo(10, 60);
        path.lineTo(160, 180);
        path.close();

        canvas.drawPath(path, paint);

        canvas.save();
        canvas.translate(0, 200);

        path.setFillType(skity.PathFillType.EvenOdd);

        canvas.drawPath(path, paint);

        canvas.restore();
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
        canvas.translate(0, 300);
        draw_dash_start_example(skity, canvas);
        canvas.restore();

        canvas.save();
        canvas.translate(300, 0);
        draw_path_effect(skity, canvas);
        canvas.restore();

        if (typeface) {
                canvas.save();
                canvas.translate(520, 0);
                draw_simple_text(skity, canvas, typeface);
                canvas.restore();
        }

        canvas.save();
        canvas.translate(400, 300);
        draw_linear_gradient_example(skity, canvas);
        canvas.restore();

        canvas.save();
        canvas.translate(800, 0);
        draw_even_odd_fill(skity, canvas);
        canvas.restore();

        canvas.flush();
}

