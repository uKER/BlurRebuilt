// [Blur1PCDX11] Use the game's authored FX5 permutations as native D3D11 effects.
#include "stdafx.h"
#include "fx/NativeFX11.h"
#include "fx/effect.h"
#include "fx/Shaders.h"
#include "fx/Shared.h"
#include "fx/SamplerStates.h"
#include "Display/Device/Device.h"
#include "Display/Device/D3D11/D3D11EffectLibrary.h"
#include "Display/Device/D3D11/D3D11Formats.h"
#include "Display/Device/D3D11/D3D11States.h"
#include <system/debug/Blur2PCAutomation.h>
#include <limits>
#include <memory>
#include <unordered_map>
#include <vector>
#include <wrl/client.h>

namespace FX {

const void* GetDefaultVertexTextureFor(const void* texture); // System.cpp, PC FX mechanism

static char g_fx11LastError[256] = {};

const char* NativeFX11EffectName(const Effect* effect)
{
    if (!effect) return "";
    const char* filename = effect->NativeFilename.Chars();
    return filename && *filename ? filename : effect->Name.Chars();
}

namespace {
Display::D3D11::C_D3D11EffectLibrary g_effects;
bool g_opened = false;
unsigned g_failures = 0;

// [Blur2PC] Cache immutable descriptors, not IDs: SamplerStateManager::Clear
// may reuse an ID for another state. These objects share the FX library device.
struct C_SamplerDescHash
{
    size_t operator()(const D3D11_SAMPLER_DESC& desc) const
    {
        unsigned hash = 2166136261u;
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&desc);
        for (size_t i = 0; i < sizeof(desc); ++i) hash = (hash ^ bytes[i]) * 16777619u;
        return hash;
    }
};
struct C_SamplerDescEqual
{
    bool operator()(const D3D11_SAMPLER_DESC& a, const D3D11_SAMPLER_DESC& b) const
    { return memcmp(&a, &b, sizeof(a)) == 0; }
};
std::unordered_map<D3D11_SAMPLER_DESC, Microsoft::WRL::ComPtr<ID3D11SamplerState>,
                   C_SamplerDescHash, C_SamplerDescEqual> g_samplers;

// [Blur1PCDX11] What each native texture variable holds, when that is an owned
// texture's view (D3D11Formats): binding the same texture and gamma again while
// no owned texture has been released leaves the variable as it is.
struct S_BoundView
{
    ID3D11Resource* texture = nullptr;
    bool srgb = false;
    unsigned generation = 0;
};
std::unordered_map<ID3DX11EffectShaderResourceVariable*, S_BoundView> g_boundViews;
// The sampler state each native sampler variable was last given by BindTexture.
std::unordered_map<ID3DX11EffectSamplerVariable*, ID3D11SamplerState*> g_boundSamplers;

// [Blur1PCDX11] Native bindings of one game parameter name in one technique,
// resolved once. FX names are interned SharedStrings, so the name pointer is
// the key. The reflected interfaces belong to the library's loaded effect,
// which lives as long as the library.
struct S_Param
{
    bool rawResolved = false;
    ID3DX11EffectVariable* raw = nullptr;      // HasParameter/SetRawParameter target
    UINT rawBytes = 0;                         // reflected UnpackedSize
    bool textureResolved = false;
    const char* texture = nullptr;             // HLSL texture for this sampler/texture name
    ID3DX11EffectShaderResourceVariable* view = nullptr;
    HRESULT viewResult = S_OK;                 // SetTexture's result when view is null
    bool cube = false;
    bool damageVolume = false;
    std::vector<ID3DX11EffectSamplerVariable*> samplers;
    std::vector<ID3D11SamplerState**> samplersBound; // g_boundSamplers entries of samplers
    S_BoundView* viewBound = nullptr;          // g_boundViews entry of view
    HRESULT samplerResult = S_OK;              // SetTextureSampler's lookup result
    // [Blur1PCDX11] The sampler state last bound here: its manager id and live
    // values when bound, and the state object they produced.
    int liveId = -1;
    unsigned char live[sizeof(Display::C_SamplerState)] = {};
    ID3D11SamplerState* liveSampler = nullptr;
};


struct S_Shared
{
    int index;
    size_t offset;
    S_Param* param;
    bool matrix;
};

// [Blur1PCDX11] Function metadata is immutable after effect loading. Resolve
// its parameter interfaces once, but read the live source bytes on every bind.
// unordered_map rehashing preserves the S_Param node addresses.
struct S_FunctionBinding
{
    const FunctionBinding* binding;
    const char* name;
    S_Param* param;
    bool matrix;
};
struct S_FunctionBindings
{
    const FunctionBinding* base = nullptr;
    int count = -1;
    std::vector<S_FunctionBinding> entries;
};
struct S_PassBindings
{
    S_FunctionBindings vertex, pixel;
};

struct S_Technique
{
    const char* name = nullptr;
    const void* entry = nullptr;
    Display::D3D11::C_D3D11Effect* native = nullptr;
    std::vector<ID3DX11EffectPass*> passes;
    std::vector<bool> passesResolved;
    std::vector<S_PassBindings> bindings;
    std::unordered_map<const char*, S_Param> params;
    // Shared parameters the native effect declares, rebuilt if the shared effect changes.
    std::vector<S_Shared> shared;
    int sharedCount = -1;
    const void* sharedBase = nullptr;
};

struct S_EffectCache
{
    std::vector<std::unique_ptr<S_Technique>> techniques;
};
std::unordered_map<const void*, std::unique_ptr<S_EffectCache>> g_caches;
const void* g_lastEffect = nullptr;
S_EffectCache* g_lastCache = nullptr;

// [Blur1PCDX11] Deferred pass Apply, see Display::D3D11::FlushNativeFX.
ID3DX11EffectPass* g_pendingPass = nullptr;
// Names of the pass last queued, formatted only when a diagnostic asks.
const char* g_currentEffect = nullptr;
const char* g_currentTechnique = nullptr;
const char* g_currentFile = nullptr;
int g_currentIndex = 0;

ID3D11DeviceContext* ImmediateContext()
{
    static ID3D11Device* device = nullptr;
    static ID3D11DeviceContext* context = nullptr;
    ID3D11Device* current = Display::Device() ? Display::Device()->GetD3DDevice() : nullptr;
    if (current != device)
    {
        device = current;
        context = nullptr;
        if (device)
        {
            device->GetImmediateContext(&context);
            if (context) context->Release(); // owned by the device
        }
    }
    return context;
}

bool FlushPendingPass()
{
    ID3DX11EffectPass* pass = g_pendingPass;
    if (!pass) return false;
    g_pendingPass = nullptr;
    ID3D11DeviceContext* context = ImmediateContext();
    if (!context) return false;
    if (FAILED(pass->Apply(0, context)))
        sprintf_s(g_fx11LastError, "%.80s / %.80s pass=%d", g_currentEffect ? g_currentEffect : "",
            g_currentTechnique ? g_currentTechnique : "", g_currentIndex);
    return true;
}

void OpenLibrary()
{
    if (g_opened) return;
    g_opened = true;
    wchar_t module[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, module, MAX_PATH))
    {
        wchar_t* slash = wcsrchr(module, L'\\');
        if (slash) *(slash + 1) = 0;
        wcscat_s(module, L"fx11");
        const HRESULT hr = g_effects.Open(module);
        Blur2PCAutomation::LogF("DX11 FX library: %u effects, status %08x",
            static_cast<unsigned>(g_effects.EffectCount()), static_cast<unsigned>(hr));
    }
    Display::D3D11::g_FlushNativeFX = &FlushPendingPass;
}

