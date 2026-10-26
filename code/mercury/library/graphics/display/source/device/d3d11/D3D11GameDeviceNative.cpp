// [Blur1PCDX11] Game renderer state and draw calls on the D3D11 immediate context.
#include "stdafx.h"
#include "Display/Device/D3D11/PCD3D11GameDevice.h"
#include "Display/Device/D3D11/D3D11InputLayout.h"
#include "Display/Device/D3D11/D3D11States.h"
#include "Display/Device/D3D11/D3D11Formats.h"
#include <map>
#include <unordered_map>
#include <array>
#include <wrl/client.h>
#include <system/debug/Blur2PCAutomation.h>
#include <system/debug/Variables.h>
#include <intrin.h>
#include <d3dcompiler.h>
#include "Display/Utils/Screenshot.h"

// [Blur1PCDX11] Supplied by the fx library; declared here to keep display free of fx headers.
namespace FX { const char* NativeFX11LastError(); const char* NativeFX11CurrentPass(); }

namespace Display { namespace D3D11 {
// [Blur2PC] Opt-in diagnostics (BLUR_DX11_TRACE_* and friends) write under dx11_trace\ in the
// working folder; they used to name one developer's worktree.
static const char* TraceDir() { CreateDirectoryA("dx11_trace", nullptr); return "dx11_trace"; }
static const char* TracePath(const char* name)
{
    static char path[MAX_PATH];
    sprintf_s(path, "%s\\%s", TraceDir(), name);
    return path;
}
bool (*g_FlushNativeFX)() = nullptr;
// [Blur1PCDX11] FX effects declare the alpha-test cbuffer (b13), so an FX Apply
// binds the effect's copy there; the device's buffer is rebound after each one.
static bool g_alphaConstantsBound = false;
// [Blur1PCDX11] An FX Apply can replace any legacy sampler slot. A memo may
// skip a native bind only while no intervening pass has touched those slots.
static U64 g_samplerBindingEpoch = 1;
void FlushNativeFX()
{
    if (g_FlushNativeFX && g_FlushNativeFX())
    { g_alphaConstantsBound = false; ++g_samplerBindingEpoch; }
}
namespace {
using Microsoft::WRL::ComPtr;
struct NativeState
{
    struct SamplerMemo
    {
        D3D11_SAMPLER_DESC descriptor = {};
        ComPtr<ID3D11SamplerState> object;
        ID3D11Device* device = nullptr;
        U64 boundEpoch = 0;
    };
    SamplerMemo pixelSamplerMemo[16], vertexSamplerMemo[4];
    D3D11_BLEND_DESC blend = {};
    D3D11_DEPTH_STENCIL_DESC depth = {};
    D3D11_RASTERIZER_DESC raster = {};
    D3D11_SAMPLER_DESC sampler[16] = {};
    D3D11_SAMPLER_DESC vertexSampler[4] = {};
    ComPtr<ID3D11Buffer> vertexConstants;
    ComPtr<ID3D11Buffer> pixelConstants;
    // [Blur1PCDX11] Fixed-function PC alpha test, consumed by native PS exits.
    ComPtr<ID3D11Buffer> alphaConstants;
    // format: render target 0's alpha-test precision, see AlphaTestFormat.
    struct { UINT enabled = 0, function = 8; float reference = 0; UINT format = 8; } alpha;
    bool alphaDirty = true;
    float vertexData[256 * 4] = {};
    float pixelData[256 * 4] = {};
    bool vertexDirty = true, pixelDirty = true;
    bool nativeFXActive = false;
    // [Blur1PCDX11] D3D9 render states take effect at the next draw. Build the
    // D3D11 state objects once per draw from the latest values, not per setter.
    bool blendDirty = true, depthDirty = true, rasterDirty = true;
    D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    bool defaultVertexInputsBound = false;
    // [Blur1PCDX11] Mirror DX9's unchanged-input elision, comparing native
    // resources as a wrapper can acquire a replacement buffer in place.
    struct VertexInputMemo
    {
        ID3D11Buffer* buffer = nullptr;
        UINT stride = 0, offset = 0;
        bool known = false;
    };
    VertexInputMemo vertexInputs[E_Stream_Count];
    ID3D11Buffer* indexInput = nullptr;
    DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN;
    bool indexInputKnown = false, inputLayoutKnown = false;
    const C_VertexDeclaration* inputDeclaration = nullptr;
    const C_VertexShader* inputShader = nullptr;
    // [Blur1PCDX11] depth.FrontFace/BackFace retain the PC CW/CCW banks.
    bool stencilTwoSided = false, stencilTriangles = true;
    UINT stencilRef = 0xff;
    const C_Surface* colourSurfaces[4] = {};
    const C_Surface* depthSurface = nullptr;
    std::map<std::pair<const C_VertexDeclaration*, const C_VertexShader*>, ComPtr<ID3D11InputLayout>> inputLayouts;
    ComPtr<ID3D11Buffer> defaultVertexInputs[3];
    std::map<ID3D11Resource*, ComPtr<ID3D11Texture2D>> feedbackCopies;
    ComPtr<ID3D11ShaderResourceView> pixelViews[16];
    ComPtr<ID3D11ShaderResourceView> vertexViews[4];
    std::array<S8, 16> pixelTextureRegisters = {};
    std::array<S8, 16> vertexTextureRegisters = {};
    NativeState()
    {
        blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        blend.RenderTarget[0].SrcBlend = blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blend.RenderTarget[0].DestBlend = blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blend.RenderTarget[0].BlendOp = blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        depth.DepthEnable = TRUE; depth.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depth.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        depth.StencilReadMask = depth.StencilWriteMask = 0xff;
        depth.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depth.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        depth.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depth.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        depth.BackFace = depth.FrontFace;
        raster.FillMode = D3D11_FILL_SOLID; raster.CullMode = D3D11_CULL_BACK;
        raster.DepthClipEnable = TRUE;
        for (auto& s : sampler) InitSampler(s);
        for (auto& s : vertexSampler) InitSampler(s);
        for (S8 i = 0; i < 16; ++i) pixelTextureRegisters[i] = vertexTextureRegisters[i] = i;
    }
    static void InitSampler(D3D11_SAMPLER_DESC& s)
    {
        s.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        s.AddressU = s.AddressV = s.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        s.MaxAnisotropy = 1; s.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        s.MaxLOD = D3D11_FLOAT32_MAX;
    }
};
std::map<C_D3D11GameDevice*, NativeState> g_states;
NativeState& State(C_D3D11GameDevice* device)
{
    static C_D3D11GameDevice* last = nullptr;
    static NativeState* state = nullptr;
    if (device != last) { last = device; state = &g_states[device]; }
    return *state;
}

// [Blur1PCDX11] Immutable state objects by descriptor. The runtime also
// deduplicates identical descriptors, but only after a locked lookup per call.
// Descriptors are hashed as bytes, so keys are built with zeroed padding.
template <class T_Desc> struct C_DescHash
{
    size_t operator()(const T_Desc& desc) const
    {
        unsigned hash = 2166136261u;
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&desc);
        for (size_t i = 0; i < sizeof(desc); ++i) hash = (hash ^ bytes[i]) * 16777619u;
        return hash;
    }
};
template <class T_Desc> struct C_DescEqual
{
    bool operator()(const T_Desc& a, const T_Desc& b) const { return memcmp(&a, &b, sizeof(a)) == 0; }
};
template <class T_Desc, class T_State> using T_StateCache =
    std::unordered_map<T_Desc, ComPtr<T_State>, C_DescHash<T_Desc>, C_DescEqual<T_Desc>>;
struct S_StateCaches
{
    ID3D11Device* device = nullptr;
    T_StateCache<D3D11_BLEND_DESC, ID3D11BlendState> blend;
    T_StateCache<D3D11_DEPTH_STENCIL_DESC, ID3D11DepthStencilState> depth;
    T_StateCache<D3D11_RASTERIZER_DESC, ID3D11RasterizerState> raster;
    T_StateCache<D3D11_SAMPLER_DESC, ID3D11SamplerState> sampler;
};
S_StateCaches& StateCaches(ID3D11Device* device)
{
    static S_StateCaches caches;
    if (caches.device != device)
    {
        caches.blend.clear(); caches.depth.clear(); caches.raster.clear(); caches.sampler.clear();
        caches.device = device;
    }
    return caches;
}

// [Blur1PCDX11] Opt-in, bounded stacks identify callers hidden by draw wrappers.
void TraceMissingVertexShader(bool indexed, U32 count, const C_VertexShader* shader, bool nativeFX)
{
    // [Blur1PCDX11] Launch-time diagnostic flags must not scan the environment
    // at every draw when the corresponding trace is disabled.
    static const bool enabled = getenv("BLUR_DX11_TRACE_MISSING_VS") != nullptr;
    if (!enabled) return;
    static std::map<ULONG, bool> stacks;
    if (stacks.size() >= 16) return;
    void* frames[12] = {};
    ULONG hash = 0;
    const USHORT depth = CaptureStackBackTrace(0, 12, frames, &hash);
    if (!stacks.emplace(hash, true).second) return;
    Blur2PCAutomation::LogF("DX11 missing VS stack=%08x indexed=%u count=%u shader=%p bytes=%u nativeFX=%u pass=%s module=%p",
        hash, indexed, count, shader, shader ? static_cast<unsigned>(shader->m_Bytecode.size()) : 0,
        nativeFX, FX::NativeFX11CurrentPass(), GetModuleHandleW(nullptr));
    for (USHORT frame = 0; frame < depth; ++frame)
        Blur2PCAutomation::LogF("DX11 missing VS stack=%08x frame=%u address=%p", hash, frame, frames[frame]);
}

// [Blur1PCDX11] D3D9 samples texture stage N through sampler register sN. The SM5 recompiles of the
// built-in shaders keep the sN registers but fxc packs the textures of legacy samplers into t0, t1, ...
// in order of first use, so the texture of stage N has to go to the t register the shader gave it.
// Entry N is that register, or -1 when the shader does not sample stage N.
typedef std::array<S8, 16> SamplerTextureMap;
const SamplerTextureMap& TextureRegisters(const std::vector<U8>& bytecode)
{
    // Keyed by the DXBC container checksum (bytes 4-19), so a freed shader's reused address cannot alias.
    static std::map<std::array<U32, 4>, SamplerTextureMap> maps;
    std::array<U32, 4> key = {};
    if (bytecode.size() >= 20) memcpy(key.data(), bytecode.data() + 4, sizeof(key));
    auto found = maps.find(key);
    if (found != maps.end()) return found->second;
    SamplerTextureMap& map = maps[key];
    map.fill(-1);
    ComPtr<ID3D11ShaderReflection> reflection;
    if (bytecode.empty() || FAILED(D3DReflect(bytecode.data(), bytecode.size(), IID_PPV_ARGS(&reflection))))
    {
        for (S8 i = 0; i < 16; ++i) map[i] = i;
        return map;
    }
    D3D11_SHADER_DESC desc = {};
    reflection->GetDesc(&desc);
    std::vector<D3D11_SHADER_INPUT_BIND_DESC> bindings(desc.BoundResources);
    for (UINT i = 0; i < desc.BoundResources; ++i)
        reflection->GetResourceBindingDesc(i, &bindings[i]);
    bool claimed[16] = {};
    for (const auto& texture : bindings)
    {
        if (texture.Type != D3D_SIT_TEXTURE || texture.BindPoint >= 16) continue;
        for (const auto& sampler : bindings)
            if (sampler.Type == D3D_SIT_SAMPLER && sampler.BindPoint < 16 && strcmp(sampler.Name, texture.Name) == 0)
            {
                map[sampler.BindPoint] = static_cast<S8>(texture.BindPoint);
                claimed[texture.BindPoint] = true;
            }
    }
    // Textures declared apart from their sampler (hand-written SM5): stage N feeds tN.
    for (const auto& texture : bindings)
        if (texture.Type == D3D_SIT_TEXTURE && texture.BindPoint < 16 && !claimed[texture.BindPoint] && map[texture.BindPoint] < 0)
            map[texture.BindPoint] = static_cast<S8>(texture.BindPoint);
    return map;
}
void BindPixelViews(NativeState& state, ID3D11DeviceContext* context, const C_PixelShader* shader)
{
    for (S8 i = 0; i < 16; ++i) state.pixelTextureRegisters[i] = i;
    if (!shader || !shader->m_pd3dPixelShader) return;
    const SamplerTextureMap& map = TextureRegisters(shader->m_Bytecode);
    state.pixelTextureRegisters = map;
    for (U32 stage = 0; stage < 16; ++stage)
        if (map[stage] >= 0)
        {
            ID3D11ShaderResourceView* raw = state.pixelViews[stage].Get();
            context->PSSetShaderResources(map[stage], 1, &raw);
        }
}
void BindVertexViews(NativeState& state, ID3D11DeviceContext* context, const C_VertexShader* shader)
{
    for (S8 i = 0; i < 16; ++i) state.vertexTextureRegisters[i] = i;
    if (!shader || !shader->m_pd3dVertexShader) return;
    const SamplerTextureMap& map = TextureRegisters(shader->m_Bytecode);
    state.vertexTextureRegisters = map;
    for (U32 stage = 0; stage < 4; ++stage)
        if (map[stage] >= 0)
        {
            ID3D11ShaderResourceView* raw = state.vertexViews[stage].Get();
            context->VSSetShaderResources(map[stage], 1, &raw);
        }
}
ComPtr<ID3D11DeviceContext> Context(ID3D11Device* device)
{
    static ID3D11Device* last = nullptr;
    static ID3D11DeviceContext* immediate = nullptr;
    if (device != last)
    {
        last = device;
        immediate = nullptr;
        if (device)
        {
            device->GetImmediateContext(&immediate);
            if (immediate) immediate->Release(); // owned by the device
        }
    }
    return ComPtr<ID3D11DeviceContext>(immediate);
}
D3D11_BLEND Blend(E_Blend value)
{
    switch(value) {
    case E_Blend_Zero: return D3D11_BLEND_ZERO;
    case E_Blend_One: return D3D11_BLEND_ONE;
    case E_Blend_SrcColour: return D3D11_BLEND_SRC_COLOR;
    case E_Blend_InvSrcColour: return D3D11_BLEND_INV_SRC_COLOR;
    case E_Blend_SrcAlpha: return D3D11_BLEND_SRC_ALPHA;
    case E_Blend_InvSrcAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
    case E_Blend_DstAlpha: return D3D11_BLEND_DEST_ALPHA;
    case E_Blend_InvDstAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
    case E_Blend_DstColour: return D3D11_BLEND_DEST_COLOR;
    case E_Blend_InvDstColour: return D3D11_BLEND_INV_DEST_COLOR;
    default: return D3D11_BLEND_ONE;
    }
}
D3D11_BLEND_OP BlendOp(E_BlendOp value)
{
    switch(value) {
    case E_BlendOp_Subtract: return D3D11_BLEND_OP_SUBTRACT;
    case E_BlendOp_RevSubtract: return D3D11_BLEND_OP_REV_SUBTRACT;
    case E_BlendOp_Min: return D3D11_BLEND_OP_MIN;
    case E_BlendOp_Max: return D3D11_BLEND_OP_MAX;
    default: return D3D11_BLEND_OP_ADD;
    }
}
D3D11_COMPARISON_FUNC Compare(E_CmpFunc value)
{
    switch(value) {
    case E_CmpFunc_Never: return D3D11_COMPARISON_NEVER;
    case E_CmpFunc_Less: return D3D11_COMPARISON_LESS;
    case E_CmpFunc_Equal: return D3D11_COMPARISON_EQUAL;
    case E_CmpFunc_LessEqual: return D3D11_COMPARISON_LESS_EQUAL;
    case E_CmpFunc_Greater: return D3D11_COMPARISON_GREATER;
    case E_CmpFunc_NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
    case E_CmpFunc_GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
    default: return D3D11_COMPARISON_ALWAYS;
    }
}
D3D11_TEXTURE_ADDRESS_MODE Address(E_TextureAddress value)
{
    switch(value) {
    case E_TextureAddress_Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
    case E_TextureAddress_Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
    case E_TextureAddress_Border: return D3D11_TEXTURE_ADDRESS_BORDER;
    default: return D3D11_TEXTURE_ADDRESS_WRAP;
    }
}
D3D11_FILTER Filter(E_TextureFilter mag, E_TextureFilter min, E_TextureFilter mip)
{
    if (mag == E_TextureFilter_Anisotropic || min == E_TextureFilter_Anisotropic)
        return D3D11_FILTER_ANISOTROPIC;
    const int bits = (min != E_TextureFilter_Point && min != E_TextureFilter_None ? 0x10 : 0) |
                     (mag != E_TextureFilter_Point && mag != E_TextureFilter_None ? 0x04 : 0) |
                     (mip != E_TextureFilter_Point && mip != E_TextureFilter_None ? 0x01 : 0);
    return static_cast<D3D11_FILTER>(bits);
}
D3D11_PRIMITIVE_TOPOLOGY Topology(E_PrimitiveType value)
{
    switch(value) {
    case E_PrimitiveType_PointList: return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    case E_PrimitiveType_LineList: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    case E_PrimitiveType_LineStrip: return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case E_PrimitiveType_TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    default: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }
}
void ApplyBlend(C_D3D11GameDevice* device) { State(device).blendDirty = true; }
void ApplyDepth(C_D3D11GameDevice* device) { State(device).depthDirty = true; }
// [Blur1PCDX11] Only these draws set the input-assembler topology (traces excepted).
void SetTopology(C_D3D11GameDevice* device, ID3D11DeviceContext* context, D3D11_PRIMITIVE_TOPOLOGY topology)
{
    NativeState& state = State(device);
    if (state.topology == topology) return;
    state.topology = topology;
    context->IASetPrimitiveTopology(topology);
}
void SetStencilTopology(C_D3D11GameDevice* device, D3D11_PRIMITIVE_TOPOLOGY topology)
{
    NativeState& state = State(device);
    const bool triangles = topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ||
                           topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    if (state.stencilTriangles == triangles) return;
    state.stencilTriangles = triangles;
    if (state.stencilTwoSided) ApplyDepth(device);
}
void ApplyRaster(C_D3D11GameDevice* device)
{
    NativeState& state = State(device);
    state.rasterDirty = true;
    if (state.stencilTwoSided) state.depthDirty = true;
}
// [Blur1PCDX11] Bind the latest blend, depth-stencil and rasterizer states
// before a draw. Native FX passes carry no render states, so FX cannot
// override these between the setter and the draw.
void ApplyDirtyStates(C_D3D11GameDevice* device, ID3D11DeviceContext* context)
{
    NativeState& cached = State(device);
    if (!cached.blendDirty && !cached.depthDirty && !cached.rasterDirty) return;
    S_StateCaches& caches = StateCaches(device->GetD3DDevice());
    if (cached.blendDirty)
    {
        cached.blendDirty = false;
        D3D11_BLEND_DESC key;
        memset(&key, 0, sizeof(key));
        key.AlphaToCoverageEnable = cached.blend.AlphaToCoverageEnable;
        key.IndependentBlendEnable = cached.blend.IndependentBlendEnable;
        for (UINT i = 0; i < 8; ++i)
        {
            const D3D11_RENDER_TARGET_BLEND_DESC& from = cached.blend.RenderTarget[i];
            D3D11_RENDER_TARGET_BLEND_DESC& to = key.RenderTarget[i];
            to.BlendEnable = from.BlendEnable; to.SrcBlend = from.SrcBlend; to.DestBlend = from.DestBlend;
            to.BlendOp = from.BlendOp; to.SrcBlendAlpha = from.SrcBlendAlpha; to.DestBlendAlpha = from.DestBlendAlpha;
            to.BlendOpAlpha = from.BlendOpAlpha; to.RenderTargetWriteMask = from.RenderTargetWriteMask;
        }
        ComPtr<ID3D11BlendState>& state = caches.blend[key];
        if (state || SUCCEEDED(device->GetD3DDevice()->CreateBlendState(&key, &state)))
        { const float factors[4] = {1,1,1,1}; context->OMSetBlendState(state.Get(), factors, 0xffffffff); }
    }
    if (cached.depthDirty)
    {
        cached.depthDirty = false;
        D3D11_DEPTH_STENCIL_DESC desc = cached.depth;
        ResolveStencilFaces(desc, cached.stencilTwoSided,
                            cached.raster.FrontCounterClockwise != FALSE, cached.stencilTriangles);
        D3D11_DEPTH_STENCIL_DESC key;
        memset(&key, 0, sizeof(key));
        key.DepthEnable = desc.DepthEnable; key.DepthWriteMask = desc.DepthWriteMask; key.DepthFunc = desc.DepthFunc;
        key.StencilEnable = desc.StencilEnable; key.StencilReadMask = desc.StencilReadMask;
        key.StencilWriteMask = desc.StencilWriteMask; key.FrontFace = desc.FrontFace; key.BackFace = desc.BackFace;
        ComPtr<ID3D11DepthStencilState>& state = caches.depth[key];
        if (state || SUCCEEDED(device->GetD3DDevice()->CreateDepthStencilState(&key, &state)))
            context->OMSetDepthStencilState(state.Get(), cached.stencilRef);
    }
    if (cached.rasterDirty)
    {
        cached.rasterDirty = false;
        ComPtr<ID3D11RasterizerState>& state = caches.raster[cached.raster];
        if (state || SUCCEEDED(device->GetD3DDevice()->CreateRasterizerState(&cached.raster, &state)))
            context->RSSetState(state.Get());
    }
}
void ApplySampler(C_D3D11GameDevice* device, U32 slot, bool vertex)
{
    FlushNativeFX();
    auto context = Context(device->GetD3DDevice()); if (!context || slot >= (vertex ? 4U : 16U)) return;
    D3D11_SAMPLER_DESC& desc = vertex ? State(device).vertexSampler[slot] : State(device).sampler[slot];
    // [Blur2PC] D3D9 MIPFILTER=NONE samples mip zero even when minifying.
    const auto mip = vertex ? device->m_VertexSampler[slot].m_FilterMip : device->m_Sampler[slot].m_FilterMip;
    desc.MaxLOD = mip == E_TextureFilter_None ? 0.f : D3D11_FLOAT32_MAX;
    // [Blur1PCDX11] A material sets several sampler fields consecutively and
    // often repeats the same descriptor. Reuse the last lookup for this slot.
    NativeState::SamplerMemo& memo = vertex ? State(device).vertexSamplerMemo[slot] : State(device).pixelSamplerMemo[slot];
    ID3D11Device* nativeDevice = device->GetD3DDevice();
    if (!memo.object || memo.device != nativeDevice || memcmp(&memo.descriptor, &desc, sizeof(desc)) != 0)
    {
        ComPtr<ID3D11SamplerState>& object = StateCaches(nativeDevice).sampler[desc];
        if (!object && FAILED(nativeDevice->CreateSamplerState(&desc, &object))) return;
        memo.descriptor = desc; memo.object = object; memo.device = nativeDevice; memo.boundEpoch = 0;
    }
    if (memo.boundEpoch != g_samplerBindingEpoch)
    {
        ID3D11SamplerState* raw = memo.object.Get();
        if (vertex) context->VSSetSamplers(slot, 1, &raw); else context->PSSetSamplers(slot, 1, &raw);
        D3DX11InvalidateBoundState(vertex ? 1u : 2u, 4u);
        memo.boundEpoch = g_samplerBindingEpoch;
    }
}
void BindConstants(C_D3D11GameDevice* device, bool vertex)
{
    NativeState& state = State(device);
    // [Blur1PCDX11] FX11 owns its reflected constant buffers. Legacy register
    // writes may update the cache, but must not replace those buffers on the GPU.
    if (state.nativeFXActive) return;
    FlushNativeFX();
    auto context = Context(device->GetD3DDevice()); if (!context) return;
    auto& buffer = vertex ? state.vertexConstants : state.pixelConstants;
    bool& dirty = vertex ? state.vertexDirty : state.pixelDirty;
    float* data = vertex ? state.vertexData : state.pixelData;
    if (!buffer)
    {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = 256 * 4 * sizeof(float);
        desc.Usage = D3D11_USAGE_DEFAULT; desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        if (FAILED(device->GetD3DDevice()->CreateBuffer(&desc, nullptr, &buffer))) return;
    }
    if (dirty) { context->UpdateSubresource(buffer.Get(), 0, nullptr, data, 0, 0); dirty = false; }
    ID3D11Buffer* raw[8] = {};
    for (auto& slot : raw) slot = buffer.Get();
    if (vertex) context->VSSetConstantBuffers(0, 8, raw);
    else context->PSSetConstantBuffers(0, 8, raw);
    D3DX11InvalidateBoundState(vertex ? 1u : 2u, 2u);
    if (!vertex) device->BindHDROutputConstants(); // [Blur2PC][HDR]
}
// [Blur1PCDX11] D3D9 compares alpha at render target 0's precision: 0 float32,
// 1 float16, otherwise the UNORM bit count. Measured on the D3D9 HAL for
// 8-, 10- and 16-bit UNORM and 16/32-bit float targets (analysis.md).
UINT AlphaTestFormat(DXGI_FORMAT format)
{
    switch (GetLinearFormat(format))
    {
    case DXGI_FORMAT_R16G16B16A16_FLOAT: case DXGI_FORMAT_R16G16_FLOAT: case DXGI_FORMAT_R16_FLOAT:
        return 1;
    case DXGI_FORMAT_R32G32B32A32_FLOAT: case DXGI_FORMAT_R32G32_FLOAT: case DXGI_FORMAT_R32_FLOAT:
        return 0;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
        return 10;
    case DXGI_FORMAT_R16G16B16A16_UNORM: case DXGI_FORMAT_R16G16_UNORM: case DXGI_FORMAT_R16_UNORM:
        return 16;
    case DXGI_FORMAT_B8G8R8A8_UNORM: case DXGI_FORMAT_B8G8R8X8_UNORM: case DXGI_FORMAT_R8G8B8A8_UNORM:
        return 8;
    default:
    {
        // Unmeasured target class: compare unconverted and log it (unverified).
        static unsigned traced = 0;
        if (traced++ < 8) Blur2PCAutomation::LogF("DX11 alpha test unmeasured target format %d", (int)format);
        return 0;
    }
    }
}
void BindAlphaTest(C_D3D11GameDevice* device)
{
    FlushNativeFX();
    auto context = Context(device->GetD3DDevice()); if (!context) return;
    NativeState& state = State(device);
    if (!state.alphaConstants)
    {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(state.alpha);
        desc.Usage = D3D11_USAGE_DEFAULT; desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        if (FAILED(device->GetD3DDevice()->CreateBuffer(&desc, nullptr, &state.alphaConstants))) return;
        g_alphaConstantsBound = false;
    }
    if (state.alphaDirty)
    {
        context->UpdateSubresource(state.alphaConstants.Get(), 0, nullptr, &state.alpha, 0, 0);
        state.alphaDirty = false;
    }
    // [Blur1PCDX11] Apply after FX11, which owns the other shader buffers.
    if (g_alphaConstantsBound) return;
    ID3D11Buffer* buffer = state.alphaConstants.Get();
    context->PSSetConstantBuffers(13, 1, &buffer);
    D3DX11InvalidateBoundConstantBuffers(2u, 13, 1);
    g_alphaConstantsBound = true;
}
void BindRenderTargets(C_D3D11GameDevice* game)
{
    ID3D11Device* device = game->GetD3DDevice();
    auto context = Context(device); if (!context) return;
    NativeState& state = State(game);
    ComPtr<ID3D11RenderTargetView> views[4];
    ID3D11RenderTargetView* raw[4] = {};
    UINT count = 0;
    for (UINT slot = 0; slot < 4; ++slot)
    {
        const C_Surface* surface = state.colourSurfaces[slot];
        if (!surface || !surface->m_pd3dSurface) continue;
        D3D11_TEXTURE2D_DESC td = {};
        surface->m_pd3dSurface->GetDesc(&td);
        D3D11_RENDER_TARGET_VIEW_DESC vd = {};
        vd.Format = GetLinearFormat(td.Format);
        if (td.SampleDesc.Count > 1) vd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        else if (td.ArraySize > 1)
        {
            vd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            vd.Texture2DArray.MipSlice = surface->m_Subresource % td.MipLevels;
            vd.Texture2DArray.FirstArraySlice = surface->m_Subresource / td.MipLevels;
            vd.Texture2DArray.ArraySize = 1;
        }
        else
        {
            vd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            vd.Texture2D.MipSlice = surface->m_Subresource;
        }
        const HRESULT hr = GetRenderTargetView(device, surface->m_pd3dSurface, &vd, &views[slot]);
        if (FAILED(hr)) { static unsigned failures = 0; if (failures++ < 12)
            Blur2PCAutomation::LogF("DX11 RTV creation failed %08x slot=%u format=%d sub=%u", (unsigned)hr, slot, (int)td.Format, surface->m_Subresource); }
        raw[slot] = views[slot].Get();
        count = slot + 1;
        {
            static std::map<const void*, unsigned> traced;
            const void* key = surface->m_pd3dSurface;
            if (traced.find(key) == traced.end() && traced.size() < 40)
            {
                traced[key] = 1;
                Blur2PCAutomation::LogF("DX11 target %p tex=%p %ux%u fmt=%d sub=%u samples=%u", raw[slot], key,
                    td.Width, td.Height, static_cast<int>(td.Format), surface->m_Subresource, td.SampleDesc.Count);
            }
        }
    }
    ComPtr<ID3D11DepthStencilView> depthView;
    if (state.depthSurface && state.depthSurface->m_pd3dSurface)
    {
        // [Blur1PCDX11] D3D9 allowed a depth buffer larger than the colour target; D3D11
        // requires equal sizes, and the engine does leave an oversized one bound when it
        // switches to a downsampled target.
        D3D11_TEXTURE2D_DESC colourDesc = {};
        D3D11_TEXTURE2D_DESC depthDesc = {};
        if (state.colourSurfaces[0] && state.colourSurfaces[0]->m_pd3dSurface)
            state.colourSurfaces[0]->m_pd3dSurface->GetDesc(&colourDesc);
        state.depthSurface->m_pd3dSurface->GetDesc(&depthDesc);
        if (colourDesc.Width == depthDesc.Width && colourDesc.Height == depthDesc.Height &&
            colourDesc.SampleDesc.Count == depthDesc.SampleDesc.Count)
            GetDepthStencilView(device, state.depthSurface->m_pd3dSurface, nullptr, &depthView);
        else
        {
            static unsigned traced = 0;
            if (traced++ < 8)
                Blur2PCAutomation::LogF("DX11 skipping depth %ux%u for colour %ux%u",
                    depthDesc.Width, depthDesc.Height, colourDesc.Width, colourDesc.Height);
        }
    }
    context->OMSetRenderTargets(count, raw, depthView.Get());
    D3DX11InvalidateBoundState(3u, 8u);
    if (raw[0])
    {
        D3D11_TEXTURE2D_DESC colourDesc = {};
        state.colourSurfaces[0]->m_pd3dSurface->GetDesc(&colourDesc);
        const UINT format = AlphaTestFormat(colourDesc.Format);
        if (state.alpha.format != format) { state.alpha.format = format; state.alphaDirty = true; }
    }
    {
        const double now = Blur2PCAutomation::Now();
        if (now > 19.95 && now < 20.05)
        {
            D3D11_TEXTURE2D_DESC d = {};
            if (state.colourSurfaces[0] && state.colourSurfaces[0]->m_pd3dSurface)
                state.colourSurfaces[0]->m_pd3dSurface->GetDesc(&d);
            Blur2PCAutomation::LogF("DX11 bind colour=%p %ux%u count=%u",
                state.colourSurfaces[0] ? state.colourSurfaces[0]->m_pd3dSurface : nullptr,
                d.Width, d.Height, count);
        }
    }
}
}

// [Blur1PCDX11] The debug layer raises 0x87D on a corrupt call. Skip the failing draw and
// keep the rest of the frame so one run lists every broken shader pair.
// [Blur1PCDX11] Temporary: one-frame draw trace (target, shaders, blend, centre pixel after the draw).
unsigned g_dx11TraceFrame = 0;
unsigned g_dx11TraceDraw = 0;
// [Blur1PCDX11] Select a diagnostic pass by a used shader variable when
// visibility changes make draw ordinals unstable between automated runs.
static bool g_dx11TraceSelectedCapture = false;
void TraceDraw(ID3D11Device* device, ID3D11DeviceContext* context, const NativeState& state, UINT count, bool indexed)
{
    if (!g_dx11TraceFrame) return;
    ComPtr<ID3D11VertexShader> vs; ComPtr<ID3D11PixelShader> ps; ComPtr<ID3D11RenderTargetView> rtv;
    context->VSGetShader(&vs, nullptr, nullptr); context->PSGetShader(&ps, nullptr, nullptr);
    context->OMGetRenderTargets(1, &rtv, nullptr);
    ComPtr<ID3D11Resource> resource; if (rtv) rtv->GetResource(&resource);
    ComPtr<ID3D11Texture2D> texture; if (resource) resource.As(&texture);
    D3D11_TEXTURE2D_DESC desc = {}; if (texture) texture->GetDesc(&desc);
    char pixel[64] = "-";
    if (texture && desc.SampleDesc.Count == 1)
    {
        D3D11_TEXTURE2D_DESC one = desc;
        one.Width = one.Height = 1; one.MipLevels = one.ArraySize = 1;
        one.Usage = D3D11_USAGE_STAGING; one.BindFlags = 0; one.CPUAccessFlags = D3D11_CPU_ACCESS_READ; one.MiscFlags = 0;
        ComPtr<ID3D11Texture2D> staging;
        if (SUCCEEDED(device->CreateTexture2D(&one, nullptr, &staging)))
        {
            static float fx = -1.f, fy = 0.5f;
            if (fx < 0.f)
            {
                char value[32] = {}; fx = 0.5f;
                if (GetEnvironmentVariableA("BLUR_DX11_TRACE_PX", value, sizeof(value))) sscanf_s(value, "%f,%f", &fx, &fy);
            }
            const UINT px = (std::min)(desc.Width - 1, static_cast<UINT>(desc.Width * fx));
            const UINT py = (std::min)(desc.Height - 1, static_cast<UINT>(desc.Height * fy));
            D3D11_BOX box = { px, py, 0, px + 1, py + 1, 1 };
            context->CopySubresourceRegion(staging.Get(), 0, 0, 0, 0, texture.Get(), 0, &box);
            D3D11_MAPPED_SUBRESOURCE mapped = {};
            if (SUCCEEDED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)) && mapped.pData)
            {
                const U8* b = static_cast<const U8*>(mapped.pData);
                sprintf_s(pixel, "%02x%02x%02x%02x%02x%02x%02x%02x", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
                context->Unmap(staging.Get(), 0);
            }
        }
    }
    const D3D11_RENDER_TARGET_BLEND_DESC& blend = state.blend.RenderTarget[0];
    // [Blur1PCDX11] Optional scalar MRT1 evidence while isolating prepass writers.
    if (getenv("BLUR_DX11_TRACE_DEPTH_PX"))
    {
        ID3D11RenderTargetView* targets[2] = {};
        context->OMGetRenderTargets(2, targets, nullptr);
        ComPtr<ID3D11Resource> depthResource;
        if (targets[1]) targets[1]->GetResource(&depthResource);
        for (auto* target : targets) if (target) target->Release();
        ComPtr<ID3D11Texture2D> depthTexture;
        if (depthResource) depthResource.As(&depthTexture);
        D3D11_TEXTURE2D_DESC depthDesc = {};
        if (depthTexture) depthTexture->GetDesc(&depthDesc);
        if (depthDesc.Format == DXGI_FORMAT_R32_FLOAT && depthDesc.SampleDesc.Count == 1)
        {
            float x = .25f, y = .75f;
            sscanf_s(getenv("BLUR_DX11_TRACE_DEPTH_PX"), "%f,%f", &x, &y);
            const UINT px = (std::min)(depthDesc.Width - 1, static_cast<UINT>(depthDesc.Width * x));
            const UINT py = (std::min)(depthDesc.Height - 1, static_cast<UINT>(depthDesc.Height * y));
            depthDesc.Width = depthDesc.Height = depthDesc.MipLevels = depthDesc.ArraySize = 1;
            depthDesc.Usage = D3D11_USAGE_STAGING; depthDesc.BindFlags = depthDesc.MiscFlags = 0;
            depthDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            ComPtr<ID3D11Texture2D> staging;
            if (SUCCEEDED(device->CreateTexture2D(&depthDesc, nullptr, &staging)))
            {
                D3D11_BOX box = { px, py, 0, px + 1, py + 1, 1 };
                context->CopySubresourceRegion(staging.Get(), 0, 0, 0, 0, depthTexture.Get(), 0, &box);
                D3D11_MAPPED_SUBRESOURCE mapped = {};
                if (SUCCEEDED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)))
                {
                    Blur2PCAutomation::LogF("DX11 prepass depth draw=%u pixel=%u,%u value=%.9g", g_dx11TraceDraw, px, py, *static_cast<const float*>(mapped.pData));
                    context->Unmap(staging.Get(), 0);
                }
            }
        }
    }
    Blur2PCAutomation::LogF("DX11 trace %u %s count=%u VS=%p PS=%p fx=%s RT=%p %ux%u fmt=%u blend=%u %u/%u op=%u mask=%x depth=%u stencil=%u px=%s",
        g_dx11TraceDraw++, indexed ? "idx" : "arr", count, vs.Get(), ps.Get(),
        state.nativeFXActive ? FX::NativeFX11CurrentPass() : "-", resource.Get(), desc.Width, desc.Height, desc.Format,
        blend.BlendEnable, blend.SrcBlend, blend.DestBlend, blend.BlendOp, blend.RenderTargetWriteMask,
        state.depth.DepthEnable, state.depth.StencilEnable, pixel);
    // [Blur1PCDX11] One still from a selected draw in the existing one-frame
    // trace isolates material output without changing any render state.
    static bool captured = false;
    static const char* selectedDraw = getenv("BLUR_DX11_CAPTURE_TRACE_DRAW");
    static const char* capturePath = getenv("BLUR_DX11_CAPTURE_TRACE_PATH");
    if (!captured && selectedDraw && capturePath && texture &&
        (g_dx11TraceSelectedCapture || g_dx11TraceDraw-1 == strtoul(selectedDraw,nullptr,10)))
    {
        captured = true;
        TakeD3D11BackBufferScreenshot(capturePath,texture.Get());
        Blur2PCAutomation::LogF("DX11 trace still draw=%u path=%s",g_dx11TraceDraw-1,capturePath);
    }
    // [Blur1PCDX11] FX11 can bind state objects after the engine cache changes.
    // Read the actual GPU state for the bounded, opt-in one-frame trace.
    ComPtr<ID3D11DepthStencilState> actualDepth;
    ComPtr<ID3D11RasterizerState> actualRaster;
    ComPtr<ID3D11DepthStencilView> actualDSV;
    UINT reference = 0;
    context->OMGetDepthStencilState(&actualDepth, &reference);
    context->RSGetState(&actualRaster);
    context->OMGetRenderTargets(0, nullptr, &actualDSV);
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    D3D11_RASTERIZER_DESC rasterDesc = {};
    if (actualDepth) actualDepth->GetDesc(&depthDesc);
    if (actualRaster) actualRaster->GetDesc(&rasterDesc);
    ComPtr<ID3D11Resource> depthResource;
    if (actualDSV) actualDSV->GetResource(&depthResource);
    Blur2PCAutomation::LogF("DX11 trace   actual DSV=%p depth=%u/write%u/func%u stencil=%u/ref%u/read%u/write%u front=%u/%u/%u/%u back=%u/%u/%u/%u cull=%u/ccw%u",
        depthResource.Get(), depthDesc.DepthEnable, depthDesc.DepthWriteMask, depthDesc.DepthFunc,
        depthDesc.StencilEnable, reference, depthDesc.StencilReadMask, depthDesc.StencilWriteMask,
        depthDesc.FrontFace.StencilFunc, depthDesc.FrontFace.StencilFailOp, depthDesc.FrontFace.StencilDepthFailOp, depthDesc.FrontFace.StencilPassOp,
        depthDesc.BackFace.StencilFunc, depthDesc.BackFace.StencilFailOp, depthDesc.BackFace.StencilDepthFailOp, depthDesc.BackFace.StencilPassOp,
        rasterDesc.CullMode, rasterDesc.FrontCounterClockwise);
    if (desc.Width <= 4)
    {
        const float* c = state.pixelData;
        Blur2PCAutomation::LogF("DX11 trace   consts c0=%.4f %.4f %.4f %.4f c1=%.4f %.4f c15=%.4f %.4f c16=%.4f %.4f %.4f %.4f",
            c[0], c[1], c[2], c[3], c[4], c[5], c[60], c[61], c[64], c[65], c[66], c[67]);
        ComPtr<ID3D11ShaderResourceView> srv; context->PSGetShaderResources(0, 1, &srv);
        ComPtr<ID3D11Resource> source; if (srv) srv->GetResource(&source);
        ComPtr<ID3D11Buffer> cb; context->PSGetConstantBuffers(0, 1, &cb);
        D3D11_BUFFER_DESC cbDesc = {}; if (cb) cb->GetDesc(&cbDesc);
        Blur2PCAutomation::LogF("DX11 trace   t0=%p cb0=%p bytes=%u", source.Get(), cb.Get(), cbDesc.ByteWidth);
    }
    ComPtr<ID3D11InfoQueue> queue;
    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&queue))))
    {
        const UINT64 stored = queue->GetNumStoredMessages();
        for (UINT64 i = 0; i < stored; ++i)
        {
            SIZE_T size = 0;
            queue->GetMessage(i, nullptr, &size);
            std::vector<U8> bytes(size);
            D3D11_MESSAGE* message = reinterpret_cast<D3D11_MESSAGE*>(bytes.data());
            if (FAILED(queue->GetMessage(i, message, &size))) continue;
            if (message->Category == D3D11_MESSAGE_CATEGORY_STATE_CREATION && message->Severity == D3D11_MESSAGE_SEVERITY_INFO) continue;
            Blur2PCAutomation::LogF("DX11 trace   msg id=%d: %.300s", static_cast<int>(message->ID), message->pDescription);
        }
        queue->ClearStoredMessages();
    }
}

