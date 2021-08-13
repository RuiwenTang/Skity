# OpenGL Vector Graphics

`Skity` is an open-source 2D graphics library written in `c++` using [OpenGL](https://www.opengl.org) as backend.
Its **API** follows the same pattern as [Skia](https://skia.org/) and implements the rendering by myself. <br/>

## Example

same as [skia demo](https://fiddle.skia.org/c/66a829e00c752fe96e2ef4195cdc5454)<br/>
code is [example.cc](./example/example.cc)
<br/>
<img src="./resources/skia_demo.png" width="800" height="800">

## Simple Demo

![demo](./resources/gl_canvas_test.png)

## Anti-Alias test

![aa_test](./resources/aa_test.png)

## Build

### Requirements

- CMake
- [Freetype](https://www.freetype.org/)
- [GLFW](https://www.glfw.org/): for build test and example

```shell
# fetch sources from github
git clone --recursive https://github.com/RuiwenTang/Skity.git
cd Skity
# Create build directory
mkdir build
cd build
cmake ..
make
```

## Current status:

- Fill (only stencil, no even-odd support)

- Stroke (done)

- Clip (stencil)

- Font (in progress)

- Line caps and joins (done)

- PathEffect dash (in progress, fill with aa is not working)

  - implement a simple discrete path effect

- Image (in progress)

## Reference

- [GPU-accelerated Path Rendering](./resources/gpupathrender.pdf)
- [Resolution Independent Curve Rendering using Programmable Graphics Hardware](./resources/p1000-loop.pdf)
