cmake_minimum_required(VERSION 3.10)
project(SHSampler)
set(LIBRARY_NAME "SHSampler")

set(SOURCE_TOP_DIR ${PROJECT_SOURCE_DIR}/../)

include_directories(${SOURCE_TOP_DIR}/ThirdParty/opencv/include)


set(SOURCES
	main.cpp
	harmonics.cpp
	harmonics.h
	cubemap.cpp
	cubemap.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/../ThirdParty/opencv/x64/vc15/lib/opencv_world451d.lib)