S_Technique* Find(const Effect* effect, int technique)
{
    if (!effect || !Display::Device() || !Display::Device()->GetD3DDevice()) return nullptr;
    if (technique < 0 || technique >= effect->Techniques.Size()) return nullptr;
    OpenLibrary();
    S_EffectCache* cache = g_lastEffect == effect ? g_lastCache : nullptr;
    if (!cache)
    {
        auto& slot = g_caches[effect];
        if (!slot) slot.reset(new S_EffectCache);
        cache = slot.get();
        g_lastEffect = effect;
        g_lastCache = cache;
    }
    if (cache->techniques.size() != static_cast<size_t>(effect->Techniques.Size()))
    {
        cache->techniques.clear();
        cache->techniques.resize(effect->Techniques.Size());
    }
    const char* name = effect->Techniques[technique].Name.Chars();
    std::unique_ptr<S_Technique>& found = cache->techniques[technique];
    if (!found || found->name != name)
    {
        found.reset(new S_Technique);
        found->name = name;
        found->entry = g_effects.FindEntry(NativeFX11EffectName(effect), name);
        const HRESULT hr = found->entry
            ? g_effects.GetEffect(Display::Device()->GetD3DDevice(), found->entry, &found->native)
            : HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        if (FAILED(hr) && g_failures++ < 20)
            Blur2PCAutomation::LogF("DX11 FX missing %s: %08x", effect->Name.Chars(), static_cast<unsigned>(hr));
        const int passes = effect->Techniques[technique].Passes.Size();
        found->passes.assign(passes, nullptr);
        found->passesResolved.assign(passes, false);
        found->bindings.resize(passes);
    }
    return found->native ? found.get() : nullptr;
}

