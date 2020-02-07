file(GLOB_RECURSE source_files *.h *.cpp *.hpp)
include_directories(${PROJECT_SOURCE_DIR}/3dparty)

add_library(Utility STATIC ${source_files})
target_link_libraries(Utility libfbxsdk)
set_target_properties(Renderer PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire(Utility)
