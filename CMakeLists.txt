 # 指定CMake版本
 cmake_minimum_required(VERSION 3.29)

 # 指定项目名称 
 project(render)

 include_directories(
         SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/include
 )

 # 添加源代码文件
 add_executable(main "src/tgaimage.cpp" "src/model.cpp" "src/geometry.cpp" "src/main.cpp")