bool TryDraw(ID3D11DeviceContext* context, UINT count, UINT start, bool indexed, UINT base = 0)
{
    __try
    {
        if (indexed) context->DrawIndexed(count, start, base);
        else context->Draw(count, start);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

void C_D3D11GameDevice::InvalidateNativeInputBindings()
{
    NativeState& state = State(this);
    for (auto& input : state.vertexInputs) input.known = false;
    state.indexInputKnown = state.inputLayoutKnown = state.defaultVertexInputsBound = false;
    state.topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

void C_D3D11GameDevice::SetVertexBuffer(U32 stream, const C_VertexBuffer* buffer, U32 offset, U32 stride)
{
    if (stream >= E_Stream_Count) return;
    m_Stream[stream].m_pVertexBuffer = buffer; m_Stream[stream].m_VertexOffset = offset; m_Stream[stream].m_VertexStride = stride;
    ID3D11Buffer* raw = buffer ? buffer->m_pd3dVertexBuffer : nullptr;
    auto& memo = State(this).vertexInputs[stream];
    if (memo.known && memo.buffer == raw && memo.stride == stride && memo.offset == offset) return;
    auto context = Context(m_pd3dDevice); if (!context) return;
    context->IASetVertexBuffers(stream, 1, &raw, &stride, &offset);
    memo.buffer = raw; memo.stride = stride; memo.offset = offset; memo.known = true;
}
void C_D3D11GameDevice::SetIndexBuffer(const C_IndexBuffer* buffer)
{
    m_pIndexBuffer = buffer;
    auto& state = State(this);
    ID3D11Buffer* raw = buffer ? buffer->m_pd3dIndexBuffer : nullptr;
    const DXGI_FORMAT format = buffer && buffer->GetFormat() == Display2::E_IndexFormat_32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
    if (state.indexInputKnown && state.indexInput == raw && state.indexFormat == format) return;
    auto context = Context(m_pd3dDevice); if (!context) return;
    context->IASetIndexBuffer(raw, format, 0);
    state.indexInput = raw; state.indexFormat = format; state.indexInputKnown = true;
}
void C_D3D11GameDevice::SetVertexDeclaration(const C_VertexDeclaration* declaration)
{
    m_pVertexDeclaration = declaration;
    // [Blur1PCDX11] Supply the fixed values used by absent D3D9 semantics.
    NativeState& state = State(this);
    if (state.inputLayoutKnown && state.inputDeclaration == declaration && state.inputShader == m_pVertexShader) return;
    auto context = Context(m_pd3dDevice); if (!context) return;
    const U32 defaults[3][4] = {
        {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 0x3f800000}
    };
    ID3D11Buffer* defaultBuffers[3] = {};
    const UINT defaultStrides[3] = {};
    const UINT defaultOffsets[3] = {};
    for (UINT input = 0; input < 3; ++input)
    {
        if (!state.defaultVertexInputs[input])
        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = 16;
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            D3D11_SUBRESOURCE_DATA initial = {};
            initial.pSysMem = defaults[input];
            m_pd3dDevice->CreateBuffer(&desc, &initial, &state.defaultVertexInputs[input]);
        }
        defaultBuffers[input] = state.defaultVertexInputs[input].Get();
    }
    if (!state.defaultVertexInputsBound)
    {
        context->IASetVertexBuffers(29, 3, defaultBuffers, defaultStrides, defaultOffsets);
        state.defaultVertexInputsBound = true;
    }
    ID3D11InputLayout* nativeLayout = nullptr;
    if (declaration && m_pVertexShader && !m_pVertexShader->m_Bytecode.empty())
    {
        auto& layouts = state.inputLayouts;
        const auto key = std::make_pair(declaration, m_pVertexShader);
        auto found = layouts.find(key);
        if (found == layouts.end())
        {
            ComPtr<ID3D11InputLayout> layout;
            const HRESULT hr = CreateGameInputLayout(m_pd3dDevice, declaration->m_Elements,
                declaration->m_ElementCount, m_pVertexShader->m_Bytecode.data(),
                m_pVertexShader->m_Bytecode.size(), layout);
            if (SUCCEEDED(hr)) found = layouts.emplace(key, std::move(layout)).first;
            else { static unsigned failures = 0; if (failures++ < 16)
                Blur2PCAutomation::LogF("DX11 input layout failed %08x decl=%p elements=%u VS=%p bytes=%u: %s", (unsigned)hr, declaration, declaration->m_ElementCount, m_pVertexShader, (unsigned)m_pVertexShader->m_Bytecode.size(), LastGameInputLayoutDiagnostic()); }
        }
        if (found != layouts.end()) nativeLayout = found->second.Get();
    }
    context->IASetInputLayout(nativeLayout);
    state.inputDeclaration = declaration; state.inputShader = m_pVertexShader;
    state.inputLayoutKnown = nativeLayout || !declaration || !m_pVertexShader;
}
void C_D3D11GameDevice::SetVertexShader(const C_VertexShader* shader)
{
    FlushNativeFX();
    m_pVertexShader = shader; State(this).nativeFXActive = false;
    if (shader && !shader->m_pd3dVertexShader)
    {
        static unsigned traced = 0;
        if (traced++ < 8)
            Blur2PCAutomation::LogF("DX11 VS object %p has no native shader bytes=%u caller=%p rva=%08x",
                shader, (unsigned)shader->m_Bytecode.size(), _ReturnAddress(),
                (unsigned)((const char*)_ReturnAddress() - (const char*)GetModuleHandleA(nullptr)));
    }
    auto context = Context(m_pd3dDevice); if (!context) return;
    context->VSSetShader(shader ? shader->m_pd3dVertexShader : nullptr, nullptr, 0);
    D3DX11InvalidateBoundState(1u, 1u | 8u);
    BindVertexViews(State(this), context.Get(), shader);
    if (m_pVertexDeclaration) SetVertexDeclaration(m_pVertexDeclaration);
}
const C_VertexShader* C_D3D11GameDevice::GetVertexShader() { return m_pVertexShader; }
void C_D3D11GameDevice::SetPixelShader(const C_PixelShader* shader)
{
    FlushNativeFX();
    m_pPixelShader = shader; State(this).nativeFXActive = false;
    auto context = Context(m_pd3dDevice); if (!context) return;
    context->PSSetShader(shader ? shader->m_pd3dPixelShader : nullptr, nullptr, 0);
    D3DX11InvalidateBoundState(2u, 1u | 8u);
    BindPixelViews(State(this), context.Get(), shader);
}
const C_PixelShader* C_D3D11GameDevice::GetPixelShader() { return m_pPixelShader; }
// [Blur1PCDX11] Raw statistics of a texture, for finding the black link of a chain.
void ReportNativeTexture(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Resource* resource,
                         const char* tag, unsigned index)
{
    if (!device || !context || !resource) return;
    ID3D11Texture2D* texture = nullptr;
    if (FAILED(resource->QueryInterface(IID_PPV_ARGS(&texture))) || !texture) return;
    D3D11_TEXTURE2D_DESC desc = {};
    texture->GetDesc(&desc);
    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;
    ComPtr<ID3D11Texture2D> staging;
    if (FAILED(device->CreateTexture2D(&stagingDesc, nullptr, &staging)))
    {
        Blur2PCAutomation::LogF("DX11 stats %s %u texture=%p %ux%u fmt=%u (no staging)", tag, index, resource,
            desc.Width, desc.Height, static_cast<unsigned>(desc.Format));
        texture->Release();
        return;
    }
    context->CopyResource(staging.Get(), texture);
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)) && mapped.pData)
    {
        size_t pixelBytes = 4;
        if (desc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT) pixelBytes = 8;
        else if (desc.Format == DXGI_FORMAT_R8G8_UNORM || desc.Format == DXGI_FORMAT_R16_FLOAT) pixelBytes = 2;
        else if (desc.Format == DXGI_FORMAT_R8_UNORM) pixelBytes = 1;
        const U8* row = static_cast<const U8*>(mapped.pData) +
                        static_cast<size_t>(desc.Height / 2) * mapped.RowPitch;
        const size_t rowBytes = mapped.RowPitch;
        U8 bytes[8] = {};
        if (rowBytes >= 8)
        {
            size_t offset = static_cast<size_t>(desc.Width / 2) * pixelBytes;
            if (offset + 8 > rowBytes) offset = rowBytes - 8;
            memcpy(bytes, row + offset, 8);
        }
        U32 nonZero = 0;
        for (size_t i = 0; i < 8; ++i)
            if (bytes[i]) ++nonZero;
        Blur2PCAutomation::LogF("DX11 stats %s %u texture=%p %ux%u fmt=%u first8=%02x%02x%02x%02x%02x%02x%02x%02x nonZeroBytes=%u",
            tag, index, resource, desc.Width, desc.Height, static_cast<unsigned>(desc.Format),
            bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7], nonZero);
        context->Unmap(staging.Get(), 0);
    }
    texture->Release();
}
void C_D3D11GameDevice::ReportPixelConstants(U32 start, U32 count) const
{
    const float* data = State(const_cast<C_D3D11GameDevice*>(this)).pixelData;
    for (U32 reg = start; reg < start + count; reg += 2)
        Blur2PCAutomation::LogF("DX11 pixel constants c%u = %.4f %.4f %.4f %.4f | c%u = %.4f %.4f %.4f %.4f",
            reg, data[reg * 4], data[reg * 4 + 1], data[reg * 4 + 2], data[reg * 4 + 3],
            reg + 1, data[(reg + 1) * 4], data[(reg + 1) * 4 + 1], data[(reg + 1) * 4 + 2], data[(reg + 1) * 4 + 3]);
}
ID3D11Resource* C_D3D11GameDevice::GetBoundTexture(U32 slot) const
{
    return slot < E_Sampler_Count && m_Sampler[slot].m_pTexture ? m_Sampler[slot].m_pTexture->m_pd3dTexture : nullptr;
}
void C_D3D11GameDevice::ReportTextureStats(const char* tag, ID3D11Resource* resource, unsigned index)
{
    ID3D11DeviceContext* context = nullptr;
    m_pd3dDevice->GetImmediateContext(&context);
    ReportNativeTexture(m_pd3dDevice, context, resource, tag, index);
    if (context) context->Release();
}
void C_D3D11GameDevice::SetTexture(U32 slot, const C_Texture* texture)
{
    if (slot >= E_Sampler_Count) return;
    m_Sampler[slot].m_pTexture = texture;
    SetNativeTexture(slot, texture ? texture->m_pd3dTexture : nullptr);
}
// [Blur1PCDX11] Bounded opt-in GPU evidence for the recovered vehicle path.
// Read the actually bound constant buffers, not only the CPU parameter sources.
static void TraceVehicleShader(ID3D11Device* device, ID3D11DeviceContext* context,
                               const std::vector<U8>& code, bool vertex)
{
    const char* tracePart = getenv("BLUR_DX11_TRACE_VEHICLE_PART");
    if (tracePart && ((vertex && strstr(tracePart, "ps")) || (!vertex && strstr(tracePart, "vs")))) return;
    if (tracePart && strstr(tracePart, "raw"))
    {
        if (strstr(tracePart, "noop")) return;
        ComPtr<ID3D11Buffer> buffer;
        context->PSGetConstantBuffers(strstr(tracePart, "13") ? 13 : 0, 1, &buffer);
        return;
    }
    ComPtr<ID3D11ShaderReflection> reflection;
    if (code.empty() || FAILED(D3DReflect(code.data(), code.size(), IID_PPV_ARGS(&reflection)))) return;
    D3D11_SHADER_DESC shader = {};
    reflection->GetDesc(&shader);
    for (UINT i = 0; i < shader.BoundResources; ++i)
    {
        D3D11_SHADER_INPUT_BIND_DESC binding = {};
        reflection->GetResourceBindingDesc(i, &binding);
        if (tracePart && (strstr(tracePart, "constants") || strstr(tracePart, "descriptors") || strstr(tracePart, "copy")) && binding.Type != D3D_SIT_CBUFFER) continue;
        if (tracePart && strstr(tracePart, "textures") && binding.Type != D3D_SIT_TEXTURE) continue;
        if (binding.Type == D3D_SIT_CBUFFER)
        {
            ComPtr<ID3D11Buffer> buffer, staging;
            if (vertex) context->VSGetConstantBuffers(binding.BindPoint, 1, &buffer);
            else context->PSGetConstantBuffers(binding.BindPoint, 1, &buffer);
            if (!buffer) continue;
            D3D11_BUFFER_DESC desc = {};
            buffer->GetDesc(&desc);
            if (tracePart && strstr(tracePart, "descriptors")) continue;
            desc.Usage = D3D11_USAGE_STAGING;
            desc.BindFlags = desc.MiscFlags = 0;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            if (FAILED(device->CreateBuffer(&desc, nullptr, &staging))) continue;
            context->CopyResource(staging.Get(), buffer.Get());
            if (tracePart && strstr(tracePart, "copy")) continue;
            D3D11_MAPPED_SUBRESOURCE mapped = {};
            if (FAILED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped))) continue;
            auto* cb = reflection->GetConstantBufferByName(binding.Name);
            D3D11_SHADER_BUFFER_DESC cbDesc = {};
            cb->GetDesc(&cbDesc);
            for (UINT v = 0; v < cbDesc.Variables; ++v)
            {
                D3D11_SHADER_VARIABLE_DESC variable = {};
                cb->GetVariableByIndex(v)->GetDesc(&variable);
                if (!(variable.uFlags & D3D_SVF_USED) || variable.StartOffset + variable.Size > desc.ByteWidth) continue;
                if (getenv("BLUR_DX11_TRACE_SCENE") || getenv("BLUR_DX11_TRACE_VEHICLE_DUMP"))
                {
                    char path[MAX_PATH];
                    sprintf_s(path, "%s\\scene_%s_%s.bin", TraceDir(), vertex ? "VS" : "PS", variable.Name);
                    FILE* file = nullptr;
                    if (fopen_s(&file, path, "wb") == 0)
                    { fwrite(static_cast<const U8*>(mapped.pData) + variable.StartOffset, variable.Size, 1, file); fclose(file); }
                }
                float first[4] = {}, last[4] = {};
                const UINT copied = (std::min)(variable.Size, 16U);
                memcpy(first, static_cast<const U8*>(mapped.pData) + variable.StartOffset, copied);
                memcpy(last, static_cast<const U8*>(mapped.pData) + variable.StartOffset + variable.Size - copied, copied);
                Blur2PCAutomation::LogF("DX11 vehicle GPU %s %s bytes=%u first=%.5f %.5f %.5f %.5f last=%.5f %.5f %.5f %.5f",
                    vertex ? "VS" : "PS", variable.Name, variable.Size,
                    first[0], first[1], first[2], first[3], last[0], last[1], last[2], last[3]);
            }
            context->Unmap(staging.Get(), 0);
        }
        else if (binding.Type == D3D_SIT_SAMPLER)
        {
            ComPtr<ID3D11SamplerState> sampler;
            if (vertex) context->VSGetSamplers(binding.BindPoint, 1, &sampler);
            else context->PSGetSamplers(binding.BindPoint, 1, &sampler);
            D3D11_SAMPLER_DESC sd = {}; if (sampler) sampler->GetDesc(&sd);
            Blur2PCAutomation::LogF("DX11 scene sampler %s s%u filter=%u addr=%u/%u/%u lod=%f..%f bias=%f",
                binding.Name, binding.BindPoint, sd.Filter, sd.AddressU, sd.AddressV, sd.AddressW, sd.MinLOD, sd.MaxLOD, sd.MipLODBias);
        }
        else if (binding.Type == D3D_SIT_TEXTURE)
        {
            ComPtr<ID3D11ShaderResourceView> view;
            if (vertex) context->VSGetShaderResources(binding.BindPoint, 1, &view);
            else context->PSGetShaderResources(binding.BindPoint, 1, &view);
            D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
            ComPtr<ID3D11Resource> resource;
            if (view) { view->GetDesc(&desc); view->GetResource(&resource); }
            Blur2PCAutomation::LogF("DX11 vehicle GPU %s texture %s t%u view=%p resource=%p format=%u dimension=%u",
                vertex ? "VS" : "PS", binding.Name, binding.BindPoint, view.Get(), resource.Get(), desc.Format, desc.ViewDimension);
            if (getenv("BLUR_DX11_TRACE_VEHICLE_DUMP") && resource &&
                desc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE3D)
            {
                // [Blur1PCDX11] Preserve row/slice pitches for LUT and damage-volume evidence.
                ComPtr<ID3D11Texture3D> texture, staging;
                if (FAILED(resource.As(&texture))) continue;
                D3D11_TEXTURE3D_DESC td = {}; texture->GetDesc(&td);
                td.Usage = D3D11_USAGE_STAGING; td.BindFlags = td.MiscFlags = 0;
                td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                if (FAILED(device->CreateTexture3D(&td, nullptr, &staging))) continue;
                context->CopyResource(staging.Get(), texture.Get());
                const UINT mip = desc.Texture3D.MostDetailedMip;
                D3D11_MAPPED_SUBRESOURCE mapped = {};
                if (FAILED(context->Map(staging.Get(), mip, D3D11_MAP_READ, 0, &mapped))) continue;
                const UINT depth = (std::max)(1U, td.Depth >> mip);
                const UINT header[] = {(std::max)(1U, td.Width >> mip),
                    (std::max)(1U, td.Height >> mip), depth, static_cast<UINT>(td.Format),
                    mapped.RowPitch, mapped.DepthPitch};
                char path[MAX_PATH];
                sprintf_s(path, "%s\\scene_%s_volume.bin", TraceDir(), binding.Name);
                FILE* file = nullptr;
                if (fopen_s(&file, path, "wb") == 0)
                { fwrite(header, sizeof(header), 1, file); fwrite(mapped.pData, mapped.DepthPitch, depth, file); fclose(file); }
                context->Unmap(staging.Get(), mip);
            }
            if ((getenv("BLUR_DX11_TRACE_SCENE") || getenv("BLUR_DX11_TRACE_VEHICLE_DUMP")) && resource &&
                (desc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D || desc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURECUBE))
            {
                ComPtr<ID3D11Texture2D> texture, staging;
                if (FAILED(resource.As(&texture))) continue;
                D3D11_TEXTURE2D_DESC td = {}; texture->GetDesc(&td);
                td.Usage = D3D11_USAGE_STAGING; td.BindFlags = td.MiscFlags = 0; td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                if (FAILED(device->CreateTexture2D(&td, nullptr, &staging))) continue;
                context->CopyResource(staging.Get(), texture.Get());
                // [Blur1PCDX11] Read each face/mip once for the selected
                // draw; mip filtering can expose data absent from mip zero.
                for (UINT face = 0; face < td.ArraySize; ++face)
                for (UINT mip = 0; mip < td.MipLevels; ++mip)
                {
                    const UINT subresource = face * td.MipLevels + mip;
                    D3D11_MAPPED_SUBRESOURCE mapped = {};
                    if (FAILED(context->Map(staging.Get(), subresource, D3D11_MAP_READ, 0, &mapped))) continue;
                    const UINT width = (std::max)(1U, td.Width >> mip);
                    const UINT height = (std::max)(1U, td.Height >> mip);
                    const UINT header[] = {width, height, static_cast<UINT>(td.Format), mapped.RowPitch};
                    const bool bc = td.Format >= DXGI_FORMAT_BC1_TYPELESS && td.Format <= DXGI_FORMAT_BC3_UNORM_SRGB;
                    char path[MAX_PATH], suffix[64] = {};
                    if (td.ArraySize > 1) sprintf_s(suffix, "_face%u", face);
                    if (mip) { char level[32]; sprintf_s(level, "_mip%u", mip); strcat_s(suffix, level); }
                    sprintf_s(path, "%s\\scene_%s%s.bin", TraceDir(), binding.Name, suffix);
                    FILE* file = nullptr;
                    if (fopen_s(&file, path, "wb") == 0)
                    { fwrite(header, sizeof(header), 1, file); fwrite(mapped.pData, mapped.RowPitch, bc ? (height + 3) / 4 : height, file); fclose(file); }
                    context->Unmap(staging.Get(), subresource);
                }
            }
        }
    }
}
// [Blur1PCDX11] Opt-in capture of actual paint vertex inputs/outputs.
static void TraceVehicleVertices(ID3D11Device* device, ID3D11DeviceContext* context,
                                const std::vector<U8>& code, UINT indices, UINT start, INT base)
{
    if (!getenv("BLUR_DX11_TRACE_VEHICLE_VERTICES") || code.empty()) return;
    ComPtr<ID3D11GeometryShader> oldGS;
    context->GSGetShader(&oldGS, nullptr, nullptr);
    if (oldGS) return;
    ComPtr<ID3D11Buffer> vb, ib;
    UINT stride = 0, offset = 0, ibOffset = 0; DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    context->IAGetVertexBuffers(0, 1, &vb, &stride, &offset);
    context->IAGetIndexBuffer(&ib, &format, &ibOffset);
    if (!vb || !stride || base < 0) return;
    auto dump = [&](ID3D11Buffer* buffer, const char* name) {
        if (!buffer) return;
        D3D11_BUFFER_DESC desc = {}; buffer->GetDesc(&desc);
        desc.Usage = D3D11_USAGE_STAGING; desc.BindFlags = desc.MiscFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        ComPtr<ID3D11Buffer> staging;
        if (FAILED(device->CreateBuffer(&desc, nullptr, &staging))) return;
        context->CopyResource(staging.Get(), buffer);
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (FAILED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped))) return;
        char path[MAX_PATH]; sprintf_s(path, "%s\\paint_vertices_trace_%s.bin", TraceDir(), name);
        FILE* file = nullptr;
        if (fopen_s(&file, path, "wb") == 0) { fwrite(mapped.pData, 1, desc.ByteWidth, file); fclose(file); }
        context->Unmap(staging.Get(), 0);
    };
    dump(vb.Get(), "vb"); dump(ib.Get(), "ib");
    ComPtr<ID3D11Buffer> controls;
    UINT controlStride = 0, controlOffset = 0;
    context->IAGetVertexBuffers(3, 1, &controls, &controlStride, &controlOffset);
    dump(controls.Get(), "controls");
    Blur2PCAutomation::LogF("DX11 vehicle controls stride=%u offset=%u", controlStride, controlOffset);
    D3D11_BUFFER_DESC vbDesc = {}; vb->GetDesc(&vbDesc);
    const UINT vertices = (std::min)(2048U, (vbDesc.ByteWidth - offset) / stride - static_cast<UINT>(base));
    // [Blur1PCDX11] Preserve draw addressing even when the paint-only stream
    // output declaration cannot capture another material's signature.
    Blur2PCAutomation::LogF("DX11 vertex capture stride=%u offset=%u base=%d vertices=%u indices=%u start=%u ibOffset=%u format=%u bytes=%u",
        stride, offset, base, vertices, indices, start, ibOffset, format, vbDesc.ByteWidth);
    const D3D11_SO_DECLARATION_ENTRY output[] = {
        {0,"SV_Position",0,0,4,0},{0,"TEXCOORD",0,0,4,0},{0,"TEXCOORD",1,0,4,0},
        {0,"TEXCOORD",2,0,4,0},{0,"TEXCOORD",4,0,4,0}
    };
    const UINT soStride = 80;
    ComPtr<ID3D11GeometryShader> capture;
    HRESULT hr = device->CreateGeometryShaderWithStreamOutput(code.data(), code.size(), output, 5,
        &soStride, 1, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, &capture);
    if (FAILED(hr)) { Blur2PCAutomation::LogF("DX11 paint vertex SO failed %08x", hr); return; }
    D3D11_BUFFER_DESC desc = {}; desc.ByteWidth = vertices * soStride; desc.BindFlags = D3D11_BIND_STREAM_OUTPUT;
    ComPtr<ID3D11Buffer> so;
    if (!vertices || FAILED(device->CreateBuffer(&desc, nullptr, &so))) return;
    D3D11_PRIMITIVE_TOPOLOGY oldTopology;
    context->IAGetPrimitiveTopology(&oldTopology);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context->GSSetShader(capture.Get(), nullptr, 0);
    ID3D11Buffer* target = so.Get(); const UINT zero = 0;
    context->SOSetTargets(1, &target, &zero); context->Draw(vertices, base);
    context->SOSetTargets(0, nullptr, nullptr); context->GSSetShader(nullptr, nullptr, 0);
    context->IASetPrimitiveTopology(oldTopology);
    dump(so.Get(), "so");
    Blur2PCAutomation::LogF("DX11 paint vertex inputs stride=%u offset=%u base=%d vertices=%u indices=%u start=%u ibOffset=%u format=%u",
        stride, offset, base, vertices, indices, start, ibOffset, format);
}
void C_D3D11GameDevice::SetNativeTexture(U32 slot, ID3D11Resource* sampled)
{
    FlushNativeFX();
    if (slot >= E_Sampler_Count) return;
    auto context = Context(m_pd3dDevice); if (!context) return;
    if (sampled)
    {
        ID3D11RenderTargetView* targets[4] = {};
        context->OMGetRenderTargets(4, targets, nullptr);
        bool feedback = false;
        for (ID3D11RenderTargetView* target : targets)
        {
            if (!target) continue;
            ComPtr<ID3D11Resource> resource;
            target->GetResource(&resource);
            feedback |= resource.Get() == sampled;
            target->Release();
        }
        if (feedback)
        {
            // [Blur1PCDX11] D3D11 unbinds an RTV when the same resource is
            // assigned as an SRV. Snapshot the colour target before sampling.
            ComPtr<ID3D11Texture2D> source;
            if (SUCCEEDED(sampled->QueryInterface(IID_PPV_ARGS(&source))))
            {
                D3D11_TEXTURE2D_DESC desc = {};
                source->GetDesc(&desc);
                auto& copy = State(this).feedbackCopies[sampled];
                D3D11_TEXTURE2D_DESC old = {};
                if (copy) copy->GetDesc(&old);
                if (!copy || old.Width != desc.Width || old.Height != desc.Height ||
                    old.Format != desc.Format || old.MipLevels != desc.MipLevels ||
                    old.ArraySize != desc.ArraySize || old.SampleDesc.Count != desc.SampleDesc.Count)
                {
                    copy.Reset();
                    desc.Usage = D3D11_USAGE_DEFAULT;
                    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                    desc.CPUAccessFlags = 0;
                    desc.MiscFlags = 0;
                    m_pd3dDevice->CreateTexture2D(&desc, nullptr, &copy);
                }
                if (copy)
                {
                    context->CopyResource(copy.Get(), sampled);
                    sampled = copy.Get();
                }
            }
        }
    }
    ComPtr<ID3D11ShaderResourceView> view;
    if (sampled)
    {
        const HRESULT viewResult = CreateTextureView(m_pd3dDevice, sampled, m_Sampler[slot].m_Gamma == E_Bool_True, &view);
        if (FAILED(viewResult) && Blur2PCAutomation::Now() > 20.0)
        {
            static unsigned failures = 0;
            if (failures++ < 12)
                Blur2PCAutomation::LogF("DX11 sampler view failed slot=%u resource=%p result=%08x",
                    slot, sampled, static_cast<unsigned>(viewResult));
        }
    }
    NativeState& state = State(this);
    state.pixelViews[slot] = view;
    S32 target = static_cast<S32>(slot);
    if (!state.nativeFXActive)
        target = state.pixelTextureRegisters[slot];
    if (target < 0) return;
    ID3D11ShaderResourceView* raw = view.Get(); context->PSSetShaderResources(target, 1, &raw);
    D3DX11InvalidateBoundState(2u, 8u);
}
void C_D3D11GameDevice::SetVertexTexture(U32 slot, const C_Texture* texture)
{
    if (slot >= E_VertexSampler_Count) return;
    m_VertexSampler[slot].m_pTexture = texture;
    SetNativeVertexTexture(slot, texture ? texture->m_pd3dTexture : nullptr);
}
void C_D3D11GameDevice::SetNativeVertexTexture(U32 slot, ID3D11Resource* texture)
{
    FlushNativeFX();
    if (slot >= E_VertexSampler_Count) return;
    auto context = Context(m_pd3dDevice); if (!context) return;
    ComPtr<ID3D11ShaderResourceView> view;
    if (texture)
        CreateTextureView(m_pd3dDevice, texture, m_VertexSampler[slot].m_Gamma == E_Bool_True, &view);
    NativeState& state = State(this);
    state.vertexViews[slot] = view;
    S32 target = static_cast<S32>(slot);
    if (!state.nativeFXActive)
        target = state.vertexTextureRegisters[slot];
    if (target < 0) return;
    ID3D11ShaderResourceView* raw = view.Get(); context->VSSetShaderResources(target, 1, &raw);
    D3DX11InvalidateBoundState(1u, 8u);
}
void C_D3D11GameDevice::SetVertexShaderConstantF(U32 start, const F32* data, U32 count)
{ if (!data || start >= 256 || count > 256 - start) return; /* [Blur1PCDX11] UINT_MAX (missing parameter) wrapped start + count past the check */ memcpy(State(this).vertexData + start * 4, data, count * 4 * sizeof(F32)); State(this).vertexDirty = true; }
void C_D3D11GameDevice::SetPixelShaderConstantF(U32 start, const F32* data, U32 count)
{ if (!data || start >= 256 || count > 256 - start) return; /* [Blur1PCDX11] UINT_MAX (missing parameter) wrapped start + count past the check */ memcpy(State(this).pixelData + start * 4, data, count * 4 * sizeof(F32)); State(this).pixelDirty = true; }
void C_D3D11GameDevice::SetVertexShaderParameterF(C_VertexShaderParameter param, const F32* data, U32 count)
{ SetVertexShaderConstantF(param.m_RegisterIndex, data, count); }
void C_D3D11GameDevice::SetPixelShaderParameterF(C_PixelShaderParameter param, const F32* data)
{ SetPixelShaderConstantF(param.m_RegisterIndex, data, 1); }
void C_D3D11GameDevice::SetPixelShaderParameterArrayF(const C_PixelShaderParameterArray& param, const F32* data)
{ SetPixelShaderConstantF(param.m_RegisterIndex, data, param.m_RegisterCount); }
void C_D3D11GameDevice::SetPixelShaderParameters(const C_PixelShader* shader)
{ (void)shader; } // [Blur1PCDX11] constants are uploaded by the next legacy draw
void C_D3D11GameDevice::DrawPrimitive(E_PrimitiveType type, U32 start, U32 count)
{ FlushNativeFX(); auto context = Context(m_pd3dDevice); if (!context) return; SetTopology(this, context.Get(), Topology(type));
  SetStencilTopology(this, Topology(type));
  ApplyDirtyStates(this, context.Get());
  // [Blur1PCDX11] A native FX pass always binds its vertex shader; only legacy
  // draws can arrive without one.
  ComPtr<ID3D11VertexShader> bound;
  if (!State(this).nativeFXActive) context->VSGetShader(&bound, nullptr, nullptr);
  if (!State(this).nativeFXActive && !bound) { TraceMissingVertexShader(false, count, m_pVertexShader, State(this).nativeFXActive);
      static unsigned missing = 0; if (missing++ < 64)
      Blur2PCAutomation::LogF("DX11 draw without VS caller=%p module=%p lastFX=%s", _ReturnAddress(), GetModuleHandleW(nullptr), FX::NativeFX11LastError()); return; }
  { static unsigned trace = 0; if (trace++ < 18) {
      ComPtr<ID3D11PixelShader> ps; ComPtr<ID3D11InputLayout> layout;
      ComPtr<ID3D11RenderTargetView> rtv; ComPtr<ID3D11DepthStencilView> dsv;
      context->PSGetShader(&ps, nullptr, nullptr); context->IAGetInputLayout(&layout);
      context->OMGetRenderTargets(1, &rtv, &dsv);
      Blur2PCAutomation::LogF("DX11 Draw count=%u VS=%p PS=%p IA=%p RTV=%p DSV=%p caller=%p", count, bound.Get(), ps.Get(), layout.Get(), rtv.Get(), dsv.Get(), _ReturnAddress());
  } }
  { static unsigned lateTrace = 0; if (lateTrace < 20 && Blur2PCAutomation::Now() > 5.0 && lateTrace++ < 20) {
      ComPtr<ID3D11PixelShader> ps; ComPtr<ID3D11InputLayout> layout;
      ComPtr<ID3D11RenderTargetView> rtv; ComPtr<ID3D11Buffer> vb, ib;
      UINT stride = 0, offset = 0, indexOffset = 0; DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN;
      context->PSGetShader(&ps, nullptr, nullptr); context->IAGetInputLayout(&layout);
      context->OMGetRenderTargets(1, &rtv, nullptr);
      context->IAGetVertexBuffers(0, 1, &vb, &stride, &offset);
      context->IAGetIndexBuffer(&ib, &indexFormat, &indexOffset);
      Blur2PCAutomation::LogF("DX11 late Draw count=%u VS=%p PS=%p IA=%p RTV=%p VB=%p stride=%u IB=%p write=%u blend=%u",
          count, bound.Get(), ps.Get(), layout.Get(), rtv.Get(), vb.Get(), stride, ib.Get(),
          State(this).blend.RenderTarget[0].RenderTargetWriteMask,
          State(this).blend.RenderTarget[0].BlendEnable);
  } }
  if (!State(this).nativeFXActive) { BindConstants(this, true); BindConstants(this, false); }
  else BindHDROutputConstants(); // [Blur2PC][HDR] Bind after the FX11 pass flushes its own buffers.
  BindAlphaTest(this);
  // [Blur1PCDX11] Inspect actual built-in bindings at one requested trace draw.
  // This includes deferred light passes, which do not use the FX material path.
  g_dx11TraceSelectedCapture = false;
  if (g_dx11TraceFrame && !State(this).nativeFXActive)
  {
      static const char* selected = getenv("BLUR_DX11_TRACE_BOUND_DRAW");
      static const char* variableName = getenv("BLUR_DX11_TRACE_BOUND_VARIABLE");
      static bool tracedVariable = false;
      bool selectedVariable = false;
      if (variableName && !tracedVariable && m_pPixelShader)
      {
          const auto& bytes = m_pPixelShader->m_Bytecode;
          ComPtr<ID3D11ShaderReflection> reflection;
          D3D11_SHADER_VARIABLE_DESC variable = {};
          if (!bytes.empty() && SUCCEEDED(D3DReflect(bytes.data(), bytes.size(), IID_PPV_ARGS(&reflection))) &&
              SUCCEEDED(reflection->GetVariableByName(variableName)->GetDesc(&variable)) &&
              (variable.uFlags & D3D_SVF_USED))
              selectedVariable = true;
      }
      if (selectedVariable || (selected && g_dx11TraceDraw == strtoul(selected, nullptr, 10)))
      {
          tracedVariable |= selectedVariable;
          g_dx11TraceSelectedCapture = selectedVariable;
          Blur2PCAutomation::LogF("DX11 selected built-in bindings draw=%u", g_dx11TraceDraw);
          if (selectedVariable)
          {
              if (strcmp(variableName, "g_mColour") == 0)
              {
                  const auto* correction = Debug::FindByName("Graphics.Render.PostProcess.HDR.ColourCorrection");
                  if (correction && correction->Type() == Debug::kVtBool)
                      Blur2PCAutomation::LogF("DX11 colour correction enabled=%u default=%u",
                          static_cast<const Debug::Bool*>(correction)->Value(),
                          static_cast<const Debug::Bool*>(correction)->Default());
              }
              const auto parameter = m_pPixelShader->GetParamFromName(variableName);
              Blur2PCAutomation::LogF("DX11 selected built-in parameter %s register=%u count=%u",
                  variableName, parameter.m_RegisterIndex, parameter.m_RegisterCount);
              if (parameter.m_RegisterCount && parameter.m_RegisterIndex < 255 &&
                  parameter.m_RegisterCount <= 255 - parameter.m_RegisterIndex)
                  ReportPixelConstants(parameter.m_RegisterIndex, parameter.m_RegisterCount);
          }
          if (m_pVertexShader) TraceVehicleShader(m_pd3dDevice, context.Get(), m_pVertexShader->m_Bytecode, true);
          if (m_pPixelShader) TraceVehicleShader(m_pd3dDevice, context.Get(), m_pPixelShader->m_Bytecode, false);
      }
  }
  if (!TryDraw(context.Get(), count, start, false))
  {
      static unsigned traced = 0;
      if (traced++ < 24) Blur2PCAutomation::LogF("DX11 draw rejected VS=%p PS=%p count=%u", m_pVertexShader, m_pPixelShader, count);
  }
  TraceDraw(m_pd3dDevice, context.Get(), State(this), count, false); }
