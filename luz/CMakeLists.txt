# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

if(IDF_TOOLCHAIN MATCHES "clang")
    # NimBLE requires these warnings to be silenced under clang due to -Werror option
    list(APPEND luz_clang_options "-Wno-error=vla-cxx-extension"
                                  "-Wno-error=missing-braces")
    idf_build_set_property(COMPILE_OPTIONS "${luz_clang_options}" APPEND)
endif()

project(luz)
