cmake_minimum_required(VERSION 3.8.0)
project(threads_alhorihtms VERSION 0.1.0)

# set (CMAKE_C_COMPILER_VERSION gcc-9)
# set (CMAKE_CXX_COMPILER_VERSION g++-9)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

find_package(Threads)


if (NOT Threads_FOUND)
    message(FATAL "Can't find Threads library")
endif()


add_executable(LFQueue LockFreeQueue.cpp)
add_executable(FCQueue FlatCombining.cpp)

set_property(TARGET LFQueue PROPERTY CXX_STANDARD 14)
set_property(TARGET FCQueue PROPERTY CXX_STANDARD 14)


target_link_libraries(LFQueue ${CMAKE_THREAD_LIBS_INIT})