// [Blur1PCDX11] Verifies actual D3D11 device creation, shaders, draw and readback.
#include "Display/Device/D3D11/D3D11Device.h"
#include "Display/Device/D3D11/D3D11Effect.h"
#include "Display/Device/D3D11/D3D11EffectLibrary.h"
#include "Display/Device/D3D11/D3D11InputLayout.h"
#include "Display/Device/D3D11/D3D11States.h"
#include "Display/Device/D3D11/D3D11Formats.h"
#include <cstdio>
#include <string>
#include <cmath>
#include <fstream>
#include <vector>
#include <d3dcompiler.h>
#include <d3d11shader.h>
// [Blur1PCDX11] Raw context resets must also forget FX binding memoization.
extern void D3DX11InvalidateBoundState(unsigned stages, unsigned what);
#include "luminance_checks.h"
#include "sampler_checks.h"
#include "polygon_font_checks.h"
#include "road_prepass_checks.h"
#include "billboard_prepass_checks.h"
#include "polygon_packed_checks.h"
#include "vehicle_volume_checks.h"
#include "cube_binding_checks.h"

// [Blur1PCDX11][retail-recovered] Alloy's shipped prepass decodes packed
// normals, packs normal.z across BA, and writes negative clip z/w to MRT1.
static bool CheckVehiclePrepass(Display::D3D11::C_D3D11Device& device,
                               Display::D3D11::C_D3D11EffectLibrary& library)
{
    std::ifstream recoveredIndex("_work/build/gen/fx11/effects/effects.recovered");
    std::vector<std::string> effects;
    std::string key;
    while (std::getline(recoveredIndex, key)) if (!key.empty()) effects.push_back(key);
    if (effects.empty() || recoveredIndex.bad()) return false;
    const char* techniques[] = { "Material", "MaterialNoDamage", "MaterialAlphaLuminance",
        "MaterialAlphaLuminanceNoDamage", "MaterialLOD", "MaterialNoDamageLOD",
        "MaterialAlphaLuminanceLOD", "MaterialAlphaLuminanceNoDamageLOD",
        "MaterialPrepass", "MaterialPrepassNoDamage" };
    for (const std::string& effect : effects)
    {
        Display::D3D11::C_D3D11Effect* native = nullptr;
        if (FAILED(library.Get(device.Device(), effect.c_str(), &native))) return false;
        for (const char* technique : techniques)
        {
            const void* bytes = nullptr;
            SIZE_T size = 0;
            if (FAILED(native->GetPassVertexShaderBytecode(technique, 0, &bytes, &size)) || !size ||
                FAILED(native->GetPassPixelShaderBytecode(technique, 0, &bytes, &size)) || !size)
            { std::fprintf(stderr, "Missing recovered pass: %s/%s\n", effect.c_str(), technique); return false; }
        }
    }
    Display::D3D11::C_D3D11Effect* alloy = nullptr;
    if (FAILED(library.Get(device.Device(), "veh_alloy_f_LN", &alloy))) return false;
    const float identity[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    if (FAILED(alloy->SetRawParameter("g_mWorld", identity, sizeof(identity))) ||
        FAILED(alloy->SetRawParameter("g_mViewProj", identity, sizeof(identity)))) return false;
    struct Vertex { float position[4], normal[4], uv[4], colour[4]; };
    const Vertex vertices[] = {
        { {0,.8f,.2f,1}, {.5f,.5f,1,0}, {0,0,0,0}, {.5f,.5f,.5f,1} },
        { {.8f,-.8f,.2f,1}, {.5f,.5f,1,0}, {0,0,0,0}, {.5f,.5f,.5f,1} },
        { {-.8f,-.8f,.2f,1}, {.5f,.5f,1,0}, {0,0,0,0}, {.5f,.5f,.5f,1} }
    };
    const unsigned short indices[] = { 0,1,2 };
    const D3DVERTEXELEMENT9 elements[] = {
        {0,0,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_POSITION,0},
        {0,16,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_NORMAL,0},
        {0,32,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_TEXCOORD,0},
        {0,48,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_COLOR,0}
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> vb, ib;
    if (FAILED(device.CreateVertexBuffer(sizeof(vertices), vertices, false, vb)) ||
        FAILED(device.CreateIndexBuffer(sizeof(indices), indices, false, ib))) return false;
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = desc.Height = 64;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R32_FLOAT;
    desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depth, staging;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> depthView;
    if (FAILED(device.Device()->CreateTexture2D(&desc, nullptr, &depth)) ||
        FAILED(device.Device()->CreateRenderTargetView(depth.Get(), nullptr, &depthView))) return false;
    desc.BindFlags = 0;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(device.Device()->CreateTexture2D(&desc, nullptr, &staging))) return false;
    D3D11_RASTERIZER_DESC raster = {};
    raster.FillMode = D3D11_FILL_SOLID;
    raster.CullMode = D3D11_CULL_NONE;
    raster.DepthClipEnable = TRUE;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterState;
    if (FAILED(device.Device()->CreateRasterizerState(&raster, &rasterState))) return false;
    D3D11_VIEWPORT viewport = {0,0,64,64,0,1};
    const float clear[4] = {0,0,0,0};
    for (const char* technique : { "MaterialPrepass", "MaterialPrepassNoDamage" })
    {
        device.Context()->ClearState(); D3DX11InvalidateBoundState(3,15);
        device.Context()->ClearRenderTargetView(device.RenderTargetView(), clear);
        device.Context()->ClearRenderTargetView(depthView.Get(), clear);
        device.Context()->ClearDepthStencilView(device.DepthStencilView(), D3D11_CLEAR_DEPTH, 1, 0);
        ID3D11RenderTargetView* targets[] = {device.RenderTargetView(), depthView.Get()};
        device.Context()->OMSetRenderTargets(2, targets, device.DepthStencilView());
        device.Context()->RSSetViewports(1, &viewport);
        device.Context()->RSSetState(rasterState.Get());
        HRESULT hr = device.DrawGameEffectIndexed(*alloy, technique, 0, elements, 4,
            vb.Get(), sizeof(Vertex), ib.Get(), DXGI_FORMAT_R16_UINT, 3);
        unsigned char pixel[4] = {};
        if (FAILED(hr) || FAILED(device.ReadPixel(32,32,pixel))) return false;
        device.Context()->OMSetRenderTargets(0, nullptr, nullptr);
        device.Context()->CopyResource(staging.Get(), depth.Get());
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (FAILED(device.Context()->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped))) return false;
        const float z = reinterpret_cast<const float*>(static_cast<const unsigned char*>(mapped.pData) +
            32 * mapped.RowPitch)[32];
        device.Context()->Unmap(staging.Get(),0);
        std::printf("Recovered alloy %s normal RGBA %u %u %u %u depth %.6f\n",
            technique,pixel[0],pixel[1],pixel[2],pixel[3],z);
        if (pixel[0] < 127 || pixel[0] > 128 || pixel[1] < 127 || pixel[1] > 128 ||
            pixel[2] != 255 || pixel[3] != 0 || std::fabs(z + .2f) > .0001f) return false;
    }
    device.Context()->ClearState(); D3DX11InvalidateBoundState(3,15);
    std::printf("Recovered vehicle passes: %zu exact permutations x 10 techniques available\n", effects.size());
    return true;
}

