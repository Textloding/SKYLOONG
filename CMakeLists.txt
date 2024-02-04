# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

idf_build_set_property(COMPILE_OPTIONS "-Wno-error=stringop-truncation" APPEND)

project(GK87-Screen)

littlefs_create_partition_image(spiffs data)
