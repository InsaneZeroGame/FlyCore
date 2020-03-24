file(GLOB_RECURSE source_files *.h *.cpp)
add_library(Gameplay STATIC ${source_files})
target_link_libraries(Gameplay Framework Utility)
cotire(Gameplay)