// [Blur1PCDX11] Check the actual compiled FX11 sampler defaults. FX9 leaves
// AlbedoSampler at WRAP while EnvironmentSampler explicitly requests CLAMP.
static bool CheckSceneSamplerDefaults(Display::D3D11::C_D3D11Device& device,
                                     Display::D3D11::C_D3D11EffectLibrary& library)
{
    Display::D3D11::C_D3D11Effect* effect = nullptr;
    if (FAILED(library.Get(device.Device(), "Default_LN_NB_NV.fxb", &effect, "MaterialLights"))) return false;
    const void* code = nullptr;
    SIZE_T bytes = 0;
    if (FAILED(effect->GetPassPixelShaderBytecode("MaterialLights", 0, &code, &bytes))) return false;
    Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflection;
    if (FAILED(D3DReflect(code, bytes, IID_PPV_ARGS(&reflection)))) return false;
    if (FAILED(effect->Apply("MaterialLights", 0, device.Context()))) return false;
    D3D11_SHADER_DESC shader = {};
    reflection->GetDesc(&shader);
    unsigned checked = 0;
    for (UINT i = 0; i < shader.BoundResources; ++i)
    {
        D3D11_SHADER_INPUT_BIND_DESC binding = {};
        reflection->GetResourceBindingDesc(i, &binding);
        if (binding.Type != D3D_SIT_SAMPLER) continue;
        const std::string name(binding.Name);
        if (name != "AlbedoSampler" && name != "EnvironmentSampler") continue;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
        device.Context()->PSGetSamplers(binding.BindPoint, 1, &sampler);
        if (!sampler) return false;
        D3D11_SAMPLER_DESC desc = {};
        sampler->GetDesc(&desc);
        const auto expected = name == "AlbedoSampler" ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
        if (desc.AddressU != expected || desc.AddressV != expected || desc.AddressW != expected)
        { std::fprintf(stderr, "%s address defaults: %u/%u/%u expected %u\n", binding.Name,
                       desc.AddressU, desc.AddressV, desc.AddressW, expected); return false; }
        ++checked;
    }
    device.Context()->ClearState(); D3DX11InvalidateBoundState(3,15);
    std::printf("Scene sampler defaults: %u checked (albedo WRAP, environment CLAMP)\n", checked);
    return checked == 2;
}

// [Blur1PCDX11] Exercise the generated PS exit with the engine's mutable state.
// Boundary pixels distinguish Greater from GreaterEqual and disabled from Never.
// [Blur1PCDX11] D3D11 stage linkage requires register/component agreement,
// even when the original D3D9 stages shared matching semantic names.
static bool CheckBuiltinShaderLinks()
{
    std::vector<std::pair<std::wstring, std::wstring>> pairs;
    for (const wchar_t* suffix : {L"", L"R", L"G", L"B", L"A", L"HDR"})
        pairs.emplace_back(L"render/vsScreenBlobShadows", std::wstring(L"render/psScreenBlobShadows") + suffix);
    pairs.emplace_back(L"ui/vsMultiTexture1", L"ui/psAMLoading");
    for (const wchar_t* suffix : {L"", L"Projected", L"Volume"})
        pairs.emplace_back(L"deferred/vsSpotlightPrepass", std::wstring(L"deferred/psSpotlightPrepass") + suffix);
    for (unsigned count = 1; count <= 8; ++count)
        for (const wchar_t* suffix : {L"", L"HDR"})
            pairs.emplace_back(L"dropshadows/vsDropShadowsViewNormal",
                L"dropshadows/psDropShadowsViewNormal" + std::to_wstring(count) + suffix);
    for (const wchar_t* prefix : {L"deferred/psDynamicLightsPrepass", L"deferred/psDynamicLightsPrepass2xMSAA"})
        for (unsigned count = 1; count <= 8; ++count)
            pairs.emplace_back(L"deferred/vsDynamicLightsPrepass", std::wstring(prefix) + std::to_wstring(count));
    // [Blur1PCDX11] These shaders must link once the PC initialization also runs on DX11.
    pairs.emplace_back(L"display/vsOcclusion", L"display/psOcclusion");
    pairs.emplace_back(L"splashCamera/vsSplashCamera", L"splashCamera/psSplashCamera");
    pairs.emplace_back(L"splashCamera/vsSplashCameraRefract", L"splashCamera/psSplashCameraRefract");
    pairs.emplace_back(L"splashCamera/vsSplashCameraRefract", L"splashCamera/psSplashCameraLight");
    const std::pair<const wchar_t*, const wchar_t*> particlePairs[] = {
        {L"vsParticle", L"psParticle"}, {L"vsParticle", L"psParticleA8"},
        {L"vsParticle2D", L"psParticle"}, {L"vsParticle2D", L"psParticleA8"},
        {L"vsParticleUVW", L"psParticle"}, {L"vsParticleUVW", L"psParticleA8"},
        {L"vsParticleNdotL", L"psParticle"}, {L"vsParticleNdotL", L"psParticleA8"},
        {L"vsParticleNormalMap", L"psParticleNormalMap"},
        {L"vsParticleSoft", L"psParticleSoft"}, {L"vsParticleSoft", L"psParticleSoftA8"},
        {L"vsParticleSoft", L"psParticle"}, {L"vsParticleSoft", L"psParticleA8"},
        {L"vsParticleDecal", L"psParticleDecal"}, {L"vsParticlePointSprite", L"psParticlePointSprite"}
    };
    for (const auto& pair : particlePairs)
        for (const wchar_t* lights : {L"", L"PointLights"})
            for (const wchar_t* fog : {L"", L"HeightFog", L"HDR", L"HDRU32"})
                pairs.emplace_back(std::wstring(L"dynamic/") + pair.first + lights,
                    std::wstring(L"dynamic/") + pair.second + fog);
    for (const auto& pair : pairs)
    {
        Microsoft::WRL::ComPtr<ID3DBlob> vb, pb;
        const std::wstring root = L"_work/build/gen/dx11_builtins/";
        if (FAILED(D3DReadFileToBlob((root + pair.first + L".cso").c_str(), &vb)) ||
            FAILED(D3DReadFileToBlob((root + pair.second + L".cso").c_str(), &pb))) return false;
        Microsoft::WRL::ComPtr<ID3D11ShaderReflection> vs, ps;
        if (FAILED(D3DReflect(vb->GetBufferPointer(), vb->GetBufferSize(), IID_PPV_ARGS(&vs))) ||
            FAILED(D3DReflect(pb->GetBufferPointer(), pb->GetBufferSize(), IID_PPV_ARGS(&ps)))) return false;
        D3D11_SHADER_DESC vd = {}, pd = {};
        vs->GetDesc(&vd); ps->GetDesc(&pd);
        for (UINT i = 0; i < pd.InputParameters; ++i)
        {
            D3D11_SIGNATURE_PARAMETER_DESC input = {};
            ps->GetInputParameterDesc(i, &input);
            if (!input.ReadWriteMask) continue;
            bool matched = false;
            for (UINT j = 0; j < vd.OutputParameters; ++j)
            {
                D3D11_SIGNATURE_PARAMETER_DESC output = {};
                vs->GetOutputParameterDesc(j, &output);
                if (_stricmp(input.SemanticName, output.SemanticName) == 0 &&
                    input.SemanticIndex == output.SemanticIndex && input.Register == output.Register &&
                    input.ComponentType == output.ComponentType &&
                    (output.Mask & input.ReadWriteMask) == input.ReadWriteMask) matched = true;
            }
            if (!matched)
            {
                std::fprintf(stderr, "Shader linkage failed: %ls / %ls, %s%u register %u\n",
                    pair.first.c_str(), pair.second.c_str(), input.SemanticName, input.SemanticIndex, input.Register);
                return false;
            }
        }
    }
    std::printf("Built-in shader linkage: %zu stage pairs checked\n", pairs.size());
    return true;
}

static bool CheckNativeAlphaTest(Display::D3D11::C_D3D11Device& device)
{
    struct AlphaState { UINT enabled, function; float reference; UINT padding; };
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(AlphaState);
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
    if (FAILED(device.Device()->CreateBuffer(&desc, nullptr, &buffer))) return false;
    const unsigned char alphas[] = {0, 127, 128, 129, 255};
    const unsigned expectedMasks[] = {0x00, 0x03, 0x04, 0x07, 0x18, 0x1b, 0x1c, 0x1f, 0x1f};
    for (UINT test = 0; test < 9; ++test)
        for (UINT pixel = 0; pixel < 5; ++pixel)
        {
            const AlphaState state = {test == 8 ? 0U : 1U, test == 8 ? 1U : test + 1, 128.f / 255.f, 0};
            device.Context()->UpdateSubresource(buffer.Get(), 0, nullptr, &state, 0, 0);
            ID3D11Buffer* raw = buffer.Get();
            device.Context()->PSSetConstantBuffers(13, 1, &raw);
            const unsigned char texel[] = {200, 150, 20, alphas[pixel]};
            const HRESULT hr = device.DrawTexturedQuad(
                L"_work/build/gen/dx11_builtins/display/vsBasicRenderTextured2D.cso",
                L"_work/build/gen/dx11_builtins/display/psBasicRenderTextured.cso", texel);
            unsigned char rgba[4] = {};
            if (FAILED(hr) || FAILED(device.ReadPixel(32, 32, rgba))) return false;
            const bool expected = (expectedMasks[test] & (1U << pixel)) != 0;
            if (rgba[0] != (expected ? 200 : 0))
            {
                std::fprintf(stderr, "Alpha test failed: enabled=%u func=%u alpha=%u red=%u expectedPass=%u\n",
                    state.enabled, state.function, alphas[pixel], rgba[0], expected);
                return false;
            }
        }
    std::printf("Native alpha test: 45 GPU pixel checks passed (all comparisons, threshold, disabled)\n");
    return true;
}