void C_D3D11GameDevice::DrawIndexedPrimitive(E_PrimitiveType type, U32 base, U32 start, U32 count)
{ FlushNativeFX(); auto context = Context(m_pd3dDevice); if (!context) return; SetTopology(this, context.Get(), Topology(type));
  SetStencilTopology(this, Topology(type));
  ApplyDirtyStates(this, context.Get());
  // [Blur1PCDX11] Inspect one actual scene material draw after loading.
  static const bool traceScene = getenv("BLUR_DX11_TRACE_SCENE") != nullptr;
  if (State(this).nativeFXActive && traceScene &&
      Blur2PCAutomation::Now() > 5 &&
      strncmp(FX::NativeFX11CurrentPass(), "Default / MaterialLights", 24) == 0)
  {
      static bool tracedScene = false;
      if (!tracedScene)
      {
          tracedScene = true;
          Blur2PCAutomation::LogF("DX11 scene GPU draw %s base=%u start=%u count=%u", FX::NativeFX11CurrentPass(), base, start, count);
          for (int stage = 0; stage < 2; ++stage)
          {
              const auto* bytes = stage ? (m_pPixelShader ? &m_pPixelShader->m_Bytecode : nullptr) :
                  (m_pVertexShader ? &m_pVertexShader->m_Bytecode : nullptr);
              ComPtr<ID3DBlob> assembly;
              if (bytes && SUCCEEDED(D3DDisassemble(bytes->data(), bytes->size(), 0, nullptr, &assembly)))
              {
                  FILE* file = nullptr;
                  if (fopen_s(&file, stage ? TracePath("scene_bound_ps.txt") :
                      TracePath("scene_bound_vs.txt"), "wb") == 0)
                  { fwrite(assembly->GetBufferPointer(), 1, assembly->GetBufferSize(), file); fclose(file); }
              }
          }
          if (m_pVertexShader) TraceVehicleShader(m_pd3dDevice, context.Get(), m_pVertexShader->m_Bytecode, true);
          if (m_pPixelShader) TraceVehicleShader(m_pd3dDevice, context.Get(), m_pPixelShader->m_Bytecode, false);
          if (m_pVertexDeclaration) for (U32 e = 0; e < m_pVertexDeclaration->m_ElementCount; ++e)
          {
              const auto& element = m_pVertexDeclaration->m_Elements[e];
              Blur2PCAutomation::LogF("DX11 scene IA usage=%u index=%u type=%u stream=%u offset=%u",
                  element.Usage, element.UsageIndex, element.Type, element.Stream, element.Offset);
          }
          for (UINT slot = 0; slot < 2; ++slot)
          {
              ComPtr<ID3D11Buffer> buffer, staging;
              UINT stride = 0, offset = 0; DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
              if (slot) context->IAGetIndexBuffer(&buffer, &format, &offset);
              else context->IAGetVertexBuffers(0, 1, &buffer, &stride, &offset);
              if (!buffer) continue;
              D3D11_BUFFER_DESC desc = {}; buffer->GetDesc(&desc);
              desc.Usage = D3D11_USAGE_STAGING; desc.BindFlags = desc.MiscFlags = 0; desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
              if (FAILED(m_pd3dDevice->CreateBuffer(&desc, nullptr, &staging))) continue;
              context->CopyResource(staging.Get(), buffer.Get());
              D3D11_MAPPED_SUBRESOURCE mapped = {};
              if (FAILED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped))) continue;
              FILE* file = nullptr;
              if (fopen_s(&file, slot ? TracePath("scene_ib.bin") :
                  TracePath("scene_vb.bin"), "wb") == 0)
              { fwrite(mapped.pData, 1, desc.ByteWidth, file); fclose(file); }
              context->Unmap(staging.Get(), 0);
              Blur2PCAutomation::LogF("DX11 scene buffer slot=%u stride=%u offset=%u format=%u bytes=%u", slot, stride, offset, format, desc.ByteWidth);
          }
      }
  }
  // [Blur1PCDX11] Select one material for the existing bounded GPU-input trace.
  bool traceVehicleAfter = false;
  static const char* traceMaterial = getenv("BLUR_DX11_TRACE_VEHICLE_EFFECT");
  if (!traceMaterial) traceMaterial = "veh_paint / Material";
  static const bool traceVehicle = getenv("BLUR_DX11_TRACE_VEHICLE") != nullptr;
  if (State(this).nativeFXActive && traceVehicle &&
      (!getenv("BLUR_DX11_TRACE_VEHICLE_FRAME") || g_dx11TraceFrame) &&
      (!getenv("BLUR_DX11_TRACE_VEHICLE_DRAW") ||
       g_dx11TraceDraw == strtoul(getenv("BLUR_DX11_TRACE_VEHICLE_DRAW"), nullptr, 10)) &&
      (!getenv("BLUR_DX11_TRACE_VEHICLE_COUNT") ||
       count == strtoul(getenv("BLUR_DX11_TRACE_VEHICLE_COUNT"), nullptr, 10)) &&
      (!getenv("BLUR_DX11_TRACE_VEHICLE_FILE") || strstr(FX::NativeFX11CurrentPass(), getenv("BLUR_DX11_TRACE_VEHICLE_FILE"))) &&
      Blur2PCAutomation::Now() > 5 && strncmp(FX::NativeFX11CurrentPass(), traceMaterial, strlen(traceMaterial)) == 0 &&
      (!strstr(FX::NativeFX11CurrentPass(), "Prepass") || strstr(traceMaterial, "Prepass")) &&
      (!strstr(FX::NativeFX11CurrentPass(), "LOD") || strstr(traceMaterial, "LOD")))
  {
      static bool tracedVehicle = false;
      if (!tracedVehicle)
      {
          tracedVehicle = true;
          if (getenv("BLUR_DX11_TRACE_VEHICLE_VERTICES")) g_dx11TraceSelectedCapture = true;
          Blur2PCAutomation::LogF("DX11 vehicle GPU draw %s", FX::NativeFX11CurrentPass());
          traceVehicleAfter = getenv("BLUR_DX11_TRACE_VEHICLE_AFTER") != nullptr;
          if (!traceVehicleAfter)
          {
              if (m_pVertexShader) TraceVehicleVertices(m_pd3dDevice, context.Get(), m_pVertexShader->m_Bytecode, count, start, base);
              if (m_pVertexShader) TraceVehicleShader(m_pd3dDevice, context.Get(), m_pVertexShader->m_Bytecode, true);
              if (m_pPixelShader) TraceVehicleShader(m_pd3dDevice, context.Get(), m_pPixelShader->m_Bytecode, false);
          }
          if (m_pVertexDeclaration) for (U32 e = 0; e < m_pVertexDeclaration->m_ElementCount; ++e)
          {
              const auto& element = m_pVertexDeclaration->m_Elements[e];
              Blur2PCAutomation::LogF("DX11 vehicle IA usage=%u index=%u type=%u stream=%u offset=%u",
                  element.Usage, element.UsageIndex, element.Type, element.Stream, element.Offset);
          }
      }
  }
  // [Blur1PCDX11] A native FX pass always binds its vertex shader; only legacy
  // draws can arrive without one.
  ComPtr<ID3D11VertexShader> bound;
  if (!State(this).nativeFXActive) context->VSGetShader(&bound, nullptr, nullptr);
  if (!State(this).nativeFXActive && !bound) { TraceMissingVertexShader(true, count, m_pVertexShader, State(this).nativeFXActive);
      static unsigned missing = 0; if (missing++ < 64)
      Blur2PCAutomation::LogF("DX11 indexed draw without VS caller=%p module=%p lastFX=%s", _ReturnAddress(), GetModuleHandleW(nullptr), FX::NativeFX11LastError()); return; }
  { static unsigned trace = 0; if (trace++ < 18) {
      ComPtr<ID3D11PixelShader> ps; ComPtr<ID3D11InputLayout> layout;
      ComPtr<ID3D11RenderTargetView> rtv; ComPtr<ID3D11DepthStencilView> dsv;
      context->PSGetShader(&ps, nullptr, nullptr); context->IAGetInputLayout(&layout);
      context->OMGetRenderTargets(1, &rtv, &dsv);
      ComPtr<ID3D11Buffer> vb; ComPtr<ID3D11Buffer> ib;
      UINT stride = 0, offset = 0, ibOffset = 0; DXGI_FORMAT ibFormat = DXGI_FORMAT_UNKNOWN;
      context->IAGetVertexBuffers(0, 1, &vb, &stride, &offset);
      context->IAGetIndexBuffer(&ib, &ibFormat, &ibOffset);
      D3D11_VIEWPORT viewport = {}; UINT viewportCount = 1;
      context->RSGetViewports(&viewportCount, &viewport);
      ComPtr<ID3D11DepthStencilState> ds; UINT stencilRef = 0;
      context->OMGetDepthStencilState(&ds, &stencilRef);
      D3D11_DEPTH_STENCIL_DESC dd = {}; if (ds) ds->GetDesc(&dd);
      Blur2PCAutomation::LogF("DX11 DrawIndexed count=%u VS=%p PS=%p IA=%p RTV=%p DSV=%p VB=%p stride=%u IB=%p VP=%.0fx%.0f z=%.1f..%.1f depth=%u/%u caller=%p", count, bound.Get(), ps.Get(), layout.Get(), rtv.Get(), dsv.Get(), vb.Get(), stride, ib.Get(), viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth, dd.DepthEnable, dd.DepthFunc, _ReturnAddress());
  } }
  { static unsigned lateTrace = 0; if (lateTrace < 20 && Blur2PCAutomation::Now() > 5.0 && lateTrace++ < 20) {
      ComPtr<ID3D11PixelShader> ps; ComPtr<ID3D11InputLayout> layout;
      ComPtr<ID3D11RenderTargetView> rtv; ComPtr<ID3D11Buffer> vb, ib;
      UINT stride = 0, offset = 0, indexOffset = 0; DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN;
      context->PSGetShader(&ps, nullptr, nullptr); context->IAGetInputLayout(&layout);
      context->OMGetRenderTargets(1, &rtv, nullptr);
      context->IAGetVertexBuffers(0, 1, &vb, &stride, &offset);
      context->IAGetIndexBuffer(&ib, &indexFormat, &indexOffset);
      Blur2PCAutomation::LogF("DX11 late Indexed count=%u VS=%p PS=%p IA=%p RTV=%p VB=%p stride=%u IB=%p write=%u blend=%u decl=%p shader=%p reason=%s",
          count, bound.Get(), ps.Get(), layout.Get(), rtv.Get(), vb.Get(), stride, ib.Get(),
          State(this).blend.RenderTarget[0].RenderTargetWriteMask,
          State(this).blend.RenderTarget[0].BlendEnable, m_pVertexDeclaration,
          m_pVertexShader, LastGameInputLayoutDiagnostic());
  } }
  if (!State(this).nativeFXActive) { BindConstants(this, true); BindConstants(this, false); }
  else BindHDROutputConstants(); // [Blur2PC][HDR] Bind after the FX11 pass flushes its own buffers.
  BindAlphaTest(this);
  static bool testPSChecked = false;
  static ComPtr<ID3D11PixelShader> testPS;
  static ComPtr<ID3D11VertexShader> testVS;
  if (!testPSChecked)
  {
      testPSChecked = true;
      char enabled[2] = {};
      if (GetEnvironmentVariableA("BLUR_DX11_TEST_PS", enabled, sizeof(enabled)) && enabled[0] == '1')
      {
          const char source[] = "float4 main() : SV_Target { return float4(1,0,1,1); }";
          ComPtr<ID3DBlob> code;
          if (SUCCEEDED(D3DCompile(source, sizeof(source)-1, nullptr, nullptr, nullptr,
              "main", "ps_5_0", 0, 0, &code, nullptr)))
              m_pd3dDevice->CreatePixelShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &testPS);
          const char vertexSource[] = "float4 main(float2 p : POSITION) : SV_Position { return float4(p / 32767.0 * 2.0 - 1.0, 0, 1); }";
          code.Reset();
          if (SUCCEEDED(D3DCompile(vertexSource, sizeof(vertexSource)-1, nullptr, nullptr, nullptr,
              "main", "vs_5_0", 0, 0, &code, nullptr)))
              m_pd3dDevice->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &testVS);
      }
  }
  ComPtr<ID3D11PixelShader> original;
  if (testPS) { context->PSGetShader(&original, nullptr, nullptr); context->PSSetShader(testPS.Get(), nullptr, 0); }
  ComPtr<ID3D11RasterizerState> oldRaster;
  ComPtr<ID3D11DepthStencilState> oldDepth;
  ComPtr<ID3D11BlendState> oldBlend;
  UINT oldStencil = 0, oldSampleMask = 0;
  float oldBlendFactor[4] = {};
  if (testPS)
  {
      context->RSGetState(&oldRaster);
      context->OMGetDepthStencilState(&oldDepth, &oldStencil);
      context->OMGetBlendState(&oldBlend, oldBlendFactor, &oldSampleMask);
      D3D11_RASTERIZER_DESC raster = State(this).raster;
      raster.CullMode = D3D11_CULL_NONE;
      ComPtr<ID3D11RasterizerState> noCull;
      if (SUCCEEDED(m_pd3dDevice->CreateRasterizerState(&raster, &noCull))) context->RSSetState(noCull.Get());
      D3D11_DEPTH_STENCIL_DESC depth = State(this).depth;
      depth.DepthEnable = FALSE; depth.StencilEnable = FALSE;
      ComPtr<ID3D11DepthStencilState> noDepth;
      if (SUCCEEDED(m_pd3dDevice->CreateDepthStencilState(&depth, &noDepth))) context->OMSetDepthStencilState(noDepth.Get(), 0);
      D3D11_BLEND_DESC blend = State(this).blend;
      blend.RenderTarget[0].BlendEnable = FALSE;
      blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
      ComPtr<ID3D11BlendState> allWrite;
      if (SUCCEEDED(m_pd3dDevice->CreateBlendState(&blend, &allWrite)))
      { const float factor[4] = {1,1,1,1}; context->OMSetBlendState(allWrite.Get(), factor, 0xffffffff); }
  }
  if (!TryDraw(context.Get(), count, start, true, base))
  {
      static unsigned traced = 0;
      if (traced++ < 24) Blur2PCAutomation::LogF("DX11 indexed draw rejected VS=%p PS=%p count=%u", m_pVertexShader, m_pPixelShader, count);
  }
  TraceDraw(m_pd3dDevice, context.Get(), State(this), count, true);
  if (traceVehicleAfter)
  {
      if (m_pVertexShader) TraceVehicleVertices(m_pd3dDevice, context.Get(), m_pVertexShader->m_Bytecode, count, start, base);
      if (m_pVertexShader) TraceVehicleShader(m_pd3dDevice, context.Get(), m_pVertexShader->m_Bytecode, true);
      if (m_pPixelShader) TraceVehicleShader(m_pd3dDevice, context.Get(), m_pPixelShader->m_Bytecode, false);
  }
  if (!testPS && count == 1647)
  {
      static bool nativeCaptured = false;
      if (!nativeCaptured)
      {
          nativeCaptured = true;
          ComPtr<ID3D11RenderTargetView> rtv;
          context->OMGetRenderTargets(1, &rtv, nullptr);
          ComPtr<ID3D11Resource> resource;
          if (rtv) rtv->GetResource(&resource);
          ComPtr<ID3D11Texture2D> texture;
          if (resource) resource.As(&texture);
          if (texture) TakeD3D11BackBufferScreenshot(TracePath("draw_native.tga"), texture.Get());
      }
  }
  if (testPS)
  {
      static bool captured = false;
      if (!captured && count == 1647)
      {
          captured = true;
          ComPtr<ID3D11Buffer> vb, ib;
          UINT stride = 0, vertexOffset = 0, indexOffset = 0;
          DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN;
          context->IAGetVertexBuffers(0, 1, &vb, &stride, &vertexOffset);
          context->IAGetIndexBuffer(&ib, &indexFormat, &indexOffset);
          auto readBuffer = [&](ID3D11Buffer* source, const char* label)
          {
              if (!source) return;
              D3D11_BUFFER_DESC desc = {};
              source->GetDesc(&desc);
              D3D11_BUFFER_DESC stagingDesc = desc;
              stagingDesc.Usage = D3D11_USAGE_STAGING;
              stagingDesc.BindFlags = 0;
              stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
              stagingDesc.MiscFlags = 0;
              ComPtr<ID3D11Buffer> staging;
              if (FAILED(m_pd3dDevice->CreateBuffer(&stagingDesc, nullptr, &staging))) return;
              context->CopyResource(staging.Get(), source);
              D3D11_MAPPED_SUBRESOURCE mapped = {};
              if (FAILED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped))) return;
              const U8* bytes = static_cast<const U8*>(mapped.pData);
              char path[MAX_PATH] = {};
              sprintf_s(path, "%s\\probe.%s", TraceDir(), label);
              FILE* dump = nullptr;
              if (fopen_s(&dump, path, "wb") == 0)
              { fwrite(bytes, 1, desc.ByteWidth, dump); fclose(dump); }
              if (desc.ByteWidth >= 24)
              {
                  const float* values = reinterpret_cast<const float*>(bytes);
                  const U16* indices = reinterpret_cast<const U16*>(bytes);
                  Blur2PCAutomation::LogF("DX11 probe %s bytes=%u stride=%u offsets=%u/%u fmt=%u floats=%.3f %.3f %.3f %.3f %.3f %.3f indices=%u %u %u %u %u %u",
                      label, desc.ByteWidth, stride, vertexOffset, indexOffset, indexFormat,
                      values[0], values[1], values[2], values[3], values[4], values[5],
                      indices[0], indices[1], indices[2], indices[3], indices[4], indices[5]);
              }
              context->Unmap(staging.Get(), 0);
          };
          readBuffer(vb.Get(), "VB");
          readBuffer(ib.Get(), "IB");
          const float* constants = State(this).vertexData;
          for (unsigned slot : {32U, 33U, 34U, 35U, 36U, 37U, 39U, 40U, 42U})
              Blur2PCAutomation::LogF("DX11 probe c%u = %.3f %.3f %.3f %.3f",
                  slot, constants[slot*4], constants[slot*4+1], constants[slot*4+2], constants[slot*4+3]);
          Blur2PCAutomation::LogF("DX11 probe nativeFXActive=%u VSobject=%p boundVS=%p",
              State(this).nativeFXActive, m_pVertexShader,
              bound.Get());
          if (m_pVertexDeclaration)
          {
              for (U32 element = 0; element < m_pVertexDeclaration->m_ElementCount; ++element)
              {
                  const D3DVERTEXELEMENT9& e = m_pVertexDeclaration->m_Elements[element];
                  Blur2PCAutomation::LogF("DX11 probe element %u stream=%u offset=%u type=%u usage=%u index=%u",
                      element, e.Stream, e.Offset, e.Type, e.Usage, e.UsageIndex);
              }
          }
          if (m_pVertexShader && !m_pVertexShader->m_Bytecode.empty())
          {
              ComPtr<ID3DBlob> listing;
              if (SUCCEEDED(D3DDisassemble(m_pVertexShader->m_Bytecode.data(),
                  m_pVertexShader->m_Bytecode.size(), 0, nullptr, &listing)))
              {
                  FILE* shaderDump = nullptr;
                  if (fopen_s(&shaderDump, TracePath("probe.vs.txt"), "wb") == 0)
                  { fwrite(listing->GetBufferPointer(), 1, listing->GetBufferSize(), shaderDump); fclose(shaderDump); }
              }
          }
          ComPtr<ID3D11VertexShader> originalVS;
          ComPtr<ID3D11InputLayout> originalLayout;
          context->VSGetShader(&originalVS, nullptr, nullptr);
          context->IAGetInputLayout(&originalLayout);
          context->VSSetShader(testVS.Get(), nullptr, 0);
          context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
          State(this).topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
          context->DrawIndexed(count, start, base);
          context->VSSetShader(originalVS.Get(), nullptr, 0);
          context->IASetInputLayout(originalLayout.Get());
          ComPtr<ID3D11RenderTargetView> rtv;
          context->OMGetRenderTargets(1, &rtv, nullptr);
          ComPtr<ID3D11Resource> resource;
          if (rtv) rtv->GetResource(&resource);
          ComPtr<ID3D11Texture2D> texture;
          if (resource) resource.As(&texture);
          if (texture) TakeD3D11BackBufferScreenshot(TracePath("draw.tga"), texture.Get());
      }
      context->RSSetState(oldRaster.Get());
      context->OMSetDepthStencilState(oldDepth.Get(), oldStencil);
      context->OMSetBlendState(oldBlend.Get(), oldBlendFactor, oldSampleMask);
  }
  if (testPS) { context->PSSetShader(original.Get(), nullptr, 0); D3DX11InvalidateBoundState(3u, 1u); }
}
void C_D3D11GameDevice::SetNativeFXActive(bool active) { State(this).nativeFXActive = active; }
void C_D3D11GameDevice::DrawIndexedPrimitive(E_PrimitiveType type, U32 base, U32 minIndex, U32 vertexCount, U32 start, U32 count)
{ DrawIndexedPrimitive(type, base, start, count); }
void C_D3D11GameDevice::SetViewport(const C_Viewport& viewport)
{
    m_Viewport = viewport; auto context = Context(m_pd3dDevice); if (!context) return;
    D3D11_VIEWPORT native = {};
    native.TopLeftX = static_cast<F32>(viewport.GetX()); native.TopLeftY = static_cast<F32>(viewport.GetY());
    native.Width = static_cast<F32>(viewport.GetW()); native.Height = static_cast<F32>(viewport.GetH());
    native.MinDepth = viewport.GetMinZ(); native.MaxDepth = viewport.GetMaxZ();
    context->RSSetViewports(1, &native);
}
void C_D3D11GameDevice::SetColourDepthStencilSurface(const C_Surface* colour, const C_Surface* depth)
{
    {
        const C_Surface* backSurface = GetBackBuffer().GetColourSurface();
        if (colour == backSurface)
        {
            static std::map<const void*, unsigned> callers;
            const void* caller = _ReturnAddress();
            if (callers[caller]++ % 60 == 0)
                Blur2PCAutomation::LogF("DX11 back buffer bound caller=%p count=%u t=%.1f", caller, callers[caller], Blur2PCAutomation::Now());
        }
    }
    if (g_dx11TraceFrame)
        Blur2PCAutomation::LogF("DX11 trace bind colour=%p native=%p depth=%p back=%p caller=%p", colour,
            colour ? colour->m_pd3dSurface : nullptr, depth, GetBackBuffer().GetColourSurface(), _ReturnAddress());
    m_RenderTarget.m_pColourSurface = colour; m_RenderTarget.m_pDepthStencilSurface = depth;
    NativeState& state = State(this);
    state.colourSurfaces[0] = colour;
    for (UINT slot = 1; slot < 4; ++slot) state.colourSurfaces[slot] = nullptr;
    state.depthSurface = depth;
    BindRenderTargets(this);
}
void C_D3D11GameDevice::SetColourDepthStencilSurface()
{ SetColourDepthStencilSurface(GetBackBuffer().GetColourSurface(), GetBackBuffer().GetDepthStencilSurface()); }
void C_D3D11GameDevice::SetColourSurface()
{ SetColourDepthStencilSurface(GetBackBuffer().GetColourSurface(), m_RenderTarget.m_pDepthStencilSurface); }
void C_D3D11GameDevice::SetColourSurface(const C_Surface* surface)
{
    SetColourDepthStencilSurface(surface, m_RenderTarget.m_pDepthStencilSurface);
}
void C_D3D11GameDevice::SetColourSurface(U32 index, const C_Surface* surface)
{
    if (index >= 4) return;
    if (index == 0) { SetColourSurface(surface); return; }
    State(this).colourSurfaces[index] = surface;
    BindRenderTargets(this);
}
void C_D3D11GameDevice::SetDepthStencilSurface()
{ SetDepthStencilSurface(GetBackBuffer().GetDepthStencilSurface()); }
void C_D3D11GameDevice::SetDepthStencilSurface(const C_Surface* surface)
{
    m_RenderTarget.m_pDepthStencilSurface = surface;
    State(this).depthSurface = surface;
    BindRenderTargets(this);
}
void C_D3D11GameDevice::Clear(U32 flags)
{ Clear(flags, m_ColourClear, GetDepthClear(), m_StencilClear); }
void C_D3D11GameDevice::Clear(U32 flags, Maths::RGBA colour)
{ Clear(flags, colour, GetDepthClear(), m_StencilClear); }
void C_D3D11GameDevice::Clear(U32 flags, Maths::RGBA colour, F32 depth, U8 stencil)
{
    auto context = Context(m_pd3dDevice); if (!context) return;
    ID3D11RenderTargetView* rtv[4] = {};
    ComPtr<ID3D11DepthStencilView> dsv;
    context->OMGetRenderTargets(4, rtv, &dsv);
    if (flags & E_Clear_Colour)
    {
        float rgba[4] = {colour.R()/255.f, colour.G()/255.f, colour.B()/255.f, colour.A()/255.f};
        for (auto* target : rtv) if (target) context->ClearRenderTargetView(target, rgba);
    }
    for (auto* target : rtv) if (target) target->Release();
    if ((flags & (E_Clear_Depth | E_Clear_Stencil)) && dsv)
    { UINT native = (flags & E_Clear_Depth ? D3D11_CLEAR_DEPTH : 0) | (flags & E_Clear_Stencil ? D3D11_CLEAR_STENCIL : 0); context->ClearDepthStencilView(dsv.Get(), native, depth, stencil); }
}
C_BackBuffer::C_BackBuffer() : m_Width(0), m_Height(0), m_ColourFormat(E_Format_Unknown),
    m_DepthStencilFormat(E_Format_Unknown), m_Multisample(E_Multisample_None) {}
