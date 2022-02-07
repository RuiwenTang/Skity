
option(BUILD_EXAMPLE "option for building example" ON)
option(BUILD_TEST "option for building test" ON)
option(BUILD_SVG_MODULE "option for build svg module" ON)
# logging option
option(ENABLE_LOG "option for logging" ON)
# backend option
option(ENABLE_HW_RENDER "option for hardware backends" ON)
option(VULKAN_BACKEND "option for vulkan backend" OFF)
option(OPENGL_BACKEND "option for opengl backend" ON)


if (${ENABLE_LOG})
    set(SKITY_LOG 1)
    # no exception
    add_definitions(-DSPDLOG_NO_EXCEPTIONS)
    include_directories(third_party/spdlog/include)
endif()


if (${VULKAN_BACKEND})
    set(SKITY_VULKAN 1)
    include_directories(third_party/Vulkan-Headers/include)
    include_directories(third_party/VulkanMemoryAllocator/include)
endif()

if (${OPENGL_BACKEND})
    set(SKITY_OPENGL 1)
    include_directories(third_party/OpenGL)
endif()

# svg module
if (${BUILD_SVG_MODULE})
    add_subdirectory(third_party/pugixml)
    add_subdirectory(module/svg)
endif()