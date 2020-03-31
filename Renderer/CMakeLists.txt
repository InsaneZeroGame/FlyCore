file(GLOB_RECURSE source_files *.h *.cpp)
set(ShaderSource 
"forward_ps.hlsl"
"forward_vs.hlsl"
"lightcull_cs.hlsl"
"shader_common.hlsli"
"frame_quad_ps.hlsl"
"frame_quad_vs.hlsl"
"shadow_pass_vs.hlsl"
"shadow_pass_ps.hlsl"
"skybox_ps.hlsl"
"skybox_vs.hlsl"
)

set(vs
    forward_vs.hlsl
    frame_quad_vs.hlsl
    shadow_pass_vs.hlsl
    skybox_vs.hlsl
)

set(ps
    forward_ps.hlsl
    frame_quad_ps.hlsl
    shadow_pass_ps.hlsl
    skybox_ps.hlsl
)

set(cs
    lightcull_cs.hlsl
)

set_property(SOURCE ${ps} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${ps} PROPERTY VS_SHADER_TYPE Pixel)
set_property(SOURCE ${ps} PROPERTY VS_SHADER_MODEL 5.0)
set_property(SOURCE ${ps} PROPERTY VS_SHADER_FLAGS "/Zi")

set_property(SOURCE ${vs} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${vs} PROPERTY VS_SHADER_TYPE Vertex)
set_property(SOURCE ${vs} PROPERTY VS_SHADER_MODEL 5.0)
set_property(SOURCE ${vs} PROPERTY VS_SHADER_FLAGS "/Zi")

set_property(SOURCE ${gs} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${gs} PROPERTY VS_SHADER_TYPE Geometry)
set_property(SOURCE ${gs} PROPERTY VS_SHADER_MODEL 5.0)
set_property(SOURCE ${gs} PROPERTY VS_SHADER_FLAGS "/Zi")

set_property(SOURCE ${cs} PROPERTY VS_SHADER_ENTRYPOINT main)
set_property(SOURCE ${cs} PROPERTY VS_SHADER_TYPE Compute)
set_property(SOURCE ${cs} PROPERTY VS_SHADER_MODEL 5.0)
set_property(SOURCE ${cs} PROPERTY VS_SHADER_FLAGS "/Zi")

source_group(Shaders FILES ${ShaderSource})
add_library(Renderer STATIC ${source_files} ${ShaderSource})
target_link_libraries(Renderer Framework D3DCompiler Utility)
cotire(Renderer)