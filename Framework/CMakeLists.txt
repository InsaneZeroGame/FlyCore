file(GLOB_RECURSE source_files *.h *.cpp)
add_library(Framework STATIC ${source_files})
target_link_libraries(Framework glfw3)
cotire(Framework)
