 # 指定CMake版本
 cmake_minimum_required(VERSION 3.29)

 set(CMAKE_CXX_STANDARD 17)

 # 指定项目名称 
 project(SolarNow)


 #头文件
 include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/include)

 # 添加源代码文件
 add_executable(main src/main.cpp )

 # 添加子目录到构建
 add_subdirectory(SolarGL)
 add_subdirectory(tgaimage)

 #链接
 target_link_libraries(main SolarGL tga)
