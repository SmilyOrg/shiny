cmake_minimum_required(VERSION 3.0)

project(shiny)

find_package(CUDA REQUIRED)

include(FindCUDA)

#list(APPEND CMAKE_CXX_FLAGS "-O0 -g -ffast-math -Wall -Zi /openmp")
#set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_22,code=sm_22)

set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};
	-O0
	-g -Xcompiler -Zi
	-Xcompiler /openmp
)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

include_directories(
	include/
	src/vendor/
	src/proto/
)

add_definitions(
	-DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_SYSTEM_OMP
	-DUSE_WEBSOCKET
	-D_CRT_NO_TIME_T
)

file(GLOB SOURCES
	src/*.cpp
	src/*.hpp
	
	src/proto/*.pb.cc
	
	src/vendor/ujson/ujson.cpp
	src/vendor/ujson/double-conversion.cc
	
	src/vendor/civetweb/civetweb.c
)

cuda_add_executable(shiny ${SOURCES})

link_directories(
	lib
)

target_link_libraries(
	shiny
	../lib/libprotoc
	../lib/libprotobuf
)