int PassIndex(const Effect* effect, const Pass* pass, int technique)
{
    if (!effect || !pass || technique < 0 || technique >= effect->Techniques.Size()) return -1;
    const auto& passes = effect->Techniques[technique].Passes;
    for (int i = 0; i < passes.Size(); ++i)
        if (&passes[i] == pass) return i;
    return -1;
}

ID3DX11EffectPass* FindPass(S_Technique& technique, int index)
{
    if (index < 0 || index >= static_cast<int>(technique.passes.size())) return nullptr;
    if (!technique.passesResolved[index])
    {
        technique.passesResolved[index] = true;
        technique.passes[index] = technique.native->Pass(technique.name, index);
    }
    return technique.passes[index];
}

S_Param& RawParam(S_Technique& technique, const char* name)
{
    S_Param& param = technique.params[name];
    if (!param.rawResolved)
    {
        param.rawResolved = true;
        ID3DX11EffectVariable* variable = technique.native->Variable(name);
        D3DX11_EFFECT_TYPE_DESC type = {};
        if (variable && variable->IsValid() && SUCCEEDED(variable->GetType()->GetDesc(&type)))
        {
            param.raw = variable;
            param.rawBytes = type.UnpackedSize;
        }
    }
    return param;
}

S_Param& TextureParam(S_Technique& technique, const char* sampler)
{
    S_Param& param = technique.params[sampler];
    if (param.textureResolved) return param;
    param.textureResolved = true;
    param.texture = g_effects.EntryTextureForSampler(technique.entry, sampler);
    if (!param.texture) return param;
    param.damageVolume = strcmp(param.texture, "g_sVehicleDamageVolumeTexture") == 0;
    ID3DX11EffectVariable* variable = technique.native->Variable(param.texture);
    if (!variable || !variable->IsValid()) param.viewResult = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    else
    {
        ID3DX11EffectShaderResourceVariable* resource = variable->AsShaderResource();
        if (!resource || !resource->IsValid()) param.viewResult = E_INVALIDARG;
        else
        {
            param.view = resource;
            param.viewBound = &g_boundViews[resource];
            D3DX11_EFFECT_TYPE_DESC type = {};
            param.cube = SUCCEEDED(variable->GetType()->GetDesc(&type)) && type.Type == D3D_SVT_TEXTURECUBE;
        }
    }
    // [Blur2PC] Shared/function bindings use texture names. Update each active
    // sampler of that texture, including vertex samplers; unused ones vanish.
    std::vector<const char*> names;
    param.samplerResult = g_effects.EntrySamplerNames(technique.entry, sampler, &names);
    for (const char* name : names)
    {
        ID3DX11EffectVariable* samplerVariable = technique.native->Variable(name);
        if (!samplerVariable || !samplerVariable->IsValid()) continue;
        ID3DX11EffectSamplerVariable* state = samplerVariable->AsSampler();
        if (!state || !state->IsValid()) { param.samplerResult = E_INVALIDARG; break; }
        param.samplers.push_back(state);
        param.samplersBound.push_back(&g_boundSamplers[state]);
    }
    return param;
}

HRESULT SetRawParameter(S_Technique& technique, const char* name, const void* data, UINT bytes)
{
    if (!name || !data || !bytes) return E_INVALIDARG;
    const S_Param& param = RawParam(technique, name);
    if (!param.raw) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    return param.raw->SetRawValue(data, 0, bytes < param.rawBytes ? bytes : param.rawBytes);
}

