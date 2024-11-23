 # 指定CMake版本
 cmake_minimum_required(VERSION 3.29)

 set(CMAKE_CXX_STANDARD 20)

 # 指定项目名称 
 project(SolarNow)


 #头文件
 include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/include)

 set(RESOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")
 source_group("Resource Files" FILES ${RESOURCE_FILE})
 # 添加源代码文件
 add_executable(SolarNow src/main.cpp ${RESOURCE_FILE})

 # 添加子目录到构建
 add_subdirectory(SolarGL)
 add_subdirectory(tgaimage)

 #链接
 target_link_libraries(SolarNow SolarGL tga)