C_BackBuffer::~C_BackBuffer() {}
C_FrontBuffer::C_FrontBuffer() : m_Width(0), m_Height(0), m_Format(E_Format_Unknown) {}
C_FrontBuffer::~C_FrontBuffer() {}
C_RenderTarget::C_RenderTarget() : m_pColourSurface(nullptr), m_pDepthStencilSurface(nullptr) {}
C_RenderTarget::~C_RenderTarget() {}

void C_D3D11GameDevice::SetAlphaTestEnable(E_Bool value)
{ m_AlphaTestEnable = value; const UINT enabled = value == E_Bool_True;
  if (State(this).alpha.enabled != enabled) { State(this).alpha.enabled = enabled; State(this).alphaDirty = true; } }
void C_D3D11GameDevice::SetAlphaTestFunc(E_CmpFunc value)
{ m_AlphaTestFunc = value; const UINT function = Compare(value);
  if (State(this).alpha.function != function) { State(this).alpha.function = function; State(this).alphaDirty = true; } }
void C_D3D11GameDevice::SetAlphaTestRef(U32 value)
{ m_AlphaTestRef = value; const F32 reference = static_cast<F32>(value & 255) * (1.0f / 255.0f);
  if (State(this).alpha.reference != reference) { State(this).alpha.reference = reference; State(this).alphaDirty = true; } }
