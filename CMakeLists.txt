cmake_minimum_required(VERSION 3.10.0)
project(GccLibcPolyPatch VERSION 0.1.0 LANGUAGES C CXX)

add_executable(GccLibcPolyPatch src/main.cpp src/CElfImp.cpp)
target_include_directories(GccLibcPolyPatch 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/ELFIO
)
