if(NOT IOS AND NOT ANDROID)
    option(BUILD_EXAMPLE "option for building example" ON)
    option(BUILD_TEST "option for building test" ON)
    option(BUILD_SVG_MODULE "option for build svg module" ON)
    option(BUILD_CODEC_MODULE "option for build codec module" ON)
    # logging option
    option(ENABLE_LOG "option for logging" ON)
    option(CPU_BACKEND "option for cpu raster" ON)
else()
    option(BUILD_EXAMPLE "option for building example" OFF)
    option(BUILD_TEST "option for building test" OFF)
    option(BUILD_SVG_MODULE "option for build svg module" OFF)
    option(BUILD_CODEC_MODULE "option for build codec module" OFF)
    # logging option
    option(ENABLE_LOG "option for logging" OFF)
    option(CPU_BACKEND "option for cpu raster" OFF)
endif()


# backend option
option(ENABLE_HW_RENDER "option for hardware backends" ON)
option(VULKAN_BACKEND "option for vulkan backend" OFF)
option(OPENGL_BACKEND "option for opengl backend" ON)