// [Blur1PCDX11] One PC resource must support successive linear/sRGB/linear
// samplers without changing its bytes, including BC formats and alpha.
static bool CheckTextureGammaViews(Display::D3D11::C_D3D11Device& device)
{
    const unsigned char rgba[] = {128,128,128,102};
    const unsigned char bc1[] = {0x10,0x84,0,0,0,0,0,0};
    const unsigned char bc2[] = {0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x10,0x84,0,0,0,0,0,0};
    const unsigned char bc3[] = {102,0,0,0,0,0,0,0,0x10,0x84,0,0,0,0,0,0};
    const struct { D3DFORMAT format; const void* bytes; UINT size, edge; } cases[] = {
        {D3DFMT_A8R8G8B8,rgba,4,1}, {D3DFMT_X8R8G8B8,rgba,4,1},
        {D3DFMT_A8B8G8R8,rgba,4,1}, {D3DFMT_DXT1,bc1,8,4},
        {D3DFMT_DXT3,bc2,16,4}, {D3DFMT_DXT5,bc3,16,4}
    };
    unsigned checked = 0;
    for (const auto& test : cases)
    {
        const void* data[] = {test.bytes};
        const UINT sizes[] = {test.size};
        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
        HRESULT result = device.CreateTexture2DFromPCData(test.edge,test.edge,1,test.format,data,sizes,texture,view);
        if (FAILED(result)) { std::fprintf(stderr,"Gamma texture format=%u create=%08lx\n",test.format,result); return false; }
        unsigned char linear[4] = {};
        for (int pass = 0; pass < 3; ++pass)
        {
            view.Reset();
            result = Display::D3D11::CreateTextureView(device.Device(), texture.Get(), pass == 1, &view);
            if (FAILED(result)) { std::fprintf(stderr,"Gamma texture format=%u pass=%d view=%08lx\n",test.format,pass,result); return false; }
            result = device.DrawTexturedQuadView(
                L"_work/build/gen/dx11_builtins/display/vsBasicRenderTextured2D.cso",
                L"_work/build/gen/dx11_builtins/display/psBasicRenderTextured.cso", view.Get());
            if (FAILED(result)) { std::fprintf(stderr,"Gamma texture format=%u pass=%d draw=%08lx\n",test.format,pass,result); return false; }
            unsigned char actual[4] = {};
            result = device.ReadPixel(32,32,actual);
            if (FAILED(result)) { std::fprintf(stderr,"Gamma texture format=%u pass=%d read=%08lx\n",test.format,pass,result); return false; }
            if (pass == 0) memcpy(linear,actual,sizeof(linear));
            for (unsigned channel = 0; channel < 4; ++channel)
            {
                const float encoded = linear[channel] / 255.f;
                const int expected = pass == 1 && channel < 3 ? static_cast<int>(std::lround(
                    (encoded <= .04045f ? encoded / 12.92f : std::pow((encoded + .055f) / 1.055f, 2.4f)) * 255.f)) : linear[channel];
                if (std::abs(int(actual[channel]) - expected) > 1)
                { std::fprintf(stderr,"Gamma view format=%u pass=%d channel=%u got=%u expected=%d\n",
                    test.format,pass,channel,actual[channel],expected); return false; }
            }
            ++checked;
        }
    }
    // HDR values ignore SRGBTEXTURE; alpha must also stay linear.
    const unsigned short half[] = {0x3800,0x3800,0x3800,0x3400};
    const void* data[] = {half}; const UINT sizes[] = {sizeof(half)};
    Microsoft::WRL::ComPtr<ID3D11Texture2D> hdr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> hdrView;
    HRESULT hdrResult = device.CreateTexture2DFromPCData(1,1,1,D3DFMT_A16B16G16R16F,data,sizes,hdr,hdrView);
    if (FAILED(hdrResult)) { std::fprintf(stderr,"HDR gamma texture create=%08lx\n",hdrResult); return false; }
    hdrView.Reset();
    hdrResult = Display::D3D11::CreateTextureView(device.Device(),hdr.Get(),true,&hdrView);
    if (SUCCEEDED(hdrResult)) hdrResult = device.DrawTexturedQuadView(
            L"_work/build/gen/dx11_builtins/display/vsBasicRenderTextured2D.cso",
            L"_work/build/gen/dx11_builtins/display/psBasicRenderTextured.cso",hdrView.Get());
    if (FAILED(hdrResult)) { std::fprintf(stderr,"HDR gamma view/draw=%08lx\n",hdrResult); return false; }
    unsigned char hdrPixel[4] = {};
    if (FAILED(device.ReadPixel(32,32,hdrPixel)) || std::abs(int(hdrPixel[0])-128) > 1 ||
        std::abs(int(hdrPixel[1])-128) > 1 || std::abs(int(hdrPixel[2])-128) > 1 || std::abs(int(hdrPixel[3])-64) > 1)
    { std::fprintf(stderr,"HDR gamma-ignore RGBA %u %u %u %u\n",hdrPixel[0],hdrPixel[1],hdrPixel[2],hdrPixel[3]); return false; }
    for (bool cube : {true,false})
    {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeTexture;
        Microsoft::WRL::ComPtr<ID3D11Texture3D> volumeTexture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
        const void* faces[] = {rgba,rgba,rgba,rgba,rgba,rgba};
        const UINT faceSizes[] = {4,4,4,4,4,4};
        HRESULT result = cube ? device.CreateTextureCubeFromPCData(1,1,D3DFMT_A8R8G8B8,faces,faceSizes,cubeTexture,view) :
            device.CreateTexture3DFromPCData(1,1,1,1,D3DFMT_A8R8G8B8,faces,faceSizes,volumeTexture,view);
        if (FAILED(result)) return false;
        const char* shader = cube ?
            "TextureCube T:register(t0); SamplerState S:register(s0); float4 main():SV_Target {return T.Sample(S,float3(1,0,0));}" :
            "Texture3D T:register(t0); SamplerState S:register(s0); float4 main():SV_Target {return T.Sample(S,float3(.5,.5,.5));}";
        Microsoft::WRL::ComPtr<ID3DBlob> code,errors;
        if (FAILED(D3DCompile(shader,strlen(shader),nullptr,nullptr,nullptr,"main","ps_5_0",0,0,&code,&errors))) return false;
        const wchar_t* path = cube ? L"_work/build/gen/probe_gamma_cube.cso" : L"_work/build/gen/probe_gamma_volume.cso";
        if (FAILED(D3DWriteBlobToFile(code.Get(),path,TRUE))) return false;
        ID3D11Resource* resource = cube ? static_cast<ID3D11Resource*>(cubeTexture.Get()) : volumeTexture.Get();
        for (int pass = 0; pass < 3; ++pass)
        {
            view.Reset();
            if (FAILED(Display::D3D11::CreateTextureView(device.Device(),resource,pass == 1,&view)) ||
                FAILED(device.DrawTexturedQuadView(L"_work/build/gen/dx11_builtins/display/vsBasicRenderTextured2D.cso",path,view.Get()))) return false;
            unsigned char pixel[4] = {};
            const int expected = pass == 1 ? 55 : 128;
            if (FAILED(device.ReadPixel(32,32,pixel)) || std::abs(int(pixel[0])-expected)>1 ||
                std::abs(int(pixel[1])-expected)>1 || std::abs(int(pixel[2])-expected)>1 || std::abs(int(pixel[3])-102)>1)
            { std::fprintf(stderr,"%s gamma pass=%d RGBA %u %u %u %u\n",cube?"Cube":"Volume",pass,pixel[0],pixel[1],pixel[2],pixel[3]); return false; }
            ++checked;
        }
    }
    std::printf("Texture gamma views: %u GPU transitions across six colour formats, cube and volume; HDR unchanged\n",checked);
    return true;
}