void BindTexture(const Effect* effect, S_Technique& technique,
                 const char* sampler, ID3D11Resource* texture, const char* techniqueName,
                 int samplerState, bool immediate = false, S_Param* resolved = nullptr)
{
    // [Blur1PCDX11] FX sampler values are GPUTexture pointers (Display2: ID3D11Resource), as D3D9's were IDirect3DBaseTexture9.
    const S_Param& param = resolved ? *resolved : TextureParam(technique, sampler);
    const char* name = param.texture;
    // [Blur1PCDX11] Diagnostic options are set before launch; do not scan the
    // process environment for every texture or constant bound while rendering.
    static const bool traceScene = getenv("BLUR_DX11_TRACE_SCENE") != nullptr;
    if (traceScene && Blur2PCAutomation::Now() > 21 &&
        strstr(sampler, "DamageVolume"))
    {
        static unsigned tracedDamageVolume = 0;
        if (tracedDamageVolume++ < 12)
            Blur2PCAutomation::LogF("DX11 FX damage binding %s / %s lookup=%s resource=%p",
                NativeFX11EffectName(effect), sampler, name ? name : "<missing>", texture);
    }
    if (!name) return;
    // [Blur1PCDX11] Every recovered vehicle damage-volume sampler is a
    // vertex sampler. Match FxContext's PC default substitution: its 2D
    // default becomes the initialized zero float volume, preserving type.
    if (param.damageVolume)
        texture = reinterpret_cast<ID3D11Resource*>(const_cast<void*>(GetDefaultVertexTextureFor(texture)));
    const Display::C_SamplerState& live = *g_SamplerStateMgr.GetSamplerState(samplerState);
    ID3D11Device* device = Display::Device()->GetD3DDevice();
    S_Param& memo = const_cast<S_Param&>(param);
    HRESULT samplerResult = param.samplerResult;
    bool samplerBound = memo.liveSampler && memo.liveId == samplerState &&
        memcmp(memo.live, &live, sizeof(memo.live)) == 0;
    for (size_t i = 0; samplerBound && i < param.samplersBound.size(); ++i)
        samplerBound = *param.samplersBound[i] == memo.liveSampler;
    if (!samplerBound)
    {
        // The sampler state differs from the one these variables hold.
        D3D11_SAMPLER_DESC desc = {};
        // [Blur2PC] PC Context_Base only applies Display2 sampler fields. Its
        // MaxAnisotropy override is Xenon-only, so retain the PC device default 1.
        samplerResult = Display::D3D11::MakeSamplerDesc(
            static_cast<D3DTEXTUREFILTERTYPE>(live.m_TextureFilterMin),
            static_cast<D3DTEXTUREFILTERTYPE>(live.m_TextureFilterMag),
            static_cast<D3DTEXTUREFILTERTYPE>(live.m_TextureFilterMip),
            static_cast<D3DTEXTUREADDRESS>(live.m_TextureAddressU),
            static_cast<D3DTEXTUREADDRESS>(live.m_TextureAddressV),
            static_cast<D3DTEXTUREADDRESS>(live.m_TextureAddressW), 1, &desc,
            live.m_TextureBorderColour, live.m_TextureLodBias) ? S_OK : E_INVALIDARG;
        memo.liveId = -1;
        memo.liveSampler = nullptr;
        if (SUCCEEDED(samplerResult))
        {
            auto& state = g_samplers[desc];
            if (!state) samplerResult = device->CreateSamplerState(&desc, &state);
            if (SUCCEEDED(samplerResult))
            {
                samplerResult = param.samplerResult;
                bool bound = true;
                for (size_t i = 0; i < param.samplers.size(); ++i)
                {
                    const HRESULT hr = param.samplers[i]->SetSampler(0, state.Get());
                    if (FAILED(hr)) { samplerResult = hr; bound = false; *param.samplersBound[i] = nullptr; break; }
                    *param.samplersBound[i] = state.Get();
                }
                if (bound)
                {
                    memo.liveId = samplerState;
                    memcpy(memo.live, &live, sizeof(memo.live));
                    memo.liveSampler = state.Get();
                }
            }
        }
    }
    if (FAILED(samplerResult)) { static unsigned failures = 0; if (failures++ < 24)
        Blur2PCAutomation::LogF("DX11 FX sampler binding failed %s / %s result=%08x",
            NativeFX11EffectName(effect), sampler, static_cast<unsigned>(samplerResult)); }
    static const bool traceGamma = getenv("BLUR_DX11_TRACE_TEXTURE_GAMMA") != nullptr;
    const unsigned generation = Display::D3D11::TextureViewOwnerGeneration();
    if (param.view && texture && !traceGamma)
    {
        bool srgb = live.m_TextureGamma == Display2::E_Bool_True;
        const Display::E_Bool forced = Display::Device()->GetForceTextureGamma();
        if (forced != Display::E_Bool_Default) srgb = forced == Display::E_Bool_True;
        const S_BoundView& bound = *param.viewBound;
        if (bound.texture == texture && bound.srgb == srgb && bound.generation == generation) return;
    }
    ID3D11ShaderResourceView* view = nullptr;
    bool owned = false, viewSRGB = false;
    if (texture)
    {
        // [Blur2PC] The PC FX path binds the current parameter's sampler state,
        // including immediate overrides; compiled metadata only describes its default.
        bool srgb = live.m_TextureGamma == Display2::E_Bool_True;
        const Display::E_Bool forced = Display::Device()->GetForceTextureGamma();
        if (forced != Display::E_Bool_Default) srgb = forced == Display::E_Bool_True;
        HRESULT result = param.viewResult;
        if (param.view)
        {
            // Reuse the view the effect already retains when it matches.
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> previous;
            result = param.view->GetResourceArray(&previous, 0, 1);
            if (SUCCEEDED(result))
                result = Display::D3D11::CreateTextureView(device, texture, srgb, &view, previous.Get(), &owned);
        }
        viewSRGB = srgb;
        if (traceGamma)
        {
            bool authored = false;
            const HRESULT metadata = g_effects.TextureUsesSRGB(NativeFX11EffectName(effect), name, &authored, techniqueName);
            static unsigned counts[4] = {};
            const unsigned category = (immediate ? 2U : 0U) + (SUCCEEDED(metadata) && authored != srgb ? 1U : 0U);
            if (counts[category]++ < 12)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
                if (view) view->GetDesc(&viewDesc);
                Blur2PCAutomation::LogF("DX11 FX gamma %s / %s immediate=%u state=%d authored=%u live=%u forced=%u view=%u result=%08x",
                    NativeFX11EffectName(effect), name, immediate, samplerState, authored, srgb,
                    static_cast<unsigned>(forced), static_cast<unsigned>(viewDesc.Format), static_cast<unsigned>(result));
            }
        }
        if (FAILED(result)) { static unsigned failures = 0; if (failures++ < 24)
            Blur2PCAutomation::LogF("DX11 FX texture view failed %s / %s srgb=%u result=%08x",
                NativeFX11EffectName(effect), name, srgb, static_cast<unsigned>(result)); }
    }
    HRESULT result = param.viewResult;
    if (param.view)
    {
        ID3D11ShaderResourceView* bound = view;
        if (bound && param.cube)
        {
            // [Blur1PCDX11] PC cube sampling a bound 2D texture returns RGBA zero
            // (including nonzero texels). An unbound native SRV preserves that
            // measured result without the D3D11 draw-time dimension error.
            D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
            bound->GetDesc(&viewDesc);
            if (viewDesc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D) bound = nullptr;
        }
        result = param.view->SetResource(bound);
        S_BoundView& memo = *param.viewBound;
        memo = S_BoundView();
        if (SUCCEEDED(result) && owned)
        {
            memo.texture = texture;
            memo.srgb = viewSRGB;
            memo.generation = generation;
        }
    }
    if (FAILED(result)) { static unsigned failures = 0; if (failures++ < 24)
        Blur2PCAutomation::LogF("DX11 FX texture binding failed %s / %s result=%08x", effect->Name.Chars(), name, static_cast<unsigned>(result)); }
    if (view) view->Release();
}

