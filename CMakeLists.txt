cmake_minimum_required(VERSION 3.12)

project(frida-gum-cmake)

include(ExternalProject)

# Options
option(FRIDA-GUM-CMAKE_BUILD_GUMPP "Enable build of c++ bindings" OFF)
option(FRIDA-GUM-CMAKE_BUILD_GUM "Enable build of c bindings" ON)
option(FRIDA-GUM-CMAKE_BUILD_GUMJS "Enable build of js bindings" OFF)
option(FRIDA-GUM-CMAKE_BUILD_TEST "Enable build test for frida gum" OFF)

# Make configure environment
set(MESON_CONFIGURE_GUM_OPTIONS "")

if(FRIDA-GUM-CMAKE_BUILD_GUMPP)
    set(MESON_CONFIGURE_GUM_OPTIONS "${MESON_CONFIGURE_GUM_OPTIONS} --enable-gumpp")
endif()

if(FRIDA-GUM-CMAKE_BUILD_GUM)
    set(MESON_CONFIGURE_GUM_OPTIONS "${MESON_CONFIGURE_GUM_OPTIONS} --with-devkits=gum")
endif()

if(FRIDA-GUM-CMAKE_BUILD_GUMJS)
    if(FRIDA-GUM-CMAKE_BUILD_GUM)
        set(MESON_CONFIGURE_GUM_OPTIONS "${MESON_CONFIGURE_GUM_OPTIONS},gumjs --enable-gumjs --enable-tests")
    else()
        set(MESON_CONFIGURE_GUM_OPTIONS "${MESON_CONFIGURE_GUM_OPTIONS} --enable-gumjs --with-devkits=gumjs")
    endif()
else()
    set(MESON_CONFIGURE_GUM_OPTIONS "${MESON_CONFIGURE_GUM_OPTIONS} --disable-quickjs --disable-v8 --disable-database --disable-frida-objc-bridge --disable-frida-swift-bridge --disable-frida-java-bridge")
endif()

message(STATUS "MESON_CONFIGURE_GUM_OPTIONS: ${MESON_CONFIGURE_GUM_OPTIONS}")

