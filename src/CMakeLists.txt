cmake_minimum_required(VERSION 3.2)

include_directories(${SoundBoard_SOURCE_DIR}/include)

set(SOURCE_FILES plugin.c hashmap.c)
add_library(soundboard SHARED ${SOURCE_FILES})