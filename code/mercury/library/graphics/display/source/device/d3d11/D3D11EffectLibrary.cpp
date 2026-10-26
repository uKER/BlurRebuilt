#include "Display/Device/D3D11/D3D11EffectLibrary.h"
#include "Display/Device/D3D11/D3D11EffectNames.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>

namespace Display { namespace D3D11 {

namespace {
std::string EffectKey(const char* name)
{
    if (!name) return {};
    std::string key(NativeEffectParameterName(name));
    const size_t slash = key.find_last_of("/\\");
    if (slash != std::string::npos) key.erase(0, slash + 1);
    const size_t dot = key.find_last_of('.');
    if (dot != std::string::npos) key.erase(dot);
    std::transform(key.begin(), key.end(), key.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return key;
}
}

HRESULT C_D3D11EffectLibrary::Open(const wchar_t* rootPath)
{
    if (!rootPath) return E_INVALIDARG;
    const std::filesystem::path root(rootPath);
    m_Effects.clear();
    std::ifstream index(root / "effects.index");
    if (!index) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    std::string line;
    while (std::getline(index, line))
    {
        const size_t separator = line.find('\t');
        if (separator == std::string::npos) return E_FAIL;
        const std::string key = line.substr(0, separator);
        const std::filesystem::path relative(line.substr(separator + 1));
        if (key.empty() || relative.empty() || relative.is_absolute()) return E_FAIL;
        C_Entry entry;
        entry.path = (root / relative).wstring();
        if (!m_Effects.emplace(key, std::move(entry)).second) return E_FAIL;
    }
    if (index.bad() || m_Effects.empty()) return E_FAIL;
    std::ifstream samplers(root / "effects.samplers");
    if (!samplers) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    while (std::getline(samplers, line))
    {
        const size_t a = line.find('\t');
        const size_t b = a == std::string::npos ? a : line.find('\t', a + 1);
        const size_t c = b == std::string::npos ? b : line.find('\t', b + 1);
        const size_t d = c == std::string::npos ? c : line.find('\t', c + 1);
        if (d == std::string::npos) return E_FAIL;
        const auto effect = m_Effects.find(line.substr(0, a));
        if (effect == m_Effects.end()) return E_FAIL;
        const std::string texture = EffectKey(line.substr(b + 1, c - b - 1).c_str());
        const std::string sampler = EffectKey(line.substr(a + 1, b - a - 1).c_str());
        const std::string value = line.substr(c + 1, d - c - 1);
        if (texture.empty() || (value != "0" && value != "1")) return E_FAIL;
        const bool srgb = value == "1";
        const auto found = effect->second.textureSRGB.find(texture);
        if (found != effect->second.textureSRGB.end() && found->second != srgb)
            return E_FAIL;
        effect->second.textureSRGB[texture] = srgb;
        effect->second.samplerTextures[sampler] = line.substr(b + 1, c - b - 1);
        effect->second.textureNames[texture] = line.substr(b + 1, c - b - 1);
        // [Blur2PC] Preserve HLSL spelling for live FX sampler overrides.
        const std::string samplerName = line.substr(a + 1, b - a - 1);
        effect->second.samplerNames[sampler] = samplerName;
        effect->second.textureSamplers[texture].push_back(samplerName);
    }
    return samplers.bad() ? E_FAIL : S_OK;
}

const char* C_D3D11EffectLibrary::TextureForSampler(const char* gameEffectName,
                                                      const char* samplerName,
                                                      const char* technique) const
{
    const std::string effectKey = EffectKey(gameEffectName);
    auto effect = m_Effects.end();
    if (technique) effect = m_Effects.find(effectKey + "@" + EffectKey(technique));
    if (effect == m_Effects.end()) effect = m_Effects.find(effectKey);
    if (effect == m_Effects.end()) return nullptr;
    const std::string key = EffectKey(samplerName);
    const auto sampler = effect->second.samplerTextures.find(key);
    if (sampler != effect->second.samplerTextures.end()) return sampler->second.c_str();
    // [Blur1PCDX11] Function bindings and shared parameters name the texture (g_tAlbedo), not its sampler.
    const auto texture = effect->second.textureNames.find(key);
    return texture == effect->second.textureNames.end() ? nullptr : texture->second.c_str();
}

HRESULT C_D3D11EffectLibrary::SetTextureSampler(ID3D11Device* device,
                                                const char* gameEffectName,
                                                const char* samplerOrTexture,
                                                ID3D11SamplerState* state,
                                                const char* technique)
{
    if (!samplerOrTexture || !state) return E_INVALIDARG;
    C_D3D11Effect* native = nullptr;
    HRESULT hr = Get(device, gameEffectName, &native, technique);
    if (FAILED(hr)) return hr;
    const std::string effectKey = EffectKey(gameEffectName);
    auto found = m_Effects.end();
    if (technique) found = m_Effects.find(effectKey + "@" + EffectKey(technique));
    if (found == m_Effects.end()) found = m_Effects.find(effectKey);
    const C_Entry& entry = found->second;
    const std::string key = EffectKey(samplerOrTexture);
    const auto sampler = entry.samplerNames.find(key);
    if (sampler != entry.samplerNames.end())
        return native->HasParameter(sampler->second.c_str()) ?
            native->SetSampler(sampler->second.c_str(), state) : S_FALSE;
    const auto texture = entry.textureSamplers.find(key);
    if (texture == entry.textureSamplers.end()) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    // [Blur2PC] Shared/function bindings use texture names. Update each active
    // sampler of that texture, including vertex samplers; unused ones vanish.
    for (const std::string& name : texture->second)
    {
        if (!native->HasParameter(name.c_str())) continue;
        hr = native->SetSampler(name.c_str(), state);
        if (FAILED(hr)) return hr;
    }
    return S_OK;
}

HRESULT C_D3D11EffectLibrary::TextureUsesSRGB(const char* gameEffectName,
                                               const char* textureParameter,
                                               bool* srgb, const char* technique) const
{
    if (!gameEffectName || !textureParameter || !srgb) return E_INVALIDARG;
    *srgb = false;
    const std::string effectKey = EffectKey(gameEffectName);
    auto effect = m_Effects.end();
    if (technique) effect = m_Effects.find(effectKey + "@" + EffectKey(technique));
    if (effect == m_Effects.end()) effect = m_Effects.find(effectKey);
    if (effect == m_Effects.end()) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    const auto texture = effect->second.textureSRGB.find(EffectKey(textureParameter));
    if (texture == effect->second.textureSRGB.end())
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    *srgb = texture->second;
    return S_OK;
}

HRESULT C_D3D11EffectLibrary::Get(ID3D11Device* device, const char* gameEffectName,
                                   C_D3D11Effect** effect, const char* technique)
{
    if (!device || !gameEffectName || !effect) return E_INVALIDARG;
    *effect = nullptr;
    const std::string key = EffectKey(gameEffectName);
    auto found = m_Effects.end();
    if (technique) found = m_Effects.find(key + "@" + EffectKey(technique));
    if (found == m_Effects.end()) found = m_Effects.find(key);
    if (found == m_Effects.end()) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    C_Entry& entry = found->second;
    if (!entry.effect)
    {
        std::unique_ptr<C_D3D11Effect> loaded(new C_D3D11Effect);
        const HRESULT result = loaded->LoadFile(device, entry.path.c_str());
        if (FAILED(result)) return result;
        entry.effect = std::move(loaded);
    }
    *effect = entry.effect.get();
    return S_OK;
}

const void* C_D3D11EffectLibrary::FindEntry(const char* gameEffectName, const char* technique) const
{
    if (!gameEffectName) return nullptr;
    const std::string key = EffectKey(gameEffectName);
    auto found = m_Effects.end();
    if (technique) found = m_Effects.find(key + "@" + EffectKey(technique));
    if (found == m_Effects.end()) found = m_Effects.find(key);
    return found == m_Effects.end() ? nullptr : &found->second;
}

HRESULT C_D3D11EffectLibrary::GetEffect(ID3D11Device* device, const void* entry, C_D3D11Effect** effect)
{
    if (!device || !entry || !effect) return E_INVALIDARG;
    *effect = nullptr;
    C_Entry& found = *const_cast<C_Entry*>(static_cast<const C_Entry*>(entry));
    if (!found.effect)
    {
        std::unique_ptr<C_D3D11Effect> loaded(new C_D3D11Effect);
        const HRESULT result = loaded->LoadFile(device, found.path.c_str());
        if (FAILED(result)) return result;
        found.effect = std::move(loaded);
    }
    *effect = found.effect.get();
    return S_OK;
}

const char* C_D3D11EffectLibrary::EntryTextureForSampler(const void* entry, const char* samplerName) const
{
    if (!entry || !samplerName) return nullptr;
    const C_Entry& found = *static_cast<const C_Entry*>(entry);
    const std::string key = EffectKey(samplerName);
    const auto sampler = found.samplerTextures.find(key);
    if (sampler != found.samplerTextures.end()) return sampler->second.c_str();
    const auto texture = found.textureNames.find(key);
    return texture == found.textureNames.end() ? nullptr : texture->second.c_str();
}

HRESULT C_D3D11EffectLibrary::EntrySamplerNames(const void* entry, const char* samplerOrTexture,
                                                 std::vector<const char*>* names) const
{
    if (!entry || !samplerOrTexture || !names) return E_INVALIDARG;
    names->clear();
    const C_Entry& found = *static_cast<const C_Entry*>(entry);
    const std::string key = EffectKey(samplerOrTexture);
    const auto sampler = found.samplerNames.find(key);
    if (sampler != found.samplerNames.end())
    {
        names->push_back(sampler->second.c_str());
        return S_OK;
    }
    const auto texture = found.textureSamplers.find(key);
    if (texture == found.textureSamplers.end()) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    for (const std::string& name : texture->second) names->push_back(name.c_str());
    return S_OK;
}

HRESULT C_D3D11EffectLibrary::PreloadAll(ID3D11Device* device,
                                          std::string* failedName)
{
    if (!device) return E_INVALIDARG;
    if (failedName) failedName->clear();
    for (auto& item : m_Effects)
    {
        C_D3D11Effect* effect = nullptr;
        const HRESULT result = Get(device, item.first.c_str(), &effect);
        if (FAILED(result))
        {
            if (failedName) *failedName = item.first;
            return result;
        }
    }
    return S_OK;
}

} }
