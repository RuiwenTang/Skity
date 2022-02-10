
if (${SKITY_VCPKG})
# glm
find_package(glm CONFIG REQUIRED)
# pugixml
find_package(pugixml CONFIG REQUIRED)

if (${ENABLE_LOG})
  find_package(spdlong CONFIG REQUIRED)
  target_link_libraries(skity PRIVATE spdlog::spdlog)
endif()

# vulkan deps
if (${VULKAN_BACKEND})
  find_path(VULKAN_HEADERS_INCLUDE_DIRS "vk_video/vulkan_video_codec_h264std.h")
  target_include_directories(skity ${VULKAN_HEADERS_INCLUDE_DIRS})
endif()

else()
  # glm
  add_subdirectory(third_party/glm)
  # glm headers
  install(DIRECTORY 
    third_party/glm/glm
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} 
    PATTERN "CMakeLists.txt" EXCLUDE)
  # glm config
  install(EXPORT glm 
    FILE glmConfig.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/glm NAMESPACE glm::)

  # pugixml
  add_subdirectory(third_party/pugixml)
  # 
  if (${ENABLE_LOG})
    target_include_directories(skity PRIVATE third_party/spdlog/include)
  endif()

  # Vulkan deps
  if (${VULKAN_BACKEND})
    include_directories(third_party/Vulkan-Headers/include)

  endif()

endif()

# include glm
target_link_libraries(skity PUBLIC glm::glm)


# OpenGL header file
if (${OPENGL_BACKEND})
  target_include_directories(skity PRIVATE third_party/OpenGL)
endif()

# Vulkan memory allocator
if (${VULKAN_BACKEND})
  target_include_directories(skity PRIVATE third_party/VulkanMemoryAllocator/include)
endif()


