 # 指定CMake版本
 cmake_minimum_required(VERSION 3.29)

 # 指定项目名称 
 project(SolarNow)

 #库目录
 link_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/lib)
 #头文件
 include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/include)

 # 添加源代码文件
 add_executable(main src/main.cpp)

 target_link_libraries(main SolarGL)
