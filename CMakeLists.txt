cmake_minimum_required(VERSION 3.10)
project(AudioConverter CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(audio_converter main.cpp AudioConverter.cpp)
if(MSVC)
  target_compile_options(audio_converter PRIVATE /W4)
else()
  target_compile_options(audio_converter PRIVATE -Wall -Wextra)
endif()