static bool CheckAlphaBlendFactors(Display::D3D11::C_D3D11Device& device)
{
    const D3DBLEND factors[] = {D3DBLEND_SRCCOLOR, D3DBLEND_INVSRCCOLOR,
        D3DBLEND_DESTCOLOR, D3DBLEND_INVDESTCOLOR, D3DBLEND_SRCALPHASAT};
    const unsigned char expected[][4] = {{157,39,10,64}, {43,61,40,64},
        {0,0,0,128}, {200,100,50,0}, {0,0,0,128}};
    const unsigned char texel[] = {200,100,50,128};
    for (unsigned test = 0; test < 5; ++test)
    {
        D3D11_BLEND_DESC desc = {};
        if (!Display::D3D11::MakeBlendDesc(factors[test], D3DBLEND_ZERO, D3DBLENDOP_ADD,
            factors[test], D3DBLEND_ZERO, D3DBLENDOP_ADD, true, 15, &desc)) return false;
        Microsoft::WRL::ComPtr<ID3D11BlendState> blend;
        if (FAILED(device.Device()->CreateBlendState(&desc, &blend))) return false;
        const float scale[] = {1,1,1,1};
        device.Context()->OMSetBlendState(blend.Get(), scale, 0xffffffff);
        if (FAILED(device.DrawTexturedQuad(
            L"_work/build/gen/dx11_builtins/display/vsBasicRenderTextured2D.cso",
            L"_work/build/gen/dx11_builtins/display/psBasicRenderTextured.cso", texel))) return false;
        unsigned char actual[4] = {};
        if (FAILED(device.ReadPixel(32, 32, actual))) return false;
        for (unsigned channel = 0; channel < 4; ++channel)
            if (std::abs(int(actual[channel]) - int(expected[test][channel])) > 1)
            {
                std::fprintf(stderr, "Blend factor %u channel %u: %u expected %u\n",
                    factors[test], channel, actual[channel], expected[test][channel]);
                return false;
            }
    }
    std::puts("PC alpha blend factors: 5 GPU RGBA checks passed");
    return true;
}

// [Blur1PCDX11] Read actual stencil bytes for CW/CCW geometry while switching
// one/two-sided state and rasterizer orientation, including failed tests.
static bool CheckStencilFaces(Display::D3D11::C_D3D11Device& device)
{
    using Microsoft::WRL::ComPtr;
    auto* gpu = device.Device(); auto* context = device.Context();
    const char source[] =
        "float4 VS(uint i:SV_VertexID):SV_Position {"
        "float2 p[6]={float2(-.9,-.9),float2(-.9,.9),float2(-.1,-.9),"
        "float2(.1,-.9),float2(.9,-.9),float2(.1,.9)}; return float4(p[i],.5,1); }"
        "float4 PS():SV_Target { return 1; }";
    ComPtr<ID3DBlob> vsCode, psCode;
    if (FAILED(D3DCompile(source,sizeof(source)-1,nullptr,nullptr,nullptr,"VS","vs_5_0",0,0,&vsCode,nullptr)) ||
        FAILED(D3DCompile(source,sizeof(source)-1,nullptr,nullptr,nullptr,"PS","ps_5_0",0,0,&psCode,nullptr))) return false;
    ComPtr<ID3D11VertexShader> vs; ComPtr<ID3D11PixelShader> ps;
    if (FAILED(gpu->CreateVertexShader(vsCode->GetBufferPointer(),vsCode->GetBufferSize(),nullptr,&vs)) ||
        FAILED(gpu->CreatePixelShader(psCode->GetBufferPointer(),psCode->GetBufferSize(),nullptr,&ps))) return false;
    D3D11_TEXTURE2D_DESC td = {};
    td.Width=16; td.Height=8; td.MipLevels=td.ArraySize=td.SampleDesc.Count=1;
    td.Format=DXGI_FORMAT_R24G8_TYPELESS; td.BindFlags=D3D11_BIND_DEPTH_STENCIL;
    ComPtr<ID3D11Texture2D> depth, staging, colour;
    if (FAILED(gpu->CreateTexture2D(&td,nullptr,&depth))) return false;
    td.Usage=D3D11_USAGE_STAGING; td.BindFlags=0; td.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    if (FAILED(gpu->CreateTexture2D(&td,nullptr,&staging))) return false;
    td.Format=DXGI_FORMAT_R8G8B8A8_UNORM; td.Usage=D3D11_USAGE_DEFAULT;
    td.BindFlags=D3D11_BIND_RENDER_TARGET; td.CPUAccessFlags=0;
    if (FAILED(gpu->CreateTexture2D(&td,nullptr,&colour))) return false;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<ID3D11DepthStencilView> dsv;
    D3D11_DEPTH_STENCIL_VIEW_DESC vd = {};
    vd.Format=DXGI_FORMAT_D24_UNORM_S8_UINT; vd.ViewDimension=D3D11_DSV_DIMENSION_TEXTURE2D;
    if (FAILED(gpu->CreateDepthStencilView(depth.Get(),&vd,&dsv)) ||
        FAILED(gpu->CreateRenderTargetView(colour.Get(),nullptr,&rtv))) return false;
    ComPtr<ID3D11RenderTargetView> oldRT;
    ComPtr<ID3D11DepthStencilView> oldDS;
    ComPtr<ID3D11DepthStencilState> oldDepth;
    ComPtr<ID3D11RasterizerState> oldRaster;
    ComPtr<ID3D11VertexShader> oldVS; ComPtr<ID3D11PixelShader> oldPS;
    ComPtr<ID3D11InputLayout> oldLayout;
    UINT oldRef=0, viewportCount=1; D3D11_VIEWPORT oldViewport = {};
    D3D11_PRIMITIVE_TOPOLOGY oldTopology;
    context->OMGetRenderTargets(1,&oldRT,&oldDS); context->OMGetDepthStencilState(&oldDepth,&oldRef);
    context->RSGetState(&oldRaster); context->RSGetViewports(&viewportCount,&oldViewport);
    context->VSGetShader(&oldVS,nullptr,nullptr); context->PSGetShader(&oldPS,nullptr,nullptr);
    context->IAGetInputLayout(&oldLayout); context->IAGetPrimitiveTopology(&oldTopology);
    ID3D11RenderTargetView* target=rtv.Get(); context->OMSetRenderTargets(1,&target,dsv.Get());
    const D3D11_VIEWPORT viewport = {0,0,16,8,0,1}; context->RSSetViewports(1,&viewport);
    context->VSSetShader(vs.Get(),nullptr,0); context->PSSetShader(ps.Get(),nullptr,0);
    context->IASetInputLayout(nullptr); context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    const unsigned expected[6][2] = {{3,3},{3,8},{0,0},{6,8},{248,6},{7,7}};
    unsigned checks=0;
    for (unsigned orientation=0; orientation<2; ++orientation)
    {
        D3D11_RASTERIZER_DESC raster = {};
        raster.FillMode=D3D11_FILL_SOLID; raster.CullMode=D3D11_CULL_NONE;
        raster.DepthClipEnable=TRUE; raster.FrontCounterClockwise=orientation;
        ComPtr<ID3D11RasterizerState> rs;
        if (FAILED(gpu->CreateRasterizerState(&raster,&rs))) return false;
        context->RSSetState(rs.Get());
        for (unsigned test=0; test<6; ++test)
        {
            D3D11_DEPTH_STENCIL_DESC desc = {};
            desc.DepthEnable=test==4; desc.DepthFunc=D3D11_COMPARISON_NEVER;
            desc.StencilEnable=test!=5; desc.StencilReadMask=desc.StencilWriteMask=255;
            desc.FrontFace={D3D11_STENCIL_OP_DECR_SAT,D3D11_STENCIL_OP_INVERT,
                test==2 ? D3D11_STENCIL_OP_ZERO : D3D11_STENCIL_OP_REPLACE,
                test==3 ? D3D11_COMPARISON_NEVER : D3D11_COMPARISON_ALWAYS};
            desc.BackFace={D3D11_STENCIL_OP_KEEP,D3D11_STENCIL_OP_DECR_SAT,
                D3D11_STENCIL_OP_INCR_SAT,D3D11_COMPARISON_ALWAYS};
            Display::D3D11::ResolveStencilFaces(desc,test!=0 && test!=2,orientation!=0);
            ComPtr<ID3D11DepthStencilState> state;
            if (FAILED(gpu->CreateDepthStencilState(&desc,&state))) return false;
            context->OMSetDepthStencilState(state.Get(),3);
            context->ClearDepthStencilView(dsv.Get(),D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,.5f,7);
            context->Draw(6,0);
            context->CopyResource(staging.Get(),depth.Get());
            D3D11_MAPPED_SUBRESOURCE mapped = {};
            if (FAILED(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped))) return false;
            const auto* row=reinterpret_cast<const UINT*>(static_cast<const unsigned char*>(mapped.pData)+5*mapped.RowPitch);
            const unsigned actual[] = {row[2]>>24,row[10]>>24};
            context->Unmap(staging.Get(),0);
            for (unsigned face=0; face<2; ++face,++checks)
                if (actual[face]!=expected[test][face])
                {
                    std::fprintf(stderr,"Stencil orientation=%u test=%u face=%u: %u expected %u\n",
                        orientation,test,face,actual[face],expected[test][face]);
                    return false;
                }
        }
    }
    target=oldRT.Get(); context->OMSetRenderTargets(1,&target,oldDS.Get());
    context->OMSetDepthStencilState(oldDepth.Get(),oldRef); context->RSSetState(oldRaster.Get());
    context->RSSetViewports(viewportCount,&oldViewport);
    context->VSSetShader(oldVS.Get(),nullptr,0); context->PSSetShader(oldPS.Get(),nullptr,0);
    context->IASetInputLayout(oldLayout.Get()); context->IASetPrimitiveTopology(oldTopology);
    std::printf("PC stencil faces: %u GPU checks passed (one/two-sided, winding, stencil/depth failures)\n",checks);
    return true;
}