# Make build environment
set(ARCH_CMAKE_TO_MESON_MAPPING
    "X86:x86"
    "x64:x86_64"
    "arm:arm"
    "arm64:arm64"
    "x86:x86"
    "x86_64:x86_64"
)
set(CMAKE_BUILD_ARCH "")
set(MESON_BUILD_PLATFORM "")
set(MESON_BUILD_HOST "")
set(MESON_BUILD_EXTENTION_ENV_SETUP "echo keep_quiet")

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(CMAKE_BUILD_ARCH ${CMAKE_ANDROID_ARCH})
    set(MESON_BUILD_PLATFORM "android")

    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(MESON_BUILD_EXTENTION_ENV_SETUP "set ANDROID_NDK_ROOT=${CMAKE_ANDROID_NDK}")
    else()
        set(MESON_BUILD_EXTENTION_ENV_SETUP "export ANDROID_NDK_ROOT=${CMAKE_ANDROID_NDK}")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        message(FATAL_ERROR "Not yet compatible compiler ${CMAKE_CXX_COMPILER_ID} on Windows")
    endif()

    set(CMAKE_BUILD_ARCH ${CMAKE_CXX_COMPILER_ARCHITECTURE_ID})
    set(MESON_BUILD_PLATFORM "windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_BUILD_ARCH ${CMAKE_SYSTEM_PROCESSOR})
    set(MESON_BUILD_PLATFORM "linux")
else()
    message(FATAL_ERROR "Not yet compatible platform ${CMAKE_SYSTEM_NAME}")
endif()

foreach(arch_mapping_string ${ARCH_CMAKE_TO_MESON_MAPPING})
    string(REPLACE ":" ";" archs ${arch_mapping_string})

    list(GET archs 0 cmake_arch)
    list(GET archs 1 meson_arch)
    
    if(CMAKE_BUILD_ARCH STREQUAL cmake_arch)
        set(MESON_BUILD_HOST "${MESON_BUILD_PLATFORM}-${meson_arch}")
        break()
    endif()
endforeach()

message(STATUS "CMAKE_BUILD_ARCH: ${CMAKE_BUILD_ARCH}")
message(STATUS "MESON_BUILD_PLATFORM: ${MESON_BUILD_PLATFORM}")
message(STATUS "MESON_BUILD_HOST: ${MESON_BUILD_HOST}")

# Make build tools
set(NATIVE_BASH_TOOL "")
set(MESON_BUILD_ENV_SETUP "")
set(MESON_CONFIGURE_TOOL "")
set(MESON_BUILD_TOOL "")

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(NATIVE_BASH_TOOL "cmd;/C")
    set(MESON_BUILD_ENV_SETUP "chcp 65001")
    set(MESON_CONFIGURE_TOOL "configure.bat")
    set(MESON_BUILD_TOOL "make.bat")
else()
    set(NATIVE_BASH_TOOL "bash;-c")
    set(MESON_BUILD_ENV_SETUP "echo keep_quiet")
    set(MESON_CONFIGURE_TOOL "configure")
    set(MESON_BUILD_TOOL "make")
endif()

# Add external project frida-gum
set(FRIDA-GUM-CMAKE_GUMPP_BINARY_NAME "")
set(FRIDA-GUM-CMAKE_GUM_BINARY_NAME "")
set(FRIDA-GUM-CMAKE_GUMJS_BINARY_NAME "")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(GUM_GUMPP_BINARY_NAME "frida-gumpp-1.0.dll")
    set(GUM_GUM_BINARY_NAME "frida-gum.lib")
    set(GUM_GUMJS_BINARY_NAME "frida-gumjs.lib")
else()
    set(GUM_GUMPP_BINARY_NAME "libfrida-gumpp-1.0.so")
    set(GUM_GUM_BINARY_NAME "libfrida-gum.a")
    set(GUM_GUMJS_BINARY_NAME "libfrida-gumjs.a")
endif()

ExternalProject_Add(
    frida-gum-cmake

    PREFIX ${CMAKE_BINARY_DIR}/frida-gum/${MESON_BUILD_HOST}
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/frida-gum

    GIT_REPOSITORY https://github.com/frida/frida-gum.git
    GIT_TAG 0afeb85fcdeae1d995a55bc07f0fe57b197aecae

    UPDATE_DISCONNECTED ON
    UPDATE_COMMAND
        ${NATIVE_BASH_TOOL} " \
            cd ${CMAKE_CURRENT_SOURCE_DIR}/third_party/frida-gum/releng && \
            git reset --hard 4622f5c4c432d94c1c625e598b120425a68a8414 \
        "

    PATCH_COMMAND
        ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/frida-gum/${MESON_BUILD_HOST}/src/frida-gum-cmake-build COMMAND
        ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/frida-gum/${MESON_BUILD_HOST}/src/frida-gum-cmake-build COMMAND
        ${NATIVE_BASH_TOOL} " \
            cd ${CMAKE_CURRENT_SOURCE_DIR}/third_party/frida-gum/releng && \
            git diff --quiet && (git apply ${CMAKE_CURRENT_SOURCE_DIR}/releng/msvc_chinese.patch && git apply ${CMAKE_CURRENT_SOURCE_DIR}/releng/change_mkdevkit_header_generation_rule.patch) || echo keep_quiet \
        "

    CONFIGURE_COMMAND
        ${NATIVE_BASH_TOOL} " \
            ${MESON_BUILD_ENV_SETUP}&& \
            ${MESON_BUILD_EXTENTION_ENV_SETUP}&& \
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/frida-gum/${MESON_CONFIGURE_TOOL} --host=${MESON_BUILD_HOST} \
                --disable-tests \
                ${MESON_CONFIGURE_GUM_OPTIONS} \
        "

    BUILD_BYPRODUCTS ${CMAKE_BINARY_DIR}/frida-gum/${MESON_BUILD_HOST}/src/frida-gum-cmake-build/gum/devkit/${GUM_GUM_BINARY_NAME} # It's really important!
    BUILD_COMMAND
        ${NATIVE_BASH_TOOL} " \
            ${MESON_BUILD_ENV_SETUP}&& \
            cd ${CMAKE_BINARY_DIR}/frida-gum/${MESON_BUILD_HOST}/src/frida-gum-cmake-build && \
            ${MESON_BUILD_TOOL} \
        "

    INSTALL_COMMAND ""

    LOG_DOWNLOAD ON
    LOG_UPDATE ON
    LOG_PATCH ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
    LOG_TEST ON
    LOG_MERGED_STDOUTERR ON
    LOG_OUTPUT_ON_FAILURE ON
)

# Add exported target frida-gum
if(FRIDA-GUM-CMAKE_BUILD_GUMPP)
    add_library(frida-gumpp SHARED IMPORTED GLOBAL)
    add_dependencies(frida-gumpp frida-gum-cmake)
    set_target_properties(frida-gumpp PROPERTIES
        IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/frida-gum/${MESON_BUILD_HOST}/src/frida-gum-cmake-build/bindings/gumpp/${GUM_GUMPP_BINARY_NAME}
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/third_party/frida-gum
    )
endif()

if(FRIDA-GUM-CMAKE_BUILD_GUM)
    add_library(frida-gum STATIC IMPORTED GLOBAL)
    add_dependencies(frida-gum frida-gum-cmake)
    set_target_properties(frida-gum PROPERTIES
        IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/frida-gum/${MESON_BUILD_HOST}/src/frida-gum-cmake-build/gum/devkit/${GUM_GUM_BINARY_NAME}
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/frida-gum/${MESON_BUILD_HOST}/src/frida-gum-cmake-build
    )
endif()

if(FRIDA-GUM-CMAKE_BUILD_GUMJS)
    add_library(frida-gumjs STATIC IMPORTED GLOBAL)
    add_dependencies(frida-gumjs frida-gum-cmake)
    set_target_properties(frida-gumjs PROPERTIES
        IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/frida-gum/${MESON_BUILD_HOST}/src/frida-gum-cmake-build/bindings/gumjs/devkit/${GUM_GUMJS_BINARY_NAME}
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/frida-gum/${MESON_BUILD_HOST}/src/frida-gum-cmake-build
    )
endif()

if(FRIDA-GUM-CMAKE_BUILD_TEST AND FRIDA-GUM-CMAKE_BUILD_GUM)
    add_executable(frida-gum-cmake-test src/main.cc)
    target_link_libraries(frida-gum-cmake-test frida-gum)
endif()
