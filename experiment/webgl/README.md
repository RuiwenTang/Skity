# WebGL Example

## build with emsdk

```
mkdir build
cd build
emcmake cmake ../ -DBUILD_EXAMPLE=OFF -DBUILD_TEST=OFF -DBUILD_SVG_MODULE=OFF  -DBUILD_CODEC_MODULE=OFF -DVULKAN_BACKEND=OFF

emmake make skity_wasm
```

After build success, open minimal_webgl_example.html
