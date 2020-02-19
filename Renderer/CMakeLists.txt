file(GLOB_RECURSE source_files *.h *.cpp)
set(ShaderSource 
"forward_ps.hlsl"
"forward_vs.hlsl"
"lightcull_cs.hlsl"
)
source_group(Shaders FILES ${ShaderSource})
add_library(Renderer STATIC ${source_files} ${ShaderSource})
target_link_libraries(Renderer Framework D3DCompiler Utility)
set_target_properties(Renderer PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire(Renderer)