// [Blur1PCDX11] Opt-in CPU evidence for vehicle-light state; never changes it.
void TraceVehicleParameter(const Effect* effect, const char* name, const void* data,
                           unsigned bytes, const char* source)
{
    static const bool traceVehicle = getenv("BLUR_DX11_TRACE_VEHICLE") != nullptr;
    if (!traceVehicle || !name || !data || bytes < 128 ||
        strcmp(name, "g_VehicleVtxParams") ||
        Blur2PCAutomation::Now() < 21 || Blur2PCAutomation::Now() >= 22) return;
    static unsigned vehicleInputs = 0;
    if (vehicleInputs++ >= 64) return;
    const F32* floats = reinterpret_cast<const F32*>(data);
    Blur2PCAutomation::LogF("DX11 vehicle CPU %s / %s source=%s world=%.6f %.6f %.6f rear=%.6f %.6f %.6f %.6f",
        effect->Name.Chars(), name, source, floats[9], floats[10], floats[11],
        floats[28], floats[29], floats[30], floats[31]);
}

bool IsTracedMatrix(const char* name)
{
    return strcmp(name, "g_mViewProj") == 0 || strcmp(name, "g_mWorld") == 0;
}

void BindSharedParameters(const Effect* effect, S_Technique& technique,
                          void* const* sources, const char* techniqueName)
{
    if (!sources[NGLFX_BIND_SHARED] || g_SharedEffect.ParameterData.Size() == 0) return;
    const void* base = &g_SharedEffect.ParameterData[0];
    if (technique.sharedCount != g_SharedEffect.Parameters.Size() || technique.sharedBase != base)
    {
        // The shared parameters this native effect declares, in shared-effect order.
        technique.shared.clear();
        technique.sharedCount = g_SharedEffect.Parameters.Size();
        technique.sharedBase = base;
        const uintptr_t start = reinterpret_cast<uintptr_t>(base);
        const size_t totalBytes = static_cast<size_t>(g_SharedEffect.ParameterData.Size()) * sizeof(U32);
        for (int i = 0; i < g_SharedEffect.Parameters.Size(); ++i)
        {
            const Parameter& parameter = g_SharedEffect.Parameters[i];
            if (!parameter.Data || parameter.Size <= 0) continue;
            S_Param& param = RawParam(technique, parameter.Name.Chars());
            if (!param.raw) continue;
            const uintptr_t address = reinterpret_cast<uintptr_t>(parameter.Data);
            if (address < start || address - start > totalBytes ||
                static_cast<size_t>(parameter.Size) * sizeof(U32) > totalBytes - (address - start)) continue;
            if (parameter.Type == NGLFX_PARAM_SAMPLER)
                TextureParam(technique, parameter.Name.Chars());
            technique.shared.push_back({ i, address - start, &param, IsTracedMatrix(parameter.Name.Chars()) });
        }
    }
    for (const S_Shared& shared : technique.shared)
    {
        const Parameter& parameter = g_SharedEffect.Parameters[shared.index];
        const U8* data = static_cast<const U8*>(sources[NGLFX_BIND_SHARED]) + shared.offset;
        static unsigned matricesTraced = 0;
        if (shared.matrix && matricesTraced < 12)
        {
            ++matricesTraced;
            const F32* floats = reinterpret_cast<const F32*>(data);
            Blur2PCAutomation::LogF("DX11 FX shared matrix %s / %s size=%d row0=%.3f %.3f %.3f %.3f row3=%.3f %.3f %.3f %.3f",
                effect->Name.Chars(), parameter.Name.Chars(), parameter.Size,
                floats[0], floats[1], floats[2], floats[3],
                floats[12], floats[13], floats[14], floats[15]);
        }
        if (parameter.Type == NGLFX_PARAM_SAMPLER)
        {
            const auto* sampler = reinterpret_cast<const nglFXParam_Sampler*>(data);
            BindTexture(effect, technique, parameter.Name.Chars(),
                        static_cast<ID3D11Resource*>(Core::Slot32ToPointer(sampler->Texture)), techniqueName, sampler->iSamplerState,
                        false, shared.param);
        }
        else
        {
            TraceVehicleParameter(effect, parameter.Name.Chars(), data, parameter.Size * sizeof(U32), "shared");
            const UINT bytes = static_cast<UINT>(parameter.Size) * sizeof(U32);
            const HRESULT result = shared.param->raw->SetRawValue(data, 0,
                bytes < shared.param->rawBytes ? bytes : shared.param->rawBytes);
            if (FAILED(result)) { static unsigned failures = 0; if (failures++ < 20)
                Blur2PCAutomation::LogF("DX11 FX shared binding failed %s / %s result=%08x", effect->Name.Chars(), parameter.Name.Chars(), static_cast<unsigned>(result)); }
        }
    }
}

