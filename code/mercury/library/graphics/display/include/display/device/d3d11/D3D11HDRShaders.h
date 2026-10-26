// [Blur2PC][HDR] Shared by the native presenter and its GPU reference checks.
#pragma once
namespace Display { namespace D3D11 {
static const char* const HDRPresentShader = R"hlsl(
cbuffer BlurHDROutput : register(b12) { float4 hdr; }
Texture2D<float4> frame : register(t0);
float4 VS(uint id : SV_VertexID) : SV_Position {
    return float4(id == 2 ? 3 : -1, id == 1 ? 3 : -1, 0, 1);
}
float3 DecodeSRGB(float3 value) {
    value = max(value, 0);
    return float3(value.r <= 0.04045 ? value.r / 12.92 : pow((value.r + 0.055) / 1.055, 2.4),
                  value.g <= 0.04045 ? value.g / 12.92 : pow((value.g + 0.055) / 1.055, 2.4),
                  value.b <= 0.04045 ? value.b / 12.92 : pow((value.b + 0.055) / 1.055, 2.4));
}
float4 PS(float4 position : SV_Position) : SV_Target {
    float3 colour = DecodeSRGB(frame.Load(int3(position.xy, 0)).rgb);
    if (hdr.x > 0) {
        colour *= hdr.w;
        float intensity = max(colour.r, max(colour.g, colour.b));
        // Identity through paper white, continuous slope at the shoulder;
        // peak is calibrated per display, never a fixed HDR output ceiling.
        float headroom = max(hdr.z - 1, 0);
        if (intensity > 1) {
            float mapped = headroom > 0 ? 1 + headroom * (1 - exp(-(intensity - 1) / headroom)) : 1;
            colour *= mapped / intensity;
        }
    } else colour = saturate(colour);
    // scRGB 1.0 is the Windows 80-nit unit, NOT a maximum brightness.
    return float4(colour * hdr.y, 1);
}
)hlsl";
}}
