cmake_minimum_required(VERSION 4.2)
project(Box3dTest)

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
set(RAYLIB_VERSION 6.0)
set(BUILD_DEBUG ON)
set(CUSTOMIZE_BUILD OFF CACHE BOOL "Use raylib's default config.h settings" FORCE)
FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable(raylib)
target_compile_definitions(raylib PRIVATE SUPPORT_FILEFORMAT_HDR=1 SUPPORT_FILEFORMAT_JPG=1)

FetchContent_Declare(box3d
    GIT_REPOSITORY https://github.com/erincatto/box3d.git
    GIT_TAG v0.1.0
)

FetchContent_MakeAvailable(box3d)


FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG docking
)

FetchContent_MakeAvailable(imgui)

# Create your imgui library target
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    # Add your chosen backend files here, e.g. imgui_impl_opengl3.cpp, imgui_impl_glfw.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG Raylib_6_0
)

FetchContent_MakeAvailable(rlimgui)

add_library(rlImGui STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)



target_include_directories(rlImGui PUBLIC
    ${rlimgui_SOURCE_DIR}
    ${raylib_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
)
target_link_libraries(rlImGui PUBLIC raylib)
# Link your dependencies




add_library(Box3dEngine STATIC)
add_executable(Box3dGame)
add_executable(Box3dEditor)
add_subdirectory(src)

target_link_libraries(Box3dEngine PUBLIC raylib box3d::box3d)
target_link_libraries(Box3dGame PRIVATE Box3dEngine)
target_link_libraries(Box3dEditor PRIVATE Box3dEngine imgui rlImGui)

function(copy_resources TARGET_NAME)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:${TARGET_NAME}>/resources
    )
endfunction()

copy_resources(Box3dGame)
copy_resources(Box3dEditor)
