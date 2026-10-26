#ifndef DISPLAY_DEVICE_D3D11_EFFECT_LIBRARY_H
#define DISPLAY_DEVICE_D3D11_EFFECT_LIBRARY_H

// [Blur1PCDX11] Resolves the game's Jam effect names to native FX5 binaries.
#include "Display/Device/D3D11/D3D11Effect.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Display { namespace D3D11 {

class C_D3D11EffectLibrary
{
public:
    HRESULT Open(const wchar_t* root);
    HRESULT Get(ID3D11Device* device, const char* gameEffectName,
                C_D3D11Effect** effect, const char* technique = nullptr);
    HRESULT TextureUsesSRGB(const char* gameEffectName, const char* textureParameter,
                            bool* srgb, const char* technique = nullptr) const;
    const char* TextureForSampler(const char* gameEffectName, const char* samplerName,
                                 const char* technique = nullptr) const;
    HRESULT SetTextureSampler(ID3D11Device* device, const char* gameEffectName,
                              const char* samplerOrTexture, ID3D11SamplerState* state,
                              const char* technique = nullptr);
    HRESULT PreloadAll(ID3D11Device* device, std::string* failedName = nullptr);
    size_t EffectCount() const { return m_Effects.size(); }

    // [Blur1PCDX11] Entry-level forms of the lookups above. An entry handle stays
    // valid until Open; callers resolve names once and keep the reflected interfaces.
    const void* FindEntry(const char* gameEffectName, const char* technique = nullptr) const;
    HRESULT GetEffect(ID3D11Device* device, const void* entry, C_D3D11Effect** effect);
    const char* EntryTextureForSampler(const void* entry, const char* samplerName) const;
    // HLSL samplers that SetTextureSampler updates for this name, before the
    // HasParameter filter; ERROR_NOT_FOUND when the name is neither form.
    HRESULT EntrySamplerNames(const void* entry, const char* samplerOrTexture,
                              std::vector<const char*>* names) const;

private:
    struct C_Entry
    {
        std::wstring path;
        std::unique_ptr<C_D3D11Effect> effect;
        std::unordered_map<std::string, bool> textureSRGB;
        std::unordered_map<std::string, std::string> samplerTextures;
        std::unordered_map<std::string, std::string> textureNames;
        std::unordered_map<std::string, std::string> samplerNames;
        std::unordered_map<std::string, std::vector<std::string>> textureSamplers;
    };
    std::unordered_map<std::string, C_Entry> m_Effects;
};

} }

#endif