void BindFunction(const Effect* effect, S_Technique& technique,
                  const Function& function, S_FunctionBindings& resolved,
                  void* const* sources, const char* techniqueName)
{
    const int count = function.Bindings.Size();
    const FunctionBinding* base = count ? &function.Bindings[0] : nullptr;
    if (resolved.base != base || resolved.count != count)
    {
        resolved.base = base;
        resolved.count = count;
        resolved.entries.clear();
        resolved.entries.reserve(count);
        for (int i = 0; i < count; ++i)
        {
            const FunctionBinding& binding = base[i];
            if (binding.ParameterIdx >= effect->Parameters.Size() || binding.Source > NGLFX_BIND_SHARED) continue;
            const char* name = binding.Name && binding.Name[0]
                ? binding.Name : effect->Parameters[binding.ParameterIdx].Name.Chars();
            if (!name || !name[0] || (binding.Type != NGLFX_BIND_SAMPLER && !binding.Count)) continue;
            S_Param& param = binding.Type == NGLFX_BIND_SAMPLER
                ? TextureParam(technique, name) : RawParam(technique, name);
            resolved.entries.push_back({ &binding, name, &param, IsTracedMatrix(name) });
        }
    }
    for (const S_FunctionBinding& entry : resolved.entries)
    {
        const FunctionBinding& binding = *entry.binding;
        if (!sources[binding.Source]) continue;
        const char* name = entry.name;
        const U8* data = static_cast<const U8*>(sources[binding.Source]) + binding.Offset;
        if (binding.Type == NGLFX_BIND_SAMPLER)
        {
            const auto* sampler = reinterpret_cast<const nglFXParam_Sampler*>(data);
            BindTexture(effect, technique, name,
                        static_cast<ID3D11Resource*>(Core::Slot32ToPointer(sampler->Texture)), techniqueName, sampler->iSamplerState,
                        false, entry.param);
        }
        else if (binding.Count)
        {
            static unsigned localMatricesTraced = 0;
            if (localMatricesTraced < 12 && entry.matrix)
            {
                ++localMatricesTraced;
                const F32* floats = reinterpret_cast<const F32*>(data);
                Blur2PCAutomation::LogF("DX11 FX local matrix %s / %s count=%u row0=%.3f %.3f %.3f %.3f row3=%.3f %.3f %.3f %.3f",
                    effect->Name.Chars(), name, binding.Count,
                    floats[0], floats[1], floats[2], floats[3],
                    floats[12], floats[13], floats[14], floats[15]);
            }
            static unsigned traced = 0;
            if (traced < 32)
            {
                ++traced;
                const F32* floats = reinterpret_cast<const F32*>(data);
                Blur2PCAutomation::LogF("DX11 FX value %s / %s count=%u src=%u offset=%u first=%.4f %.4f %.4f %.4f",
                    effect->Name.Chars(), name, binding.Count, binding.Source, binding.Offset,
                    floats[0], floats[1], floats[2], floats[3]);
            }
            TraceVehicleParameter(effect, name, data, binding.Count * 16U, "function");
            const UINT bytes = binding.Count * 16U;
            const HRESULT result = entry.param->raw ? entry.param->raw->SetRawValue(data, 0,
                bytes < entry.param->rawBytes ? bytes : entry.param->rawBytes) : HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            if (FAILED(result)) { static unsigned failures = 0; if (failures++ < 24)
                Blur2PCAutomation::LogF("DX11 FX parameter binding failed %s / %s result=%08x", effect->Name.Chars(), name, static_cast<unsigned>(result)); }
        }
    }
}

