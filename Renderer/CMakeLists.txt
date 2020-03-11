file(GLOB_RECURSE source_files *.h *.cpp)
set(ShaderSource 
"forward_ps.hlsl"
"forward_vs.hlsl"
"lightcull_cs.hlsl"
"shader_common.hlsli"
"frame_quad_ps.hlsl"
"frame_quad_vs.hlsl"
)

source_group(Shaders FILES ${ShaderSource})
add_library(Renderer STATIC ${source_files} ${ShaderSource})
target_link_libraries(Renderer Framework D3DCompiler Utility)
cotire(Renderer)