int main()
{
    Display::D3D11::C_D3D11Device device;
    HRESULT result = device.Initialise();
    if (FAILED(result)) return 1;
    if (!CheckPackedPolygonFont(device)) return 10;
    if (!CheckPackedPolygonShaders(device)) return 15;
    UINT packedColourSupport = 0;
    result = device.Device()->CheckFormatSupport(DXGI_FORMAT_B8G8R8A8_UNORM,
                                                  &packedColourSupport);
    if (FAILED(result)) return 1;
    std::printf("BGRA packed vertex colour input: %s\n",
                (packedColourSupport & D3D11_FORMAT_SUPPORT_IA_VERTEX_BUFFER) ?
                "supported" : "requires native upload conversion");

    const unsigned char volumeTexels[32] = {
        0, 0, 0, 255, 32, 0, 0, 255, 64, 0, 0, 255, 96, 0, 0, 255,
        128, 0, 0, 255, 160, 0, 0, 255, 192, 0, 0, 255, 224, 0, 0, 255
    };
    const void* volumeMips[] = { volumeTexels };
    const UINT volumeSizes[] = { sizeof(volumeTexels) };
    Microsoft::WRL::ComPtr<ID3D11Texture3D> volumeTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> volumeView;
    result = device.CreateTexture3DFromPCData(2, 2, 2, 1, D3DFMT_A8R8G8B8,
                                               volumeMips, volumeSizes,
                                               volumeTexture, volumeView);
    if (FAILED(result)) return 1;
    D3D11_SHADER_RESOURCE_VIEW_DESC volumeViewDesc = {};
    volumeView->GetDesc(&volumeViewDesc);
    if (volumeViewDesc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE3D) return 1;

    const unsigned char cubeFaces[6][4] = {
        { 255, 0, 0, 255 }, { 0, 255, 0, 255 }, { 0, 0, 255, 255 },
        { 255, 255, 0, 255 }, { 255, 0, 255, 255 }, { 0, 255, 255, 255 }
    };
    const void* cubeMips[6] = { cubeFaces[0], cubeFaces[1], cubeFaces[2],
                                 cubeFaces[3], cubeFaces[4], cubeFaces[5] };
    const UINT cubeSizes[6] = { 4, 4, 4, 4, 4, 4 };
    Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeView;
    result = device.CreateTextureCubeFromPCData(1, 1, D3DFMT_A8R8G8B8,
                                                 cubeMips, cubeSizes, cubeTexture,
                                                 cubeView);
    if (FAILED(result)) return 1;
    D3D11_SHADER_RESOURCE_VIEW_DESC cubeViewDesc = {};
    cubeView->GetDesc(&cubeViewDesc);
    if (cubeViewDesc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURECUBE) return 1;

    Display::D3D11::C_D3D11EffectLibrary effectLibrary;
    result = effectLibrary.Open(L"_work/build/gen/fx11/effects");
    if (FAILED(result) || effectLibrary.EffectCount() <= 1254) return 9;
    if (!CheckRoadPrepass(device, effectLibrary)) return 13;
    if (!CheckBillboardPrepass(device, effectLibrary)) return 14;
    if (!CheckVehicleVolumeBinding(device, effectLibrary)) return 15;
    std::string failedEffect;
    result = effectLibrary.PreloadAll(device.Device(), &failedEffect);
    if (FAILED(result))
    {
        std::fprintf(stderr, "FX11 effect %s failed to load: 0x%08lx\n",
                     failedEffect.c_str(), static_cast<unsigned long>(result));
        return 9;
    }

    // [Blur1PCDX11] Technique Define annotations must change the compiled
    // shader, as Fx_export.cpp:WriteEffect did for the shipped PC effects.
    if (!CheckSceneSamplerDefaults(device, effectLibrary)) return 9;
    if (!CheckLiveSamplers(device, effectLibrary)) return 9;
    Display::D3D11::C_D3D11Effect* roadLights = nullptr;
    result = effectLibrary.Get(device.Device(), "Road.fx", &roadLights, "MaterialLights");
    if (FAILED(result)) return 9;
    const void* roadShader = nullptr;
    SIZE_T roadShaderBytes = 0;
    result = roadLights->GetPassPixelShaderBytecode("MaterialLights", 0, &roadShader, &roadShaderBytes);
    if (FAILED(result)) return 9;
    Microsoft::WRL::ComPtr<ID3D11ShaderReflection> roadReflection;
    result = D3DReflect(roadShader, roadShaderBytes, IID_PPV_ARGS(&roadReflection));
    if (FAILED(result)) return 9;
    D3D11_SHADER_DESC roadDesc = {};
    if (FAILED(roadReflection->GetDesc(&roadDesc))) return 9;
    bool hasDynamicLights = false;
    for (UINT binding = 0; binding < roadDesc.BoundResources; ++binding)
    {
        D3D11_SHADER_INPUT_BIND_DESC input = {};
        if (FAILED(roadReflection->GetResourceBindingDesc(binding, &input))) return 9;
        // FXC's legacy sampler syntax names both the texture and sampler after
        // DynamicLightsSampler; enumerate to distinguish the two bindings.
        hasDynamicLights |= input.Type == D3D_SIT_TEXTURE &&
            std::string(input.Name) == "DynamicLightsSampler";
    }
    if (!hasDynamicLights) return 9;
    std::printf("Road MaterialLights technique defines: dynamic-light texture present\n");
    Display::D3D11::C_D3D11Effect* gameEffect = nullptr;
    result = effectLibrary.Get(device.Device(), "ScreenFacingSprite.fx", &gameEffect);
    if (FAILED(result) || !gameEffect) return 9;
    const void* materialSignature = nullptr;
    SIZE_T materialSignatureBytes = 0;
    result = gameEffect->GetPassInputSignature("Material", 0,
                                               &materialSignature, &materialSignatureBytes);
    if (FAILED(result) || !materialSignatureBytes) return 9;
    std::printf("FX11 indexed %zu game effects; Material pass has %zu input-signature bytes\n",
                effectLibrary.EffectCount(), materialSignatureBytes);
    // [Blur1PCDX11] FXB/render stages own fixed-function state. Reapplying FX11
    // for immediate parameters must preserve reverse depth, stencil, blend
    // masks/factors and raster overrides, even for authored ZWriteEnable passes.
    Microsoft::WRL::ComPtr<ID3D11BlendState> gameBlend;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> gameDepth;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> gameRaster;
    D3D11_BLEND_DESC blendDesc = {};
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    D3D11_RASTERIZER_DESC rasterDesc = {};
    if (!Display::D3D11::MakeBlendDesc(D3DBLEND_ZERO, D3DBLEND_SRCCOLOR, D3DBLENDOP_ADD,
        D3DBLEND_ONE, D3DBLEND_ZERO, D3DBLENDOP_ADD, true, 5, &blendDesc) ||
        !Display::D3D11::MakeDepthStencilDesc(true, false, D3DCMP_GREATEREQUAL,
        true, D3DCMP_EQUAL, D3DSTENCILOP_KEEP, D3DSTENCILOP_KEEP, D3DSTENCILOP_REPLACE,
        0x7f, 0x3f, &depthDesc) ||
        !Display::D3D11::MakeRasterizerDesc(D3DFILL_WIREFRAME, D3DCULL_CW, true, &rasterDesc) ||
        FAILED(device.Device()->CreateBlendState(&blendDesc, &gameBlend)) ||
        FAILED(device.Device()->CreateDepthStencilState(&depthDesc, &gameDepth)) ||
        FAILED(device.Device()->CreateRasterizerState(&rasterDesc, &gameRaster))) return 9;
    const FLOAT expectedFactor[4] = {.25f, .5f, .75f, 1.f};
    device.Context()->OMSetBlendState(gameBlend.Get(), expectedFactor, 0x1357);
    device.Context()->OMSetDepthStencilState(gameDepth.Get(), 37);
    device.Context()->RSSetState(gameRaster.Get());
    const char* stateEffects[] = {"MeshEffect.fx", "Puddle_LN_NV.fx"};
    const char* stateTechniques[] = {"Material", "DefaultLights"};
    for (unsigned i = 0; i < 2; ++i)
    {
        Display::D3D11::C_D3D11Effect* stateEffect = nullptr;
        if (FAILED(effectLibrary.Get(device.Device(), stateEffects[i], &stateEffect))) return 9;
        for (unsigned repeat = 0; repeat < 2; ++repeat)
        {
            if (FAILED(stateEffect->Apply(stateTechniques[i], 0, device.Context()))) return 9;
            Microsoft::WRL::ComPtr<ID3D11BlendState> actualBlend;
            Microsoft::WRL::ComPtr<ID3D11DepthStencilState> actualDepth;
            Microsoft::WRL::ComPtr<ID3D11RasterizerState> actualRaster;
            FLOAT actualFactor[4] = {};
            UINT actualMask = 0, actualReference = 0;
            device.Context()->OMGetBlendState(&actualBlend, actualFactor, &actualMask);
            device.Context()->OMGetDepthStencilState(&actualDepth, &actualReference);
            device.Context()->RSGetState(&actualRaster);
            if (actualBlend.Get() != gameBlend.Get() || actualDepth.Get() != gameDepth.Get() ||
                actualRaster.Get() != gameRaster.Get() || actualMask != 0x1357 || actualReference != 37 ||
                memcmp(actualFactor, expectedFactor, sizeof(expectedFactor)))
            { std::fprintf(stderr, "FX11 overwrote engine state: %s repeat=%u\n", stateEffects[i], repeat); return 9; }
        }
    }
    std::printf("FX11 engine state preserved across MeshEffect/Puddle and immediate reapply\n");
    device.Context()->ClearState(); D3DX11InvalidateBoundState(3,15);
    Display::D3D11::C_D3D11Effect* alphaEffect = nullptr;
    result = effectLibrary.Get(device.Device(), "AutoScrollFade.fx", &alphaEffect);
    if (FAILED(result) || !alphaEffect) return 9;
    const void* alphaBytecode = nullptr;
    SIZE_T alphaBytes = 0;
    result = alphaEffect->GetPassPixelShaderBytecode("Material", 0,
                                                     &alphaBytecode, &alphaBytes);
    Microsoft::WRL::ComPtr<ID3DBlob> alphaAssembly;
    if (SUCCEEDED(result)) result = D3DDisassemble(alphaBytecode, alphaBytes, 0,
                                                    nullptr, &alphaAssembly);
    if (FAILED(result) || !alphaAssembly ||
        std::string(static_cast<const char*>(alphaAssembly->GetBufferPointer()),
                    alphaAssembly->GetBufferSize()).find("discard") == std::string::npos)
        return 9;
    if (SUCCEEDED(result)) result = device.CreateOffscreenTarget(64, 64);
    if (FAILED(result)) { std::fprintf(stderr, "Material target: %08lx\n", result); return 10; }

    // Draw through an FX5 pass compiled from the game's authored material.
    const float identity[16] = { 1, 0, 0, 0, 0, 1, 0, 0,
                                 0, 0, 1, 0, 0, 0, 0, 1 };
    const char* matrixNames[] = { "g_mWorld", "g_mView", "g_mViewProj", "g_mProj" };
    for (const char* name : matrixNames)
    {
        if (gameEffect->HasParameter(name))
        {
            result = gameEffect->SetRawParameter(name, identity, sizeof(identity));
            if (FAILED(result)) { std::fprintf(stderr, "Material matrix %s: %08lx\n", name, result); return 10; }
        }
    }
    if (gameEffect->HasParameter("g_lodFade"))
    {
        const float fade[4] = { 1, 1, 1, 1 };
        result = gameEffect->SetRawParameter("g_lodFade", fade, sizeof(fade));
        if (FAILED(result)) { std::fprintf(stderr, "Material LOD fade: %08lx\n", result); return 10; }
    }
    const unsigned char whiteBGRA[4] = { 255, 255, 255, 255 };
    const void* whiteMip[] = { whiteBGRA };
    const UINT whiteMipSize[] = { sizeof(whiteBGRA) };
    Microsoft::WRL::ComPtr<ID3D11Texture2D> whiteTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> whiteView;
    bool materialAlbedoSRGB = false;
    result = effectLibrary.TextureUsesSRGB("ScreenFacingSprite.fx", "g_tAlbedo",
                                            &materialAlbedoSRGB);
    if (FAILED(result) || !materialAlbedoSRGB) { std::fprintf(stderr, "Material sRGB metadata: %08lx flag=%u\n", result, materialAlbedoSRGB); return 10; }
    result = device.CreateTexture2DFromPCData(1, 1, 1, D3DFMT_A8R8G8B8,
                                              whiteMip, whiteMipSize,
                                              whiteTexture, whiteView,
                                              materialAlbedoSRGB);
    if (FAILED(result)) { std::fprintf(stderr, "Material texture creation: %08lx\n", result); return 10; }
    result = gameEffect->SetTexture("g_tAlbedo", whiteView.Get());
    if (FAILED(result)) { std::fprintf(stderr, "Material texture binding: %08lx\n", result); return 10; }

    const D3DVERTEXELEMENT9 materialElements[] = {
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
        { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        { 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
        { 0, 48, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3 }
    };
    struct MaterialVertex { float x, y, z, nx, ny, nz, u, v, r, g, b, a, px, py; };
    const MaterialVertex materialVertices[] = {
        { 0, 0.8f, 0.2f, 0, 0, 1, 0.5f, 0, 1, 1, 1, 1, 0, 0 },
        { 0.8f, -0.8f, 0.2f, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
        { -0.8f, -0.8f, 0.2f, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0 }
    };
    const unsigned short materialIndices[] = { 0, 1, 2 };
    Microsoft::WRL::ComPtr<ID3D11Buffer> materialVB, materialIB;
    result = device.CreateVertexBuffer(sizeof(materialVertices), materialVertices, false,
                                       materialVB);
    if (SUCCEEDED(result)) result = device.CreateIndexBuffer(
        sizeof(materialIndices), materialIndices, false, materialIB);
    if (FAILED(result)) return 10;
    D3D11_RASTERIZER_DESC materialRaster = {};
    if (!Display::D3D11::MakeRasterizerDesc(D3DFILL_SOLID, D3DCULL_NONE, false,
                                           &materialRaster)) return 10;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> materialRasterState;
    result = device.Device()->CreateRasterizerState(&materialRaster, &materialRasterState);
    if (FAILED(result)) return 10;
    const float materialClear[4] = { 0, 0, 0, 1 };
    device.Context()->ClearRenderTargetView(device.RenderTargetView(), materialClear);
    device.Context()->ClearDepthStencilView(device.DepthStencilView(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
    ID3D11RenderTargetView* materialTarget = device.RenderTargetView();
    device.Context()->OMSetRenderTargets(1, &materialTarget, device.DepthStencilView());
    D3D11_VIEWPORT materialViewport = { 0, 0, 64, 64, 0, 1 };
    device.Context()->RSSetViewports(1, &materialViewport);
    device.Context()->RSSetState(materialRasterState.Get());
    result = device.DrawGameEffectIndexed(*gameEffect, "Material", 0,
                                          materialElements, 5, materialVB.Get(),
                                          sizeof(MaterialVertex), materialIB.Get(),
                                          DXGI_FORMAT_R16_UINT, 3);
    if (FAILED(result) || device.InputLayoutCount() != 1)
    { std::fprintf(stderr, "Material draw: %08lx layouts=%zu\n", result, device.InputLayoutCount()); return 10; }
    result = device.DrawGameEffectIndexed(*gameEffect, "Material", 0,
                                          materialElements, 5, materialVB.Get(),
                                          sizeof(MaterialVertex), materialIB.Get(),
                                          DXGI_FORMAT_R16_UINT, 3);
    if (FAILED(result) || device.InputLayoutCount() != 1)
    { std::fprintf(stderr, "Material cached draw: %08lx layouts=%zu\n", result, device.InputLayoutCount()); return 10; }
    ID3D11SamplerState* boundSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] = {};
    device.Context()->PSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT,
                                    boundSamplers);
    bool linearSamplerBound = false;
    for (ID3D11SamplerState* bound : boundSamplers)
    {
        if (bound)
        {
            D3D11_SAMPLER_DESC samplerDesc = {};
            bound->GetDesc(&samplerDesc);
            linearSamplerBound |= samplerDesc.Filter == D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            bound->Release();
        }
    }
    if (!linearSamplerBound) { std::fprintf(stderr, "Material linear sampler missing\n"); return 10; }
    unsigned char materialPixel[4] = {};
    result = device.ReadPixel(32, 32, materialPixel);
    device.Context()->ClearState(); D3DX11InvalidateBoundState(3,15);
    if (FAILED(result)) return 10;
    std::printf("FX11 material draw center RGBA %u %u %u %u\n",
                materialPixel[0], materialPixel[1], materialPixel[2], materialPixel[3]);
    if (materialPixel[0] < 200 || materialPixel[1] < 200 || materialPixel[2] < 200)
        return 10;

    // The same generated exit must run for FX11 effects, not just built-ins.
    const UINT alphaEnabled = 1, alphaNever = D3D11_COMPARISON_NEVER;
    if (FAILED(gameEffect->SetRawParameter("BlurPCAlphaEnabled", &alphaEnabled, sizeof(alphaEnabled))) ||
        FAILED(gameEffect->SetRawParameter("BlurPCAlphaFunction", &alphaNever, sizeof(alphaNever)))) return 12;
    device.Context()->ClearRenderTargetView(device.RenderTargetView(), materialClear);
    device.Context()->ClearDepthStencilView(device.DepthStencilView(), D3D11_CLEAR_DEPTH, 1.f, 0);
    device.Context()->OMSetRenderTargets(1, &materialTarget, device.DepthStencilView());
    device.Context()->RSSetViewports(1, &materialViewport);
    device.Context()->RSSetState(materialRasterState.Get());
    result = device.DrawGameEffectIndexed(*gameEffect, "Material", 0,
        materialElements, 5, materialVB.Get(), sizeof(MaterialVertex), materialIB.Get(), DXGI_FORMAT_R16_UINT, 3);
    if (FAILED(result) || FAILED(device.ReadPixel(32, 32, materialPixel)) || materialPixel[0] != 0) return 12;
    device.Context()->ClearState(); D3DX11InvalidateBoundState(3,15);
    std::printf("FX11 alpha test: previously visible material rejected by Never\n");

    if (!CheckVehiclePrepass(device, effectLibrary)) return 11;
    if (!CheckNativeAlphaTest(device) || !CheckBuiltinShaderLinks() || !CheckAlphaBlendFactors(device) ||
        !CheckTextureGammaViews(device) || !CheckLuminanceTextures(device) || !CheckStencilFaces(device) ||
        !CheckCubeTextureBinding(device)) return 12;

    if (SUCCEEDED(result)) result = device.DrawProbeTriangle(
        L"_work/build/gen/dx11_builtins/display/vsBasicRenderColoured2D.cso",
        L"_work/build/gen/dx11_builtins/display/psBasicRenderColoured.cso");
    unsigned char pixel[4] = {};
    if (SUCCEEDED(result)) result = device.ReadPixel(32, 32, pixel);
    unsigned char corner[4] = {};
    if (SUCCEEDED(result)) result = device.ReadPixel(2, 2, corner);
    if (FAILED(result))
    {
        std::fprintf(stderr, "D3D11 probe failed: HRESULT 0x%08lx\n", static_cast<unsigned long>(result));
        return 1;
    }
    std::printf("D3D11 feature level 0x%x, %s device; center RGBA %u %u %u %u; corner RGBA %u %u %u %u\n",
                static_cast<unsigned>(device.FeatureLevel()),
                device.DriverType() == D3D_DRIVER_TYPE_HARDWARE ? "hardware" : "WARP",
                pixel[0], pixel[1], pixel[2], pixel[3], corner[0], corner[1], corner[2], corner[3]);
    // The center must contain the triangle, rather than the cleared background.
    if (!(pixel[3] == 255 && pixel[0] > 20 && pixel[1] > 20 && pixel[2] > 20 &&
          corner[0] == 5 && corner[1] == 10 && corner[2] == 20 && corner[3] == 255))
        return 2;

    const unsigned char texel[4] = { 20, 150, 200, 255 };
    result = device.DrawTexturedQuad(
        L"_work/build/gen/dx11_builtins/display/vsBasicRenderTextured2D.cso",
        L"_work/build/gen/dx11_builtins/display/psBasicRenderTextured.cso",
        texel);
    if (SUCCEEDED(result)) result = device.ReadPixel(32, 32, pixel);
    if (FAILED(result) || pixel[0] != texel[0] || pixel[1] != texel[1] ||
        pixel[2] != texel[2] || pixel[3] != texel[3])
    {
        std::fprintf(stderr, "D3D11 texture path failed: HRESULT 0x%08lx, pixel %u %u %u %u\n",
                     static_cast<unsigned long>(result), pixel[0], pixel[1], pixel[2], pixel[3]);
        return 5;
    }
    const unsigned char srgbTexel[4] = { 128, 128, 128, 255 };
    result = device.DrawTexturedQuad(
        L"_work/build/gen/dx11_builtins/display/vsBasicRenderTextured2D.cso",
        L"_work/build/gen/dx11_builtins/display/psBasicRenderTextured.cso",
        srgbTexel, true);
    if (SUCCEEDED(result)) result = device.ReadPixel(32, 32, pixel);
    if (FAILED(result) || pixel[0] < 54 || pixel[0] > 56 ||
        pixel[1] < 54 || pixel[1] > 56 || pixel[2] < 54 || pixel[2] > 56)
    {
        std::fprintf(stderr, "D3D11 sRGB decode failed: 0x%08lx, pixel %u %u %u %u\n",
                     static_cast<unsigned long>(result), pixel[0], pixel[1],
                     pixel[2], pixel[3]);
        return 5;
    }
    const unsigned char bc1Block[8] = { 0x00, 0xf8, 0, 0, 0, 0, 0, 0 };
    const unsigned char bc2Block[16] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x00, 0xf8, 0, 0, 0, 0, 0, 0
    };
    const unsigned char bc3Block[16] = {
        0xff, 0, 0, 0, 0, 0, 0, 0,
        0x00, 0xf8, 0, 0, 0, 0, 0, 0
    };
    const struct { D3DFORMAT format; const void* bytes; UINT size; } bcCases[] = {
        { D3DFMT_DXT1, bc1Block, sizeof(bc1Block) },
        { D3DFMT_DXT3, bc2Block, sizeof(bc2Block) },
        { D3DFMT_DXT5, bc3Block, sizeof(bc3Block) }
    };
    for (const auto& bc : bcCases)
    {
        const void* mip[] = { bc.bytes };
        const UINT size[] = { bc.size };
        Microsoft::WRL::ComPtr<ID3D11Texture2D> compressedTexture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> compressedView;
        result = device.CreateTexture2DFromPCData(4, 4, 1, bc.format, mip, size,
                                                   compressedTexture, compressedView);
        if (SUCCEEDED(result)) result = device.DrawTexturedQuadView(
            L"_work/build/gen/dx11_builtins/display/vsBasicRenderTextured2D.cso",
            L"_work/build/gen/dx11_builtins/display/psBasicRenderTextured.cso",
            compressedView.Get());
        if (SUCCEEDED(result)) result = device.ReadPixel(32, 32, pixel);
        if (FAILED(result) || pixel[0] != 255 || pixel[1] != 0 ||
            pixel[2] != 0 || pixel[3] != 255) return 5;
    }

    // Draw with the engine's 3D shader and named world/view-projection matrices.
    const float world[16] = { 1, 0, 0, 0, 0, 1, 0, 0,
                              0, 0, 1, 0, 0, 0, 0, 1 };
    const float viewProj[16] = { 1, 0, 0, 0, 0, 1, 0, 0,
                                 0, 0, 1, 0, 0.65f, 0, 0, 1 };
    struct Vertex3D { float x, y, z, r, g, b, a; };
    const Vertex3D triangle[] = {
        {  0,     0.7f, 0.3f, 0.8f, 0.1f, 0.2f, 1 },
        { -0.25f,-0.5f, 0.3f, 0.8f, 0.1f, 0.2f, 1 },
        {  0.25f,-0.5f, 0.3f, 0.8f, 0.1f, 0.2f, 1 }
    };
    const unsigned short triangleIndices[] = { 0, 1, 2 };
    result = device.DrawColoured3D(
        L"_work/build/gen/dx11_builtins/display/vsBasicRenderColoured3D.cso",
        L"_work/build/gen/dx11_builtins/display/psBasicRenderColoured.cso",
        world, viewProj, triangle, sizeof(triangle), sizeof(Vertex3D),
        triangleIndices, 3);
    unsigned char translated[4] = {};
    if (SUCCEEDED(result)) result = device.ReadPixel(52, 32, translated);
    if (SUCCEEDED(result)) result = device.ReadPixel(32, 32, pixel);
    if (FAILED(result) || translated[0] < 150 || translated[1] < 15 ||
        translated[2] < 30 || pixel[0] != 5 || pixel[1] != 10 || pixel[2] != 20)
    {
        std::fprintf(stderr, "D3D11 3D matrices failed: HRESULT 0x%08lx, translated %u %u %u %u, center %u %u %u %u\n",
                     static_cast<unsigned long>(result), translated[0], translated[1],
                     translated[2], translated[3], pixel[0], pixel[1], pixel[2], pixel[3]);
        return 6;
    }

    // Exercise the render-target/depth views used for game postprocessing.
    Microsoft::WRL::ComPtr<ID3D11Texture2D> msColor, resolvedColor, msDepth;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> msTarget, resolvedTarget;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> msColorView, resolvedView, depthView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> msDepthTarget;
    result = device.CreateRenderTexture2D(64, 64, 1, 4, DXGI_FORMAT_R8G8B8A8_UNORM,
                                          msColor, msTarget, msColorView);
    if (SUCCEEDED(result)) result = device.CreateRenderTexture2D(
        64, 64, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM,
        resolvedColor, resolvedTarget, resolvedView);
    if (SUCCEEDED(result)) result = device.CreateDepthTexture2D(
        64, 64, 4, msDepth, msDepthTarget, depthView);
    if (FAILED(result))
    {
        std::fprintf(stderr, "D3D11 render/depth texture creation failed: 0x%08lx\n",
                     static_cast<unsigned long>(result));
        return 7;
    }
    const float resolveColour[4] = { 0.25f, 0.5f, 0.75f, 1.f };
    device.Context()->ClearRenderTargetView(msTarget.Get(), resolveColour);
    device.Context()->ClearDepthStencilView(msDepthTarget.Get(),
                                             D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
    device.Context()->ResolveSubresource(resolvedColor.Get(), 0, msColor.Get(), 0,
                                         DXGI_FORMAT_R8G8B8A8_UNORM);
    D3D11_TEXTURE2D_DESC stagingDesc = {};
    resolvedColor->GetDesc(&stagingDesc);
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;
    result = device.Device()->CreateTexture2D(&stagingDesc, nullptr, &staging);
    if (SUCCEEDED(result)) device.Context()->CopyResource(staging.Get(), resolvedColor.Get());
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(result)) result = device.Context()->Map(staging.Get(), 0, D3D11_MAP_READ,
                                                          0, &mapped);
    if (SUCCEEDED(result))
    {
        const unsigned char* sample = static_cast<const unsigned char*>(mapped.pData) +
                                      32 * mapped.RowPitch + 32 * 4;
        const bool rightColour = sample[0] >= 63 && sample[0] <= 65 &&
                                 sample[1] >= 127 && sample[1] <= 129 &&
                                 sample[2] >= 190 && sample[2] <= 192 &&
                                 sample[3] == 255;
        device.Context()->Unmap(staging.Get(), 0);
        if (!rightColour) return 8;
    }
    if (FAILED(result)) return 8;

    // Exercise the DXGI presentation and resize path without launching Amax.
    HWND window = CreateWindowExW(0, L"STATIC", L"Blur DX11 probe", WS_OVERLAPPEDWINDOW,
                                  0, 0, 64, 64, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!window) return 3;
    result = device.CreateWindowTarget(window, 64, 64);
    if (SUCCEEDED(result)) result = device.DrawProbeTriangle(
        L"_work/build/gen/dx11_builtins/display/vsBasicRenderColoured2D.cso",
        L"_work/build/gen/dx11_builtins/display/psBasicRenderColoured.cso");
    if (SUCCEEDED(result)) result = device.ReadPixel(32, 32, pixel);
    if (SUCCEEDED(result)) result = device.Present(0);
    if (SUCCEEDED(result)) result = device.ResizeWindowTarget(128, 96);
    if (SUCCEEDED(result)) result = device.DrawProbeTriangle(
        L"_work/build/gen/dx11_builtins/display/vsBasicRenderColoured2D.cso",
        L"_work/build/gen/dx11_builtins/display/psBasicRenderColoured.cso");
    if (SUCCEEDED(result)) result = device.ReadPixel(64, 48, pixel);
    if (SUCCEEDED(result)) result = device.Present(0);
    DestroyWindow(window);
    if (FAILED(result) || pixel[0] < 20 || pixel[1] < 20 || pixel[2] < 20)
    {
        std::fprintf(stderr, "DXGI window path failed: HRESULT 0x%08lx\n", static_cast<unsigned long>(result));
        return 4;
    }
    std::puts("DXGI swap chain create, present and resize passed");
    return 0;
}
