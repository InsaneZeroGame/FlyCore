cmake_minimum_required (VERSION 3.12.0)
project (FlyingCore CXX)
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
include(cotire)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
#3dparty
include_directories(3dparty/include)
link_directories(3dparty/lib)


add_subdirectory(Game)
add_subdirectory(Renderer)
add_subdirectory(Framework)
add_subdirectory(Utility)