#ifndef BLUR_HDR_RANGE_H
#define BLUR_HDR_RANGE_H

// [Blur2PC][HDR] Keep retail material bounds in SDR; retain positive radiance
// with native HDR. The device binds the same display state in VS and PS b12.
#if defined(BLUR_DX11)
cbuffer BlurHDROutput : register(b12) { float4 g_BlurHDROutput; }
#endif

float4 BlurHDRLighting(float4 value)
{
#if defined(BLUR_DX11)
    if (g_BlurHDROutput.x > 0) return max(value, 0.0);
#endif
    return saturate(value);
}

float3 BlurHDRReflection(float3 value, float ceiling)
{
#if defined(BLUR_DX11)
    if (g_BlurHDROutput.x > 0) return max(value, 0.0);
#endif
    return clamp(value, 0.0, ceiling);
}
#endif
