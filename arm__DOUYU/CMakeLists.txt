
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

set(CMAKE_CXX_COMPILER "/usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++")


# 指定工程的名称
PROJECT(arm_DOUYU)

#指定头文件目录位置
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)


#指定源文件路径
FILE(GLOB SRC_LIST "${PROJECT_SOURCE_DIR}/src/*.cpp")


#生成可执行文件
add_compile_options(-g )
ADD_EXECUTABLE(arm_DOUYU ${SRC_LIST} DOUYU.cpp)
SET(CMAKE_EXE_LINKER_FLAGS "-pthread -g")

#为hello添加共享库链接
#TARGET_LINK_LIBRARIES(hello math)
