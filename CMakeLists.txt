cmake_minimum_required(VERSION 4.1)

project(ArxConverter)


set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)


add_subdirectory("${CMAKE_SOURCE_DIR}/Libs/fmt")


file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Src/*.cpp")

file(GLOB_RECURSE IXX_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Src/*.ixx")

file(GLOB_RECURSE C_FILES   CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Src/*.c")


add_executable(${PROJECT_NAME} ${CPP_FILES} ${C_FILES})

target_sources(${PROJECT_NAME} PRIVATE FILE_SET CXX_MODULES TYPE CXX_MODULES FILES ${IXX_FILES})


target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/Include")