void C_D3D11GameDevice::SetAlphaToMaskEnable(E_Bool value)
{ m_AlphaToMaskEnable = value; State(this).blend.AlphaToCoverageEnable = value == E_Bool_True; ApplyBlend(this); }
void C_D3D11GameDevice::SetBlend(E_Blend src, E_Blend dst, E_BlendOp op,
    E_Blend srcAlpha, E_Blend dstAlpha, E_BlendOp opAlpha)
{
    m_BlendSrc = src; m_BlendDst = dst; m_BlendOp = op;
    m_BlendSrcAlpha = srcAlpha; m_BlendDstAlpha = dstAlpha; m_BlendOpAlpha = opAlpha;
    auto& rt = State(this).blend.RenderTarget[0];
    rt.SrcBlend = Blend(src); rt.DestBlend = Blend(dst); rt.BlendOp = BlendOp(op);
    rt.SrcBlendAlpha = AlphaBlendFactor(Blend(srcAlpha)); rt.DestBlendAlpha = AlphaBlendFactor(Blend(dstAlpha)); rt.BlendOpAlpha = BlendOp(opAlpha);
    ApplyBlend(this);
}
void C_D3D11GameDevice::SetBlendDst(E_Blend value)
{ m_BlendDst = value; State(this).blend.RenderTarget[0].DestBlend = Blend(value); ApplyBlend(this); }
void C_D3D11GameDevice::SetBlendDstAlpha(E_Blend value)
{ m_BlendDstAlpha = value; State(this).blend.RenderTarget[0].DestBlendAlpha = AlphaBlendFactor(Blend(value)); ApplyBlend(this); }
void C_D3D11GameDevice::SetBlendEnable(E_Bool value)
{ m_BlendEnable = value; State(this).blend.RenderTarget[0].BlendEnable = value == E_Bool_True; ApplyBlend(this); }
void C_D3D11GameDevice::SetBlendOp(E_BlendOp value)
{ m_BlendOp = value; State(this).blend.RenderTarget[0].BlendOp = BlendOp(value); ApplyBlend(this); }
void C_D3D11GameDevice::SetBlendOpAlpha(E_BlendOp value)
{ m_BlendOpAlpha = value; State(this).blend.RenderTarget[0].BlendOpAlpha = BlendOp(value); ApplyBlend(this); }
void C_D3D11GameDevice::SetBlendSrc(E_Blend value)
{ m_BlendSrc = value; State(this).blend.RenderTarget[0].SrcBlend = Blend(value); ApplyBlend(this); }
void C_D3D11GameDevice::SetBlendSrcAlpha(E_Blend value)
{ m_BlendSrcAlpha = value; State(this).blend.RenderTarget[0].SrcBlendAlpha = AlphaBlendFactor(Blend(value)); ApplyBlend(this); }
void C_D3D11GameDevice::SetBlendHighPrecisionEnable(E_Bool value) { m_BlendHighPrecisionEnable = value; }
void C_D3D11GameDevice::SetColourWriteEnable(U32 value)
{ if (value == E_ColourWriteEnable_Default) value = m_ColourWriteEnableDefault;
  m_ColourWriteEnable = value; State(this).blend.RenderTarget[0].RenderTargetWriteMask = value & 0xf; ApplyBlend(this); }
