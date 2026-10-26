#ifndef GAME_VEHICLE_LIGHT_RGBM_H
#define GAME_VEHICLE_LIGHT_RGBM_H

namespace Game
{
// [Blur2PC][HDR] Retail RGBM normalization with an extended float multiplier.
// Sampling remains rgb * alpha * 8; no light gain or authored falloff changes.
inline void EncodeExtendedVehicleLightRGBM(float r, float g, float b, float (&rgba)[4])
{
    float multiplier = r > g ? r : g;
    multiplier = multiplier > b ? multiplier : b;
    multiplier = multiplier > 0.25f ? multiplier : 0.25f;
    rgba[0] = r / multiplier;
    rgba[1] = g / multiplier;
    rgba[2] = b / multiplier;
    rgba[3] = multiplier / 8.0f;
}
}
#endif
