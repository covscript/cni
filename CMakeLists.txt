cmake_minimum_required(VERSION 3.10)

project(covscript-cni)

# Compiler Options
set(CMAKE_CXX_STANDARD 14)

if (MSVC)
    set(CMAKE_CXX_FLAGS "/O2 /EHsc /utf-8 /w")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(rc_flags "/nologo /c65001")
    set(CMAKE_RC_FLAGS ${rc_flags})
elseif (CMAKE_COMPILER_IS_GNUCXX)
    if (WIN32)
        set(CMAKE_CXX_FLAGS "--static -fPIC -s -O3")
    else ()
        set(CMAKE_CXX_FLAGS "-fPIC -s -O3")
    endif ()
else ()
    set(CMAKE_CXX_FLAGS "-fPIC -O3")
endif ()

include_directories(include)

add_library(covscript STATIC ./cni.cpp)

if (UNIX)
    target_link_libraries(covscript pthread dl)
endif ()

add_executable(test-cni ./tests/main.cpp)
add_library(test-cni-lib SHARED ./tests/dll.cpp)

target_link_libraries(test-cni covscript)
target_link_libraries(test-cni-lib covscript)

set_target_properties(test-cni-lib PROPERTIES OUTPUT_NAME test-cni)
set_target_properties(test-cni-lib PROPERTIES PREFIX "")
set_target_properties(test-cni-lib PROPERTIES SUFFIX ".cse")