void C_D3D11GameDevice::SetCullMode(E_CullMode value)
{
    if (value == E_CullMode_Front) value = E_CullMode_Reversed;
    else if (value == E_CullMode_Back) value = E_CullMode_Default;
    if (value == E_CullMode_Default) value = m_CullModeDefault;
    else if (value == E_CullMode_Reversed) value = FlippedCullMode(m_CullModeDefault);
    if (m_CullModeFlip) value = FlippedCullMode(value);
    m_CullMode = value;
    State(this).raster.CullMode = value == E_CullMode_None ? D3D11_CULL_NONE : D3D11_CULL_BACK;
    State(this).raster.FrontCounterClockwise = value == E_CullMode_Cw;
    ApplyRaster(this);
}
void C_D3D11GameDevice::SetCullModeFlip() { SetCullModeFlip(!m_CullModeFlip); }
void C_D3D11GameDevice::SetCullModeFlip(bool value)
{ m_CullModeFlip = value; SetCullMode(value ? m_CullMode : FlippedCullMode(m_CullMode)); }
void C_D3D11GameDevice::SetDepthBias(F32 value)
{ m_DepthBias = value; State(this).raster.DepthBias = static_cast<INT>(value * 0x1000000); ApplyRaster(this); }
void C_D3D11GameDevice::SetDepthEnable(E_Bool value)
{ // [Blur1PCDX11] Resolve the engine default exactly as the PC D3D9 device does.
  if (value == E_Bool_Default) value = m_DepthEnableDefault;
  m_DepthEnable = value; State(this).depth.DepthEnable = value == E_Bool_True; ApplyDepth(this); }
