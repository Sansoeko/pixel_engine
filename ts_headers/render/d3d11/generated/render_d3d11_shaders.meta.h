read_only global String8 r_d3d11_g_rect_shader_src =
Str8LitComp(""
"\n"
"//- rjf: globals\n"
"\n"
"cbuffer Globals : register(b0)\n"
"{\n"
"    float2 viewport_size;\n"
"    float sdf_mask_boldness;\n"
"    float sdf_mask_softness;\n"
"    row_major float4x4 albedo_sample_channel_map;\n"
"    float2 albedo_t2d_size;\n"
"}\n"
"\n"
"//- rjf: input/output attributes\n"
"\n"
"struct VertexShaderInputInstanced\n"
"{\n"
"    float4 p     : POS;\n"
"    float4 t     : TEX;\n"
"    float4 color00 : COL0;\n"
"    float4 color01 : COL1;\n"
"    float4 color10 : COL2;\n"
"    float4 color11 : COL3;\n"
"    float corner_radius : CRAD;\n"
"    float border_thickness : BTHC;\n"
"    uint vertex_id : SV_VertexID;\n"
"};\n"
"\n"
"struct VertexShaderOutput\n"
"{\n"
"    float4 position : SV_POSITION;\n"
"    float2 rect_pos : POS;\n"
"    float2 rect_size: PSIZE;\n"
"    float2 texcoord : TEX;\n"
"    float4 color    : COL;\n"
"    float corner_radius : CRAD;\n"
"    float border_thickness : BTHC;\n"
"};\n"
"\n"
"//- rjf: resources\n"
"\n"
"Texture2D albedo_t2d         : register(t0);\n"
"Texture2D sdf_mask_t2d       : register(t1);\n"
"SamplerState default_sampler : register(s0);\n"
"\n"
"//- rjf: helpers\n"
"\n"
"float RectSDF(float2 sample_pos, float2 rect_pos, float2 rect_size, float r)\n"
"{\n"
"    float2 rect_center = rect_pos + rect_size/2;\n"
"    float2 d2 = abs(sample_pos - rect_center) - rect_size/2 + float2(r, r);\n"
"    return min(max(d2.x, d2.y), 0.0) + length(max(d2, 0.0)) - r;   \n"
"}\n"
"\n"
"//- rjf: shaders\n"
"\n"
"VertexShaderOutput\n"
"vs_main(VertexShaderInputInstanced input)\n"
"{\n"
"    float2 p0 = float2(input.p.x, input.p.y);\n"
"    float2 p1 = float2(input.p.z, input.p.w);\n"
"    float2 p_size = abs(p1 - p0);\n"
"    float2 t0 = float2(input.t.x, input.t.y);\n"
"    float2 t1 = float2(input.t.z, input.t.w);\n"
"    float4 c00 = input.color00;\n"
"    float4 c01 = input.color01;\n"
"    float4 c10 = input.color10;\n"
"    float4 c11 = input.color11;\n"
"    \n"
"    float2 p_verts[] =\n"
"    {\n"
"        float2(p0.x, viewport_size.y - p1.y),\n"
"        float2(p0.x, viewport_size.y - p0.y),\n"
"        float2(p1.x, viewport_size.y - p1.y),\n"
"        float2(p1.x, viewport_size.y - p0.y),\n"
"    };\n"
"    \n"
"    float2 t_verts[] =\n"
"    {\n"
"        float2(t0.x/albedo_t2d_size.x, t1.y/albedo_t2d_size.y),\n"
"        float2(t0.x/albedo_t2d_size.x, t0.y/albedo_t2d_size.y),\n"
"        float2(t1.x/albedo_t2d_size.x, t1.y/albedo_t2d_size.y),\n"
"        float2(t1.x/albedo_t2d_size.x, t0.y/albedo_t2d_size.y),\n"
"    };\n"
"    \n"
"    float4 colors[] =\n"
"    {\n"
"        c01,\n"
"        c00,\n"
"        c11,\n"
"        c10,\n"
"    };\n"
"    \n"
"    VertexShaderOutput output;\n"
"    output.position  = float4(2 * p_verts[input.vertex_id].x / viewport_size.x - 1.f,\n"
"                              2 * p_verts[input.vertex_id].y / viewport_size.y - 1.f,\n"
"                              0.f,\n"
"                              1.f);\n"
"    output.rect_pos  = p0;\n"
"    output.rect_size = p_size;\n"
"    output.texcoord  = t_verts[input.vertex_id];\n"
"    output.color     = colors[input.vertex_id];\n"
"    output.corner_radius = input.corner_radius;\n"
"    output.border_thickness = input.border_thickness;\n"
"    return output;\n"
"}\n"
"\n"
"float4\n"
"ps_main(VertexShaderOutput input) : SV_TARGET\n"
"{\n"
"    // rjf: get colors\n"
"    float4 tint = input.color;\n"
"    float4 albedo_sample   = mul(albedo_t2d.Sample(default_sampler, input.texcoord), albedo_sample_channel_map);\n"
"    float4 sdf_mask_sample = mul(sdf_mask_t2d.Sample(default_sampler, input.texcoord), albedo_sample_channel_map);\n"
"    \n"
"    // rjf: derive things from colors\n"
"    float boldness = 0.5;\n"
"    float softness = 0.3;\n"
"    float sdf_mask_factor = smoothstep(1-boldness, 1-boldness + softness, sdf_mask_sample.x);\n"
"    \n"
"    // rjf: apply rect corner radius\n"
"    float corner_radius_d = RectSDF(input.position.xy, input.rect_pos, input.rect_size, input.corner_radius);\n"
"    float corner_radius_f = saturate(corner_radius_d);\n"
"    \n"
"    // rjf: apply border thickness\n"
"    float border_thickness_d = RectSDF(input.position.xy,\n"
"                                       input.rect_pos+input.border_thickness,\n"
"                                       input.rect_size-input.border_thickness*2,\n"
"                                       max(input.corner_radius-input.border_thickness, 0));\n"
"    float border_thickness_f = saturate(border_thickness_d);\n"
"    if(input.border_thickness == 0)\n"
"    {\n"
"        border_thickness_f = 1;\n"
"    }\n"
"    \n"
"    // rjf: form+return final color\n"
"    float4 final_color = albedo_sample;\n"
"    final_color *= sdf_mask_factor;\n"
"    final_color *= 1-corner_radius_f;\n"
"    final_color *= border_thickness_f;\n"
"    final_color *= tint;\n"
"    return final_color;\n"
"}\n"
""
);