// [Blur1PCDX11] Queue the pass for the device's next binding or draw. FX11
// re-applies the whole pass (shaders, constant buffers, views, samplers), so
// consecutive parameter updates before one draw need a single Apply.
bool QueuePass(const Effect* effect, S_Technique& technique, int index)
{
    ID3DX11EffectPass* pass = FindPass(technique, index);
    if (!pass)
    {
        sprintf_s(g_fx11LastError, "%.80s / %.80s pass=%d", effect->Name.Chars(), technique.name, index);
        return false;
    }
    g_pendingPass = pass;
    g_currentEffect = effect->Name.Chars();
    g_currentTechnique = technique.name;
    g_currentFile = NativeFX11EffectName(effect);
    g_currentIndex = index;
    Display::Device()->SetNativeFXActive(true);
    return true;
}
}

void NativeFX11ReleaseCache(const void* effect)
{
    if (effect == g_lastEffect) { g_lastEffect = nullptr; g_lastCache = nullptr; }
    if (!g_caches.empty()) g_caches.erase(effect);
}

const void* NativeFX11Shader(const Effect* effect, int technique, int pass,
                            E_ShaderType type, unsigned* bytes)
{
    if (bytes) *bytes = 0;
    auto* found = Find(effect, technique);
    if (!found || !bytes) return nullptr;
    const void* shader = nullptr;
    SIZE_T length = 0;
    const HRESULT hr = type == E_ShaderType_VertexShader ?
        found->native->GetPassVertexShaderBytecode(found->name, pass, &shader, &length) :
        found->native->GetPassPixelShaderBytecode(found->name, pass, &shader, &length);
    if (FAILED(hr) || length > std::numeric_limits<unsigned>::max()) return nullptr;
    *bytes = static_cast<unsigned>(length);
    return shader;
}

