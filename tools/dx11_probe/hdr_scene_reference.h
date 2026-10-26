// [Blur2PC][HDR] Verify actual built-in sky and grading shaders, not only the output container.
#pragma once
#include <fstream>
#include <d3d11shader.h>
#include "Display/Device/D3D11/D3D11Effect.h"
extern void D3DX11InvalidateBoundState(unsigned stages, unsigned what);

static bool CheckFixtureErrors(ID3D11InfoQueue* queue)
{
    bool okay = true;
    if (queue) for (UINT64 i = 0; i < queue->GetNumStoredMessages(); ++i) {
        SIZE_T size = 0; queue->GetMessage(i, nullptr, &size); std::vector<char> bytes(size);
        auto message = reinterpret_cast<D3D11_MESSAGE*>(bytes.data()); queue->GetMessage(i, message, &size);
        if (message->Severity <= D3D11_MESSAGE_SEVERITY_ERROR) {
            printf("HDR shader fixture error: %s\n", message->pDescription); okay = false;
        }
    }
    return okay;
}

// Exercise the generated FX5 shader and external device buffer binding together.
static bool CheckEmissiveRange(ID3D11Device* device, IDXGISwapChain* chain, HWND window, const char* path)
{
    ComPtr<ID3D11InfoQueue> queue; device->QueryInterface(IID_PPV_ARGS(&queue));
    if (queue) queue->ClearStoredMessages();
    std::ifstream file(path, std::ios::binary);
    std::vector<char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    Display::D3D11::C_D3D11Effect effect;
    if (bytes.empty() || FAILED(effect.LoadMemory(device, bytes.data(), bytes.size()))) return false;
    const void* pixelCode = nullptr; SIZE_T pixelSize = 0;
    if (FAILED(effect.GetPassPixelShaderBytecode("MaterialLOD", 0, &pixelCode, &pixelSize))) return false;
    ComPtr<ID3D11ShaderReflection> reflection;
    D3D11_SHADER_INPUT_BIND_DESC binding = {};
    if (FAILED(D3DReflect(pixelCode, pixelSize, IID_PPV_ARGS(&reflection))) ||
        FAILED(reflection->GetResourceBindingDescByName("BlurHDROutput", &binding)) || binding.BindPoint != 12) {
        puts("HDR emissive FX5 shader lost the device's b12 binding"); return false;
    }
    const char* vertex = "struct Out { float4 p:SV_Position; float4 c:TEXCOORD1; float2 uv:TEXCOORD0; }; Out main(uint id:SV_VertexID) { Out o; o.p=float4(id==2?3:-1,id==1?3:-1,0,1); o.uv=float2(.5,.5); o.c=float4(4,2,1,1); return o; }";
    ComPtr<ID3DBlob> code, errors; ComPtr<ID3D11VertexShader> vs;
    if (FAILED(D3DCompile(vertex, strlen(vertex), "HDREmissiveVS", nullptr, nullptr, "main", "vs_5_0", 0, 0, &code, &errors)) ||
        FAILED(device->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &vs))) return false;
    D3D11_TEXTURE2D_DESC desc = {}; desc.Width = desc.Height = desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    const unsigned short white[] = {0x3c00,0x3c00,0x3c00,0x3c00};
    D3D11_SUBRESOURCE_DATA initial = {}; initial.pSysMem = white; initial.SysMemPitch = 8;
    ComPtr<ID3D11Texture2D> texture, target, read;
    ComPtr<ID3D11ShaderResourceView> srv; ComPtr<ID3D11RenderTargetView> rtv;
    if (FAILED(device->CreateTexture2D(&desc, &initial, &texture)) ||
        FAILED(device->CreateShaderResourceView(texture.Get(), nullptr, &srv)) ||
        FAILED(effect.SetTexture("g_tTexture", srv.Get()))) return false;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    if (FAILED(device->CreateTexture2D(&desc, nullptr, &target)) ||
        FAILED(device->CreateRenderTargetView(target.Get(), nullptr, &rtv))) return false;
    desc.BindFlags = 0; desc.Usage = D3D11_USAGE_STAGING; desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(device->CreateTexture2D(&desc, nullptr, &read))) return false;
    ComPtr<ID3D11DeviceContext> context; device->GetImmediateContext(&context);
    C_HDROutput output(window);
    ComPtr<ID3D11Texture2D> frame;
    if (FAILED(output.CreateTargets(device, chain, &frame))) return false;
    for (bool hdr : {false, true}) {
        context->ClearState(); D3DX11InvalidateBoundState(3, 15);
        const float alpha = 1;
        if (FAILED(effect.SetRawParameter("g_fAlphaFactor", &alpha, sizeof(alpha))) ||
            FAILED(effect.Apply("MaterialLOD", 0, context.Get()))) return false;
        output.Calibrate(hdr, 1000, 200); output.Refresh(device, true); output.Bind(context.Get());
        context->VSSetShader(vs.Get(), nullptr, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->OMSetBlendState(nullptr, nullptr, ~0u); context->OMSetDepthStencilState(nullptr, 0);
        ID3D11RenderTargetView* view = rtv.Get(); context->OMSetRenderTargets(1, &view, nullptr);
        D3D11_VIEWPORT vp = {0,0,1,1,0,1}; context->RSSetViewports(1, &vp); context->Draw(3,0);
        context->CopyResource(read.Get(), target.Get()); D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (FAILED(context->Map(read.Get(), 0, D3D11_MAP_READ, 0, &mapped))) return false;
        float rgb[3]; for (UINT c = 0; c < 3; ++c)
            rgb[c] = DirectX::PackedVector::XMConvertHalfToFloat(static_cast<unsigned short*>(mapped.pData)[c]);
        context->Unmap(read.Get(), 0);
        printf("HDR actual racification FX5: enabled=%u RGB=%g,%g,%g\n", hdr, rgb[0], rgb[1], rgb[2]);
        if (fabsf(rgb[0] - (hdr ? 4.f : 1.f)) > .01f || fabsf(rgb[1] - (hdr ? 2.f : 1.f)) > .01f || fabsf(rgb[2] - 1.f) > .01f) return false;
    }
    context->ClearState(); D3DX11InvalidateBoundState(3, 15); return CheckFixtureErrors(queue.Get());
}
static bool CheckSceneRange(ID3D11Device* device, IDXGISwapChain* chain, HWND window, const char* root)
{
    ComPtr<ID3D11InfoQueue> queue; device->QueryInterface(IID_PPV_ARGS(&queue));
    if (queue) queue->ClearStoredMessages();
    auto loadPS = [&](const char* suffix, ComPtr<ID3D11PixelShader>& shader) {
        std::ifstream file(std::string(root) + suffix, std::ios::binary);
        std::vector<char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return !bytes.empty() && SUCCEEDED(device->CreatePixelShader(bytes.data(), bytes.size(), nullptr, &shader));
    };
    ComPtr<ID3D11PixelShader> sky, tone;
    if (!loadPS("/effect/psTexturedSkyDomeHDR.cso", sky) || !loadPS("/amax/psToneMapCustomHDRU32.cso", tone)) {
        puts("HDR scene reference: missing built-in shaders"); return false;
    }
    const char* vertex = "struct Out { float4 sky:COLOR0; float4 p:SV_Position; }; Out main(uint id:SV_VertexID) { Out o; o.p=float4(id==2?3:-1,id==1?3:-1,0,1); o.sky=float4(.5,.5,0,1); return o; }";
    ComPtr<ID3DBlob> code, errors; ComPtr<ID3D11VertexShader> vs;
    if (FAILED(D3DCompile(vertex, strlen(vertex), "HDRSceneVS", nullptr, nullptr, "main", "vs_5_0", 0, 0, &code, &errors)) ||
        FAILED(device->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &vs))) return false;
    const char* toneVertex = "struct Out { float2 uv:TEXCOORD0; float4 p:SV_Position; }; Out main(uint id:SV_VertexID) { Out o; o.p=float4(id==2?3:-1,id==1?3:-1,0,1); o.uv=float2(.5,.5); return o; }";
    ComPtr<ID3D11VertexShader> toneVS;
    if (FAILED(D3DCompile(toneVertex, strlen(toneVertex), "HDRToneVS", nullptr, nullptr, "main", "vs_5_0", 0, 0, &code, &errors)) ||
        FAILED(device->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &toneVS))) return false;
    D3D11_TEXTURE2D_DESC desc = {}; desc.Width = desc.Height = desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    auto pixelTexture = [&](float r, float g, float b, float a, ComPtr<ID3D11ShaderResourceView>& view) {
        const unsigned short pixel[] = {DirectX::PackedVector::XMConvertFloatToHalf(r), DirectX::PackedVector::XMConvertFloatToHalf(g), DirectX::PackedVector::XMConvertFloatToHalf(b), DirectX::PackedVector::XMConvertFloatToHalf(a)};
        D3D11_SUBRESOURCE_DATA initial = {}; initial.pSysMem = pixel; initial.SysMemPitch = 8;
        ComPtr<ID3D11Texture2D> texture;
        return SUCCEEDED(device->CreateTexture2D(&desc, &initial, &texture)) && SUCCEEDED(device->CreateShaderResourceView(texture.Get(), nullptr, &view));
    };
    ComPtr<ID3D11ShaderResourceView> source, black;
    if (!pixelTexture(1, .5f, .25f, 1, source) || !pixelTexture(0, 0, 0, 1, black)) return false;
    desc.Width = 80; desc.Height = 40; desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    ComPtr<ID3D11Texture2D> scene; ComPtr<ID3D11RenderTargetView> sceneRT; ComPtr<ID3D11ShaderResourceView> sceneSRV;
    if (FAILED(device->CreateTexture2D(&desc, nullptr, &scene)) || FAILED(device->CreateRenderTargetView(scene.Get(), nullptr, &sceneRT)) ||
        FAILED(device->CreateShaderResourceView(scene.Get(), nullptr, &sceneSRV))) return false;
    std::vector<unsigned short> lutPixels(32 * 32 * 32 * 4);
    for (int z=0; z<32; ++z) for (int y=0; y<32; ++y) for (int x=0; x<32; ++x) {
        auto p=&lutPixels[((z*32+y)*32+x)*4]; p[0]=DirectX::PackedVector::XMConvertFloatToHalf(x/31.f);
        p[1]=DirectX::PackedVector::XMConvertFloatToHalf(y/31.f); p[2]=DirectX::PackedVector::XMConvertFloatToHalf(z/31.f); p[3]=0x3c00;
    }
    D3D11_TEXTURE3D_DESC ld = {}; ld.Width=ld.Height=ld.Depth=32; ld.MipLevels=1;
    ld.Format=DXGI_FORMAT_R16G16B16A16_FLOAT; ld.BindFlags=D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA li={}; li.pSysMem=lutPixels.data(); li.SysMemPitch=32*8; li.SysMemSlicePitch=32*32*8;
    ComPtr<ID3D11Texture3D> lut; ComPtr<ID3D11ShaderResourceView> lutSRV;
    if (FAILED(device->CreateTexture3D(&ld,&li,&lut)) || FAILED(device->CreateShaderResourceView(lut.Get(),nullptr,&lutSRV))) return false;
    D3D11_SAMPLER_DESC sm={}; sm.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR; sm.AddressU=sm.AddressV=sm.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP; sm.MaxLOD=D3D11_FLOAT32_MAX;
    ComPtr<ID3D11SamplerState> sampler; if (FAILED(device->CreateSamplerState(&sm,&sampler))) return false;
    D3D11_BUFFER_DESC bd={}; bd.ByteWidth=4096; bd.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
    ComPtr<ID3D11Buffer> constants; if (FAILED(device->CreateBuffer(&bd,nullptr,&constants))) return false;
    bd.ByteWidth=16; const unsigned zero[4]={}; D3D11_SUBRESOURCE_DATA zi={}; zi.pSysMem=zero;
    ComPtr<ID3D11Buffer> alpha; if (FAILED(device->CreateBuffer(&bd,&zi,&alpha))) return false;
    SetEnvironmentVariableA("BLUR_DX11_HDR_TEST","1"); SetEnvironmentVariableA("BLUR_DX11_HDR_PEAK_NITS","1000"); SetEnvironmentVariableA("BLUR_DX11_HDR_PAPER_WHITE_NITS","200");
    C_HDROutput output(window); ComPtr<ID3D11Texture2D> frame;
    if (FAILED(output.CreateTargets(device,chain,&frame))) return false;
    ComPtr<ID3D11RenderTargetView> frameRT; if (FAILED(device->CreateRenderTargetView(frame.Get(),nullptr,&frameRT))) return false;
    ComPtr<ID3D11Texture2D> back, read;
    if (FAILED(chain->GetBuffer(0,IID_PPV_ARGS(&back)))) return false;
    back->GetDesc(&desc); desc.Usage=D3D11_USAGE_STAGING; desc.BindFlags=0; desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    if (FAILED(device->CreateTexture2D(&desc,nullptr,&read))) return false;
    ComPtr<ID3D11DeviceContext> context; device->GetImmediateContext(&context);
    float previous=0;
    for (float intensity : {0.f,1.f,4.f,16.f}) {
        context->ClearState(); context->VSSetShader(vs.Get(),nullptr,0); context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        D3D11_VIEWPORT viewport={0,0,80,40,0,1}; context->RSSetViewports(1,&viewport);
        ID3D11Buffer* cb=constants.Get(); context->PSSetConstantBuffers(0,1,&cb); cb=alpha.Get(); context->PSSetConstantBuffers(13,1,&cb);
        float values[256][4]={}; values[0][1]=intensity; values[31][0]=values[31][1]=1;
        context->UpdateSubresource(constants.Get(),0,nullptr,values,0,0);
        ID3D11RenderTargetView* target=sceneRT.Get(); context->OMSetRenderTargets(1,&target,nullptr);
        context->VSSetShader(vs.Get(),nullptr,0); context->PSSetShader(sky.Get(),nullptr,0); ID3D11ShaderResourceView* sr=source.Get(); context->PSSetShaderResources(0,1,&sr);
        ID3D11SamplerState* sam=sampler.Get(); context->PSSetSamplers(0,1,&sam); context->Draw(3,0);

        target=frameRT.Get(); context->OMSetRenderTargets(1,&target,nullptr);
        memset(values,0,sizeof(values)); values[0][1]=1.f/2.2f; values[0][2]=2.2f;
        for(int i=0;i<4;++i) values[2+i][i]=1;
        context->UpdateSubresource(constants.Get(),0,nullptr,values,0,0);
        context->VSSetShader(toneVS.Get(),nullptr,0); context->PSSetShader(tone.Get(),nullptr,0); output.Bind(context.Get());
        ID3D11ShaderResourceView* views[11]={}; views[0]=sceneSRV.Get(); views[1]=views[2]=views[4]=black.Get(); views[3]=lutSRV.Get(); // fxc packs textures separately from legacy sampler registers.
        context->PSSetShaderResources(0,11,views); ID3D11SamplerState* sams[11]; for(auto& s:sams) s=sampler.Get(); context->PSSetSamplers(0,11,sams);
        context->Draw(3,0); output.Compose(device); context->CopyResource(read.Get(),back.Get());
        D3D11_MAPPED_SUBRESOURCE mapped={}; if(FAILED(context->Map(read.Get(),0,D3D11_MAP_READ,0,&mapped))) return false;
        float nits=DirectX::PackedVector::XMConvertHalfToFloat(static_cast<unsigned short*>(mapped.pData)[0])*80;
        context->Unmap(read.Get(),0);
        if (!std::isfinite(nits) || nits+1<previous || nits>1001 || (intensity==0 && nits!=0) ||
            (intensity==1 && fabsf(nits-200)>1) || (intensity>1 && nits<=200)) {
            printf("HDR actual sky/grading reference failed: intensity=%g output=%g nits\n",intensity,nits); return false;
        }
        previous=nits; printf("HDR actual sky + grading + presenter: intensity=%g output=%g nits\n",intensity,nits);
    }
    context->ClearState(); output.ReleaseTargets(); return CheckFixtureErrors(queue.Get());
}
