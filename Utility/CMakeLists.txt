file(GLOB_RECURSE source_files *.h *.cpp *.hpp)
include_directories(${PROJECT_SOURCE_DIR}/3dparty)

add_library(Utility STATIC ${source_files})
target_link_libraries(Utility libfbxsdk)
cotire(Utility)