read_only global String8 r_d3d11_g_triangle_shader_src =
Str8LitComp(""
"\n"
"//- rjf: globals\n"
"\n"
"cbuffer Globals : register(b0)\n"
"{\n"
"    float2 viewport_size;\n"
"}\n"
"\n"
"//- rjf: input/output attributes\n"
"\n"
"struct VertexShaderInputInstanced\n"
"{\n"
"    float2 p0      : POS0;\n"
"    float2 p1      : POS1;\n"
"    float2 p2      : POS2;\n"
"    float4 color0  : COL0;\n"
"    float4 color1  : COL1;\n"
"    float4 color2  : COL2;\n"
"    uint vertex_id : SV_VertexID;\n"
"};\n"
"\n"
"struct VertexShaderOutput\n"
"{\n"
"    float4 p     : POS;\n"
"    float4 color : COL;\n"
"};\n"
"\n"
"//- rjf: shaders\n"
"\n"
"VertexShaderOutput\n"
"vs_main(VertexShaderInputInstanced input)\n"
"{\n"
"    float3 p0 = input.p0;\n"
"    float3 p1 = input.p1;\n"
"    float3 p2 = input.p2;\n"
"    float4 c0 = input.color0;\n"
"    float4 c1 = input.color1;\n"
"    float4 c2 = input.color2;\n"
"    float3 p[] = { p0, p1, p2 };\n"
"    float4 c[] = { c0, c1, c2 };\n"
"    \n"
"    VertexShaderOutput output;\n"
"    output.position  = float4(2 * p[input.vertex_id].x / viewport_size.x - 1.f,\n"
"                              2 * p[input.vertex_id].y / viewport_size.y - 1.f,\n"
"                              0.f,\n"
"                              1.f);\n"
"    output.color     = colors[input.vertex_id];\n"
"    return output;\n"
"}\n"
"\n"
"float4\n"
"ps_main(VertexShaderOutput input) : SV_TARGET\n"
"{\n"
"    float4 tint = input.color;\n"
"    return tint;\n"
"}\n"
""
);