void C_D3D11GameDevice::SetDepthFlip(bool comparison, bool viewport)
{
    if (comparison != m_DepthCmpFlip || (comparison && viewport) != m_DepthViewportFlip)
    {
        m_DepthFuncDefault = comparison ? E_CmpFunc_GreaterEqual : E_CmpFunc_LessEqual;
        m_DepthViewportFlip = comparison && viewport;
        m_DepthCmpFlip = comparison;
    }
}
void C_D3D11GameDevice::SetDepthFunc(E_CmpFunc value)
{
    if (value == E_CmpFunc_Default) value = m_DepthFuncDefault;
    m_DepthFunc = value;
    State(this).depth.DepthFunc = Compare(value);
    ApplyDepth(this);
}
void C_D3D11GameDevice::SetDepthWriteEnable(E_Bool value)
{ m_DepthWriteEnable = value; State(this).depth.DepthWriteMask = value == E_Bool_True ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO; ApplyDepth(this); }
void C_D3D11GameDevice::SetFillMode(E_FillMode value)
{ // [Blur1PCDX11] Preserve the device's selected default, including wireframe.
  if (value == E_FillMode_Default) value = m_FillModeDefault;
  m_FillMode = value; State(this).raster.FillMode = value == E_FillMode_Wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID; ApplyRaster(this); }
