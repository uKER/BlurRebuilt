#ifndef DISPLAY_DEVICE_D3D11_EFFECT_NAMES_H
#define DISPLAY_DEVICE_D3D11_EFFECT_NAMES_H

#include <cstring>

namespace Display { namespace D3D11 {

// [Blur1PCDX11][retail-recovered] The shipped vehicle FX resource binding
// carries a 3d suffix; recovered HLSL retains the authored Texture3D name.
// Resolve this exact alias for reflection, resource and sampler lookups.
inline const char* NativeEffectParameterName(const char* name)
{
    return name && std::strcmp(name, "g_sVehicleDamageVolumeTexture3d") == 0
        ? "g_sVehicleDamageVolumeTexture" : name;
}

} }

#endif