bool NativeFX11Apply(const Effect* effect, const Pass* pass, int technique,
                     void* const* sources)
{
    auto* found = Find(effect, technique);
    const int index = PassIndex(effect, pass, technique);
    if (!found || index < 0 || !sources) return false;
    BindSharedParameters(effect, *found, sources, found->name);
    BindFunction(effect, *found, pass->VertexShader, found->bindings[index].vertex, sources, found->name);
    BindFunction(effect, *found, pass->PixelShader, found->bindings[index].pixel, sources, found->name);
    const bool queued = QueuePass(effect, *found, index);
    // [Blur1PCDX11] The 20 s window is an opt-in trace; reading the clock per apply costs frame time.
    static const bool traceWindow = getenv("BLUR_DX11_TRACE_APPLY") != nullptr;
    static unsigned applied = 0;
    if (applied++ < 18 || (traceWindow && Blur2PCAutomation::Now() > 19.9 && Blur2PCAutomation::Now() < 20.1))
        Blur2PCAutomation::LogF("DX11 FX apply %s / %s pass=%d queued=%u t=%.2f",
            effect->Name.Chars(), found->name, index, queued ? 1u : 0u, Blur2PCAutomation::Now());
    return queued;
}

const char* NativeFX11LastError()
{
    return g_fx11LastError;
}

const char* NativeFX11CurrentPass()
{
    static char current[256] = {};
    if (g_currentEffect)
        sprintf_s(current, "%.40s / %.60s pass=%d file=%.120s", g_currentEffect,
            g_currentTechnique ? g_currentTechnique : "", g_currentIndex, g_currentFile ? g_currentFile : "");
    return current;
}

bool NativeFX11HasPass(const Effect* effect, const Pass* pass, int technique)
{
    auto* found = Find(effect, technique);
    const int index = PassIndex(effect, pass, technique);
    return found && index >= 0 && FindPass(*found, index) != nullptr;
}

bool NativeFX11SetImmediate(const Effect* effect, const Pass* pass, int technique,
                            const char* parameter, const void* value, unsigned bytes,
                            bool texture, int samplerState)
{
    auto* found = Find(effect, technique);
    const int index = PassIndex(effect, pass, technique);
    if (!found || index < 0 || !parameter || !value) return false;
    if (texture && samplerState < 0) return false;
    if (texture)
        BindTexture(effect, *found, parameter,
                    *reinterpret_cast<ID3D11Resource* const*>(value), found->name,
                    samplerState, true);
    else
    {
        TraceVehicleParameter(effect, parameter, value, bytes, "immediate");
        SetRawParameter(*found, parameter, value, bytes);
    }
    return QueuePass(effect, *found, index);
}
}