void C_D3D11GameDevice::SetPointSpriteEnable(E_Bool value) { m_PointSpriteEnable = value; }
void C_D3D11GameDevice::SetScissorTest(E_Bool value)
{ State(this).raster.ScissorEnable = value == E_Bool_True; ApplyRaster(this); }
void C_D3D11GameDevice::SetScissorRect(U32 x, U32 y, U32 w, U32 h)
{ auto context = Context(m_pd3dDevice); if (!context) return; D3D11_RECT rect = {(LONG)x,(LONG)y,(LONG)(x+w),(LONG)(y+h)}; context->RSSetScissorRects(1, &rect); }
void C_D3D11GameDevice::SetSlopeScaledBias(F32 value)
{ m_SlopeScaledBias = value; State(this).raster.SlopeScaledDepthBias = value; ApplyRaster(this); }
void C_D3D11GameDevice::SetStencilState(const Display::C_StencilState& value)
{
    SetStencilEnable(value.m_StencilEnable); SetStencilFunc(value.m_StencilFunc);
    SetStencilRef(value.m_StencilRef); SetStencilMask(value.m_StencilMask);
    SetStencilWriteMask(value.m_StencilWriteMask); SetStencilFail(value.m_StencilFail);
    SetStencilDepthFail(value.m_StencilDepthFail); SetStencilPass(value.m_StencilDepthPass);
}
namespace {
D3D11_STENCIL_OP StencilOp(E_StencilOp value)
{
    switch(value) {
    case E_StencilOp_Zero: return D3D11_STENCIL_OP_ZERO;
    case E_StencilOp_Replace: return D3D11_STENCIL_OP_REPLACE;
    case E_StencilOp_IncrSat: return D3D11_STENCIL_OP_INCR_SAT;
    case E_StencilOp_DecrSat: return D3D11_STENCIL_OP_DECR_SAT;
    case E_StencilOp_Invert: return D3D11_STENCIL_OP_INVERT;
    case E_StencilOp_Incr: return D3D11_STENCIL_OP_INCR;
    case E_StencilOp_Decr: return D3D11_STENCIL_OP_DECR;
    default: return D3D11_STENCIL_OP_KEEP;
    }
}
}
void C_D3D11GameDevice::SetStencilEnable(E_Bool value)
{ m_StencilEnable = value; State(this).depth.StencilEnable = value == E_Bool_True; ApplyDepth(this); }
void C_D3D11GameDevice::SetStencilFail(E_StencilOp value)
{ State(this).depth.FrontFace.StencilFailOp = StencilOp(value); ApplyDepth(this); }
void C_D3D11GameDevice::SetStencilDepthFail(E_StencilOp value)
{ State(this).depth.FrontFace.StencilDepthFailOp = StencilOp(value); ApplyDepth(this); }
void C_D3D11GameDevice::SetStencilPass(E_StencilOp value)
{ State(this).depth.FrontFace.StencilPassOp = StencilOp(value); ApplyDepth(this); }
void C_D3D11GameDevice::SetStencilFunc(E_CmpFunc value)
{ State(this).depth.FrontFace.StencilFunc = Compare(value); ApplyDepth(this); }
void C_D3D11GameDevice::SetStencilRef(S32 value)
{ State(this).stencilRef = static_cast<UINT>(value); ApplyDepth(this); }
void C_D3D11GameDevice::SetStencilMask(U32 value)
{ State(this).depth.StencilReadMask = value; ApplyDepth(this); }
void C_D3D11GameDevice::SetStencilWriteMask(U32 value)
{ State(this).depth.StencilWriteMask = value; ApplyDepth(this); }
void C_D3D11GameDevice::SetStencilCcwEnable(E_Bool value)
{ State(this).stencilTwoSided = value == E_Bool_True; ApplyDepth(this); }
void C_D3D11GameDevice::SetStencilCcwFail(E_StencilOp value)
{ State(this).depth.BackFace.StencilFailOp = StencilOp(value); ApplyDepth(this); }
void C_D3D11GameDevice::SetStencilCcwDepthFail(E_StencilOp value)
{ State(this).depth.BackFace.StencilDepthFailOp = StencilOp(value); ApplyDepth(this); }
void C_D3D11GameDevice::SetStencilCcwPass(E_StencilOp value)
{ State(this).depth.BackFace.StencilPassOp = StencilOp(value); ApplyDepth(this); }
void C_D3D11GameDevice::SetStencilCcwFunc(E_CmpFunc value)
{ State(this).depth.BackFace.StencilFunc = Compare(value); ApplyDepth(this); }
// [Blur1PCDX11] PC has one reference/read-mask/write-mask shared by both banks.
// The pristine PC CCW setters are no-ops, not writes to that shared state.
void C_D3D11GameDevice::SetStencilCcwRef(S32) {}
void C_D3D11GameDevice::SetStencilCcwMask(U32) {}
void C_D3D11GameDevice::SetStencilCcwWriteMask(U32) {}
// [Blur1PCDX11] PC D3D9 compiles this out (D3D9Device.inl, #ifndef PLATFORM_PC): the
// swap interval is the device parameter from the settings, applied at device
// creation/reset. The per-frame Graphics.Render.PresentInterval value is console-only.
void C_D3D11GameDevice::SetPresentInterval(E_PresentInterval value) { (void)value; }
void C_D3D11GameDevice::SetTextureAddressU(U32 slot, E_TextureAddress value)
{ if (slot >= E_Sampler_Count) return; m_Sampler[slot].m_AddressU = value;
  State(this).sampler[slot].AddressU = Address(value); ApplySampler(this, slot, false); }
void C_D3D11GameDevice::SetTextureAddressV(U32 slot, E_TextureAddress value)
{ if (slot >= E_Sampler_Count) return; m_Sampler[slot].m_AddressV = value;
  State(this).sampler[slot].AddressV = Address(value); ApplySampler(this, slot, false); }
void C_D3D11GameDevice::SetTextureAddressW(U32 slot, E_TextureAddress value)
{ if (slot >= E_Sampler_Count) return; m_Sampler[slot].m_AddressW = value;
  State(this).sampler[slot].AddressW = Address(value); ApplySampler(this, slot, false); }
void C_D3D11GameDevice::SetTextureAddressUVW(U32 slot, E_TextureAddress u, E_TextureAddress v, E_TextureAddress w)
{ SetTextureAddressU(slot,u); SetTextureAddressV(slot,v); SetTextureAddressW(slot,w); }
void C_D3D11GameDevice::SetTextureFilterMag(U32 slot, E_TextureFilter value)
{ if (slot >= E_Sampler_Count) return; m_Sampler[slot].m_FilterMag = value;
  State(this).sampler[slot].Filter = Filter(m_Sampler[slot].m_FilterMag, m_Sampler[slot].m_FilterMin, m_Sampler[slot].m_FilterMip);
  ApplySampler(this,slot,false); }
void C_D3D11GameDevice::SetTextureFilterMin(U32 slot, E_TextureFilter value)
{ if (slot >= E_Sampler_Count) return; m_Sampler[slot].m_FilterMin = value;
  State(this).sampler[slot].Filter = Filter(m_Sampler[slot].m_FilterMag, m_Sampler[slot].m_FilterMin, m_Sampler[slot].m_FilterMip);
  ApplySampler(this,slot,false); }
void C_D3D11GameDevice::SetTextureFilterMip(U32 slot, E_TextureFilter value)
{ if (slot >= E_Sampler_Count) return; m_Sampler[slot].m_FilterMip = value;
  State(this).sampler[slot].Filter = Filter(m_Sampler[slot].m_FilterMag, m_Sampler[slot].m_FilterMin, m_Sampler[slot].m_FilterMip);
  ApplySampler(this,slot,false); }
void C_D3D11GameDevice::SetTextureFilterMagMinMip(U32 slot, E_TextureFilter mag, E_TextureFilter min, E_TextureFilter mip)
{ if (slot >= E_Sampler_Count) return;
  m_Sampler[slot].m_FilterMag = mag; m_Sampler[slot].m_FilterMin = min; m_Sampler[slot].m_FilterMip = mip;
  State(this).sampler[slot].Filter = Filter(mag,min,mip); ApplySampler(this,slot,false); }
void C_D3D11GameDevice::SetTextureBorderColour(U32 slot, U32 colour)
{ if (slot >= E_Sampler_Count) return; m_Sampler[slot].m_BorderColour = colour;
  auto& d = State(this).sampler[slot]; d.BorderColor[0] = ((colour >> 16) & 255) / 255.f;
  d.BorderColor[1] = ((colour >> 8) & 255) / 255.f; d.BorderColor[2] = (colour & 255) / 255.f;
  d.BorderColor[3] = ((colour >> 24) & 255) / 255.f; ApplySampler(this,slot,false); }
void C_D3D11GameDevice::SetTextureLodBias(U32 slot, S8 bias)
{ if (slot >= E_Sampler_Count) return; m_Sampler[slot].m_LodBias = bias;
  // [Blur2PC] Display2 encodes the PC mip bias in sixteenths (D3D9Functions.inl).
  State(this).sampler[slot].MipLODBias = static_cast<float>(bias) / 16.f; ApplySampler(this,slot,false); }
void C_D3D11GameDevice::SetTextureGamma(U32 slot, E_Bool value)
{
    if (slot >= E_Sampler_Count) return;
    // [Blur1PCDX11] Match the PC sampler override and allow gamma to change
    // after the texture was bound, without requiring another SetTexture call.
    if (m_ForceTextureGamma != E_Bool_Default) value = m_ForceTextureGamma;
    if (m_Sampler[slot].m_Gamma == value) return;
    m_Sampler[slot].m_Gamma = value;
    ComPtr<ID3D11Resource> resource;
    if (State(this).pixelViews[slot]) State(this).pixelViews[slot]->GetResource(&resource);
    if (resource) SetNativeTexture(slot, resource.Get());
}
void C_D3D11GameDevice::SetVertexTextureAddressU(U32 slot, E_TextureAddress value)
{ if (slot >= E_VertexSampler_Count) return; m_VertexSampler[slot].m_AddressU = value;
  State(this).vertexSampler[slot].AddressU = Address(value); ApplySampler(this, slot, true); }
void C_D3D11GameDevice::SetVertexTextureAddressV(U32 slot, E_TextureAddress value)
{ if (slot >= E_VertexSampler_Count) return; m_VertexSampler[slot].m_AddressV = value;
  State(this).vertexSampler[slot].AddressV = Address(value); ApplySampler(this, slot, true); }
void C_D3D11GameDevice::SetVertexTextureAddressW(U32 slot, E_TextureAddress value)
{ if (slot >= E_VertexSampler_Count) return; m_VertexSampler[slot].m_AddressW = value;
  State(this).vertexSampler[slot].AddressW = Address(value); ApplySampler(this, slot, true); }
void C_D3D11GameDevice::SetVertexTextureAddressUVW(U32 slot, E_TextureAddress u, E_TextureAddress v, E_TextureAddress w)
{ SetVertexTextureAddressU(slot,u); SetVertexTextureAddressV(slot,v); SetVertexTextureAddressW(slot,w); }
void C_D3D11GameDevice::SetVertexTextureFilterMag(U32 slot, E_TextureFilter value)
{ if (slot >= E_VertexSampler_Count) return; m_VertexSampler[slot].m_FilterMag = value;
  State(this).vertexSampler[slot].Filter = Filter(m_VertexSampler[slot].m_FilterMag, m_VertexSampler[slot].m_FilterMin, m_VertexSampler[slot].m_FilterMip);
  ApplySampler(this,slot,true); }
void C_D3D11GameDevice::SetVertexTextureFilterMin(U32 slot, E_TextureFilter value)
{ if (slot >= E_VertexSampler_Count) return; m_VertexSampler[slot].m_FilterMin = value;
  State(this).vertexSampler[slot].Filter = Filter(m_VertexSampler[slot].m_FilterMag, m_VertexSampler[slot].m_FilterMin, m_VertexSampler[slot].m_FilterMip);
  ApplySampler(this,slot,true); }
void C_D3D11GameDevice::SetVertexTextureFilterMip(U32 slot, E_TextureFilter value)
{ if (slot >= E_VertexSampler_Count) return; m_VertexSampler[slot].m_FilterMip = value;
  State(this).vertexSampler[slot].Filter = Filter(m_VertexSampler[slot].m_FilterMag, m_VertexSampler[slot].m_FilterMin, m_VertexSampler[slot].m_FilterMip);
  ApplySampler(this,slot,true); }
void C_D3D11GameDevice::SetVertexTextureFilterMagMinMip(U32 slot, E_TextureFilter mag, E_TextureFilter min, E_TextureFilter mip)
{ if (slot >= E_VertexSampler_Count) return;
  m_VertexSampler[slot].m_FilterMag = mag; m_VertexSampler[slot].m_FilterMin = min; m_VertexSampler[slot].m_FilterMip = mip;
  State(this).vertexSampler[slot].Filter = Filter(mag,min,mip); ApplySampler(this,slot,true); }
void C_D3D11GameDevice::SetVertexTextureBorderColour(U32 slot, U32 colour)
{ if (slot >= E_VertexSampler_Count) return; m_VertexSampler[slot].m_BorderColour = colour;
  auto& d = State(this).vertexSampler[slot]; d.BorderColor[0] = ((colour >> 16) & 255) / 255.f;
  d.BorderColor[1] = ((colour >> 8) & 255) / 255.f; d.BorderColor[2] = (colour & 255) / 255.f;
  d.BorderColor[3] = ((colour >> 24) & 255) / 255.f; ApplySampler(this,slot,true); }
void C_D3D11GameDevice::SetVertexTextureLodBias(U32 slot, S8 bias)
{ if (slot >= E_VertexSampler_Count) return; m_VertexSampler[slot].m_LodBias = bias;
  // [Blur2PC] Same Display2 encoding for vertex texture samplers.
  State(this).vertexSampler[slot].MipLODBias = static_cast<float>(bias) / 16.f; ApplySampler(this,slot,true); }
void C_D3D11GameDevice::SetVertexTextureGamma(U32 slot, E_Bool value)
{
    if (slot >= E_VertexSampler_Count) return;
    // [Blur1PCDX11] Match the PC sampler override and allow gamma to change
    // after the texture was bound, without requiring another SetTexture call.
    if (m_ForceTextureGamma != E_Bool_Default) value = m_ForceTextureGamma;
    if (m_VertexSampler[slot].m_Gamma == value) return;
    m_VertexSampler[slot].m_Gamma = value;
    ComPtr<ID3D11Resource> resource;
    if (State(this).vertexViews[slot]) State(this).vertexViews[slot]->GetResource(&resource);
    if (resource) SetNativeVertexTexture(slot, resource.Get());
}
} }
