# Skity

`Skity` is an open-source 2D graphics library written in `c++` using [OpenGL](https://www.opengl.org) and [Vulkan (Experimental)](https://www.vulkan.org/) as backend.
Its **API** follows the same pattern as [Skia](https://skia.org/) and implements the rendering by myself. <br/>

![MacOS|build](https://github.com/RuiwenTang/Skity/actions/workflows/macos.yml/badge.svg)
![Windows|build](https://github.com/RuiwenTang/Skity/actions/workflows/windows.yml/badge.svg)
![Android|build](https://github.com/RuiwenTang/Skity/actions/workflows/android.yml/badge.svg)
[![CodeFactor](https://www.codefactor.io/repository/github/ruiwentang/skity/badge)](https://www.codefactor.io/repository/github/ruiwentang/skity)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## Example

Working in progress nanovg demo code is [frame_example.cc](./example/frame_example.cc) <br/>

![nanovg](./resources/nanovg_demo.png)

Parse and render svg file: [tiger.svg](./example/images/tiger.svg)
![tiger.svg](./resources/tiger.png)

same as [skia demo](https://fiddle.skia.org/c/66a829e00c752fe96e2ef4195cdc5454)<br/>
code is [example.cc](./example/example.cc)
<br/>
<img src="./resources/skia_demo.png" width="800" height="800">

## Android Demo

code is in [Skity-Android](https://github.com/RuiwenTang/Skity-Android)

![Android Demo](https://github.com/RuiwenTang/Skity-Android/blob/main/screen_shots/gl_frame_example_android.png?raw=true)

## Build

### Third party dependencies

- [glad](https://glad.dav1d.de/)
  Uesd in example for OpenGL context creation
- [glm](https://github.com/g-truc/glm.git)
  Used in this library for all geometric mathematical claculations
- [gtest](https://github.com/google/googletest.git)
  Used in test for some unit test only used when build for debug
- [pugixml](https://github.com/zeux/pugixml.git)
  Used in this library for xml parse when drawing svg image. It will removed to a module directory in future
- [spdlog](https://github.com/gabime/spdlog.git)
  Used in this library for logging. (Optional can trun off by setting `ENABLE_LOG=OFF`)
- [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git)
  Used when enable `VULKAN_BACKEND` for internal memory management

### CMake options

| CMake Option         | Default Value | Description                                                                                                                                                                            |
| -------------------- | ------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **ENABLE_LOG**       | ON            | Enable logging. If turn off the [spdlog](https://github.com/gabime/spdlog.git) is no longer needed.                                                                                    |
| **VULKAN_BACKEND**   | OFF           | Enable [Vulkan](https://www.vulkan.org/) backend. If turn on, the [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git) dependence is needed. |
| **OPENGL_BACKEND**   | ON            | Enable [OpenGL](https://www.opengl.org) backend                                                                                                                                        |
| **BUILD_SVG_MODULE** | ON            | Build SVG module. If turn off the [pugixml](https://github.com/zeux/pugixml.git) is no longer needed.                                                                                  |
| **BUILD_EXAMPLE**    | ON            | Build [example code](./example/)                                                                                                                                                       |
| **BUILD_TEST**       | ON            | Build [test code](./test)                                                                                                                                                              |

### Requirements

- CMake
- [Freetype](https://www.freetype.org/): If not present, font rendering will not working
- [GLFW](https://www.glfw.org/): for build test and example
- optional
  - [libpng](http://www.libpng.org/pub/png/libpng.html): for png file decode
  - [libjpeg-turbo](https://www.libjpeg-turbo.org/): for jpg file decode
  - on windows ,need to set environment value: `JPEG_PREFIX=path to libjpeg installed directory`

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

- Font (basic drawing api)

  - draw simple text, no glyph cache, no text layout

- Line caps and joins (done)

- PathEffect dash

  - implement a simple discrete path effect
  - implement a path measure algorithm and dash path effect

- Image (in progress)

  - Cocec interface
  - png image decode (need install [libpng](http://www.libpng.org/pub/png/libpng.html))
  - jpg image decode (need install [libjpeg-turbo](https://www.libjpeg-turbo.org/))

- SVG (in progress)
  - basic svg tag parser
    - `<svg>` `<g>` `<path>` `<circle>` `<rect>`

## Reference

- [GPU-accelerated Path Rendering](./resources/gpupathrender.pdf)
- [Resolution Independent Curve Rendering using Programmable Graphics Hardware](./resources/p1000-loop.pdf)

## TODO

- [x] Vulkan backend support (Experimental done)

- [ ] CPU backend support (planning)

- [ ] Support [lottie](https://airbnb.design/lottie/) anmiation.

- [ ] Support mask filters like [SkMaskFilter](https://api.skia.org/classSkMaskFilter.html)
