// [Blur2PC][HDR] Exercise production light/blur/bloom shaders and vehicle range paths.
#pragma once
#include "../../code/amax/game/Game/GameWorld/VehicleLightRGBM.h"

struct HDRRangeTarget
{
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11RenderTargetView> target;
    ComPtr<ID3D11ShaderResourceView> view;
};

struct HDRRangeFixture
{
    ID3D11Device* device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<ID3D11VertexShader> vertex, lightVertex;
    ComPtr<ID3D11Buffer> constants, alpha;
    ComPtr<ID3D11SamplerState> sampler;
    float values[256][4] = {};
    explicit HDRRangeFixture(ID3D11Device* d) : device(d) { d->GetImmediateContext(&context); }
    bool Init()
    {
        const char* vs = "struct O{float4 p:SV_Position;float4 uv:TEXCOORD0;};O main(uint id:SV_VertexID){O o;o.p=float4(id==2?3:-1,id==1?3:-1,0,1);o.uv=float4(.5,.5,0,1);return o;}";
        ComPtr<ID3DBlob> code, errors;
        if (FAILED(D3DCompile(vs, strlen(vs), "HDRRangeVS", nullptr, nullptr, "main", "vs_5_0", 0, 0, &code, &errors)) ||
            FAILED(device->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &lightVertex))) return false;
        const char* ordinaryVS = "struct O{float4 uv:TEXCOORD0;float4 p:SV_Position;};O main(uint id:SV_VertexID){O o;o.p=float4(id==2?3:-1,id==1?3:-1,0,1);o.uv=float4(.5,.5,0,1);return o;}";
        if (FAILED(D3DCompile(ordinaryVS, strlen(ordinaryVS), "HDRRangeVS", nullptr, nullptr, "main", "vs_5_0", 0, 0, &code, &errors)) ||
            FAILED(device->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &vertex))) return false;
        D3D11_BUFFER_DESC bd = {}; bd.ByteWidth = sizeof(values); bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        if (FAILED(device->CreateBuffer(&bd, nullptr, &constants))) return false;
        bd.ByteWidth = 16; const unsigned zeros[4] = {}; D3D11_SUBRESOURCE_DATA data = {}; data.pSysMem = zeros;
        if (FAILED(device->CreateBuffer(&bd, &data, &alpha))) return false;
        D3D11_SAMPLER_DESC sd = {}; sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP; sd.MaxLOD = D3D11_FLOAT32_MAX;
        return SUCCEEDED(device->CreateSamplerState(&sd, &sampler));
    }
    bool Target(HDRRangeTarget& result, DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT)
    {
        D3D11_TEXTURE2D_DESC td = {}; td.Width = td.Height = 8; td.MipLevels = td.ArraySize = td.SampleDesc.Count = 1;
        td.Format = format; td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        return SUCCEEDED(device->CreateTexture2D(&td, nullptr, &result.texture)) &&
            SUCCEEDED(device->CreateRenderTargetView(result.texture.Get(), nullptr, &result.target)) &&
            SUCCEEDED(device->CreateShaderResourceView(result.texture.Get(), nullptr, &result.view));
    }
    bool Pixel(float r, float g, float b, float a, ComPtr<ID3D11ShaderResourceView>& view)
    {
        const unsigned short pixel[] = {DirectX::PackedVector::XMConvertFloatToHalf(r), DirectX::PackedVector::XMConvertFloatToHalf(g),
            DirectX::PackedVector::XMConvertFloatToHalf(b), DirectX::PackedVector::XMConvertFloatToHalf(a)};
        D3D11_TEXTURE2D_DESC td = {}; td.Width = td.Height = td.MipLevels = td.ArraySize = td.SampleDesc.Count = 1;
        td.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA data = {}; data.pSysMem = pixel; data.SysMemPitch = 8;
        ComPtr<ID3D11Texture2D> texture;
        return SUCCEEDED(device->CreateTexture2D(&td, &data, &texture)) && SUCCEEDED(device->CreateShaderResourceView(texture.Get(), nullptr, &view));
    }
    bool Load(const std::string& path, ComPtr<ID3D11PixelShader>& result)
    {
        std::ifstream file(path, std::ios::binary); std::vector<char> bytes((std::istreambuf_iterator<char>(file)), {});
        return !bytes.empty() && SUCCEEDED(device->CreatePixelShader(bytes.data(), bytes.size(), nullptr, &result));
    }
    bool Compile(const std::string& source, ComPtr<ID3D11PixelShader>& result)
    {
        ComPtr<ID3DBlob> code, errors;
        if (FAILED(D3DCompile(source.data(), source.size(), "HDRRangePS", nullptr, nullptr, "main", "ps_5_0", 0, 0, &code, &errors))) {
            if (errors) printf("%s\n", (const char*)errors->GetBufferPointer()); return false;
        }
        return SUCCEEDED(device->CreatePixelShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &result));
    }
    void Draw(ID3D11PixelShader* shader, HDRRangeTarget& target, ID3D11ShaderResourceView* const* sources, UINT count, bool spotlight = false)
    {
        ID3D11ShaderResourceView* empty[8] = {}; context->PSSetShaderResources(0, 8, empty);
        ID3D11RenderTargetView* rt = target.target.Get(); context->OMSetRenderTargets(1, &rt, nullptr);
        context->PSSetShaderResources(0, count, sources);
        ID3D11SamplerState* samplers[8]; for (auto& s : samplers) s = sampler.Get(); context->PSSetSamplers(0, 8, samplers);
        context->UpdateSubresource(constants.Get(), 0, nullptr, values, 0, 0);
        ID3D11Buffer* cb = constants.Get(); context->PSSetConstantBuffers(0, 1, &cb); cb = alpha.Get(); context->PSSetConstantBuffers(13, 1, &cb);
        context->VSSetShader(spotlight ? lightVertex.Get() : vertex.Get(), nullptr, 0); context->PSSetShader(shader, nullptr, 0); context->GSSetShader(nullptr, nullptr, 0);
        context->IASetInputLayout(nullptr); context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        D3D11_VIEWPORT vp = {0,0,8,8,0,1}; context->RSSetViewports(1, &vp);
        context->OMSetDepthStencilState(nullptr, 0); context->Draw(3, 0);
    }
    bool Expect(HDRRangeTarget& target, float r, float g, float b, const char* name)
    {
        D3D11_TEXTURE2D_DESC td = {}; target.texture->GetDesc(&td); const auto format = td.Format;
        td.BindFlags = 0; td.Usage = D3D11_USAGE_STAGING; td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        ComPtr<ID3D11Texture2D> read;
        if (FAILED(device->CreateTexture2D(&td, nullptr, &read))) return false;
        context->CopyResource(read.Get(), target.texture.Get()); D3D11_MAPPED_SUBRESOURCE map = {};
        if (FAILED(context->Map(read.Get(), 0, D3D11_MAP_READ, 0, &map))) return false;
        float got[3];
        for (unsigned c = 0; c < 3; ++c) got[c] = format == DXGI_FORMAT_R16G16B16A16_FLOAT ?
            DirectX::PackedVector::XMConvertHalfToFloat(((unsigned short*)map.pData)[c]) : ((unsigned char*)map.pData)[2-c] / 255.0f;
        context->Unmap(read.Get(), 0); const float want[] = {r,g,b};
        for (unsigned c = 0; c < 3; ++c) if (!std::isfinite(got[c]) || fabsf(got[c]-want[c]) > 0.005f * (std::max)(1.0f, fabsf(want[c]))) {
            printf("HDR range %s: got (%g,%g,%g), expected (%g,%g,%g)\n", name, got[0],got[1],got[2],r,g,b); return false;
        }
        printf("HDR range %s: (%g,%g,%g) pass\n", name,got[0],got[1],got[2]); return true;
    }
};

static bool CheckVehicleSHRange(ID3D11Device* device, IDXGISwapChain* chain, HWND window, const char* path)
{
    std::ifstream file(path, std::ios::binary); std::vector<char> bytes((std::istreambuf_iterator<char>(file)), {});
    Display::D3D11::C_D3D11Effect effect;
    if (bytes.empty() || FAILED(effect.LoadMemory(device, bytes.data(), bytes.size()))) return false;
    const void* code = nullptr; SIZE_T size = 0;
    if (FAILED(effect.GetPassVertexShaderBytecode("Material", 0, &code, &size))) return false;
    ComPtr<ID3D11ShaderReflection> reflection; D3D11_SHADER_INPUT_BIND_DESC hdrBinding = {};
    if (FAILED(D3DReflect(code, size, IID_PPV_ARGS(&reflection))) ||
        FAILED(reflection->GetResourceBindingDescByName("BlurHDROutput", &hdrBinding)) || hdrBinding.BindPoint != 12) {
        puts("HDR recovered vehicle VS lost b12 range control"); return false;
    }
    ComPtr<ID3D11GeometryShader> stream;
    D3D11_SO_DECLARATION_ENTRY entry = {0,"TEXCOORD",4,0,4,0}; const UINT stride = 16;
    if (FAILED(device->CreateGeometryShaderWithStreamOutput(code,size,&entry,1,&stride,1,D3D11_SO_NO_RASTERIZED_STREAM,nullptr,&stream))) return false;
    D3D11_INPUT_ELEMENT_DESC elements[8] = {};
    const char* semantics[] = {"POSITION","NORMAL","TEXCOORD","TEXCOORD","TEXCOORD","TEXCOORD","TEXCOORD","COLOR"};
    const UINT indices[] = {0,0,0,1,2,3,4,0};
    for (UINT i=0;i<8;++i) { elements[i].SemanticName=semantics[i]; elements[i].SemanticIndex=indices[i];
        elements[i].Format=i==6?DXGI_FORMAT_R32G32B32A32_UINT:DXGI_FORMAT_R32G32B32A32_FLOAT; elements[i].AlignedByteOffset=i*16; }
    ComPtr<ID3D11InputLayout> layout;
    if (FAILED(device->CreateInputLayout(elements,8,code,size,&layout))) return false;
    float vertex[8][4] = {}; vertex[0][3]=1; vertex[1][0]=vertex[1][1]=.5f; vertex[1][2]=1;
    vertex[4][0]=vertex[4][1]=vertex[4][2]=.5f; vertex[5][0]=vertex[5][1]=vertex[5][2]=.5f;
    vertex[7][0]=1; vertex[7][1]=vertex[7][2]=vertex[7][3]=.5f;
    D3D11_BUFFER_DESC bd = {}; bd.ByteWidth=sizeof(vertex); bd.BindFlags=D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA data = {}; data.pSysMem=vertex; ComPtr<ID3D11Buffer> vb, output, read;
    if (FAILED(device->CreateBuffer(&bd,&data,&vb))) return false;
    bd.ByteWidth=16; bd.BindFlags=D3D11_BIND_STREAM_OUTPUT;
    if (FAILED(device->CreateBuffer(&bd,nullptr,&output))) return false;
    bd.BindFlags=0; bd.Usage=D3D11_USAGE_STAGING; bd.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    if (FAILED(device->CreateBuffer(&bd,nullptr,&read))) return false;
    const float identity[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    float parameters[8][4]={}; parameters[6][0]=parameters[6][1]=parameters[6][2]=1;
    const float sh[3][4]={{4,0,0,0},{2,0,0,0},{.5f,0,0,0}};
    if (FAILED(effect.SetRawParameter("g_mWorld",identity,sizeof(identity))) || FAILED(effect.SetRawParameter("g_mViewProj",identity,sizeof(identity))) ||
        FAILED(effect.SetRawParameter("g_VehicleVtxParams",parameters,sizeof(parameters))) || FAILED(effect.SetRawParameter("g_LocalSpaceSHNode",sh,sizeof(sh)))) return false;
    HDRRangeFixture fixture(device); if (!fixture.Init()) return false;
    ComPtr<ID3D11ShaderResourceView> damage;
    if (!fixture.Pixel(1,0,0,1,damage) || FAILED(effect.SetTexture("g_sVehicleDamageInfoTexture",damage.Get()))) return false;
    C_HDROutput hdr(window); ComPtr<ID3D11Texture2D> frame;
    if (FAILED(hdr.CreateTargets(device,chain,&frame))) return false;
    for (bool active : {false,true}) {
        fixture.context->ClearState(); D3DX11InvalidateBoundState(3,15);
        if (FAILED(effect.Apply("Material",0,fixture.context.Get()))) return false;
        hdr.Calibrate(active,1000,200); hdr.Refresh(device,true); hdr.Bind(fixture.context.Get());
        fixture.context->IASetInputLayout(layout.Get()); fixture.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        ID3D11Buffer* source=vb.Get(); const UINT vbStride=sizeof(vertex), offset=0;
        fixture.context->IASetVertexBuffers(0,1,&source,&vbStride,&offset);
        ID3D11Buffer* sink=output.Get(); fixture.context->SOSetTargets(1,&sink,&offset);
        fixture.context->GSSetShader(stream.Get(),nullptr,0); fixture.context->PSSetShader(nullptr,nullptr,0); fixture.context->Draw(1,0);
        sink=nullptr; fixture.context->SOSetTargets(1,&sink,&offset); fixture.context->CopyResource(read.Get(),output.Get());
        D3D11_MAPPED_SUBRESOURCE map={}; if (FAILED(fixture.context->Map(read.Get(),0,D3D11_MAP_READ,0,&map))) return false;
        float result[4]; memcpy(result,map.pData,sizeof(result)); fixture.context->Unmap(read.Get(),0);
        const float want[]={active?4.0f:1.0f,active?2.0f:1.0f,.5f};
        for (UINT c=0;c<3;++c) if (!std::isfinite(result[c]) || fabsf(result[c]-want[c])>.001f) {
            printf("HDR recovered vehicle SH %s failed: %g,%g,%g\n",active?"HDR":"SDR",result[0],result[1],result[2]); return false;
        }
        printf("HDR recovered vehicle SH %s: %g,%g,%g pass\n",active?"HDR":"SDR",result[0],result[1],result[2]);
    }
    fixture.context->ClearState(); return true;
}

static bool CheckIntermediateRange(ID3D11Device* device, IDXGISwapChain* chain, HWND window, const char* builtins, const char* vehicle)
{
    ComPtr<ID3D11InfoQueue> queue; device->QueryInterface(IID_PPV_ARGS(&queue)); if (queue) queue->ClearStoredMessages();
    HDRRangeFixture f(device); f.context->ClearState(); if (!f.Init()) return false;
    HDRRangeTarget a,b,history,unorm;
    if (!f.Target(a)||!f.Target(b)||!f.Target(history)||!f.Target(unorm,DXGI_FORMAT_B8G8R8A8_UNORM)) return false;
    ComPtr<ID3D11PixelShader> light,blur7,blur5,bloom;
    const std::string root(builtins);
    if (!f.Load(root+"/deferred/psSpotlightPrepass.cso",light)||!f.Load(root+"/render/psGaussBlur7.cso",blur7)||
        !f.Load(root+"/render/psGaussBlur5.cso",blur5)||!f.Load(root+"/render/psBloomCombineFeedbackGamma.cso",bloom)) return false;
    ComPtr<ID3D11ShaderResourceView> depth,normal,bright,black;
    if (!f.Pixel(1,0,0,1,depth)||!f.Pixel(.5f,.5f,0,1,normal)||!f.Pixel(16,8,4,1,bright)||!f.Pixel(0,0,0,1,black)) return false;
    ID3D11ShaderResourceView* sources[5]={depth.Get(),normal.Get()};
    f.values[5][1]=1; f.values[8][2]=1; f.values[9][0]=4; f.values[9][1]=2; f.values[9][2]=1;
    D3D11_BLEND_DESC bd={}; bd.RenderTarget[0].BlendEnable=TRUE;
    bd.RenderTarget[0].SrcBlend=bd.RenderTarget[0].DestBlend=bd.RenderTarget[0].SrcBlendAlpha=bd.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ONE;
    bd.RenderTarget[0].BlendOp=bd.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD; bd.RenderTarget[0].RenderTargetWriteMask=15;
    ComPtr<ID3D11BlendState> additive; if (FAILED(device->CreateBlendState(&bd,&additive))) return false;
    const float zero[4]={};
    for (auto target : {&a,&unorm}) {
        f.context->ClearRenderTargetView(target->target.Get(),zero); f.context->OMSetBlendState(additive.Get(),nullptr,~0u);
        f.Draw(light.Get(),*target,sources,2,true); f.Draw(light.Get(),*target,sources,2,true);
        if (!f.Expect(*target,target==&a?8.f:1.f,target==&a?4.f:1.f,target==&a?2.f:1.f,target==&a?"additive spotlight FP16":"old UNORM clipping control")) return false;
    }
    f.context->OMSetBlendState(nullptr,nullptr,~0u); memset(f.values,0,sizeof(f.values));
    f.values[1][0]=.25f; f.values[1][1]=f.values[1][2]=f.values[1][3]=.125f;
    sources[0]=bright.Get(); f.Draw(blur7.Get(),a,sources,1);
    f.values[1][1]=f.values[1][2]=.1875f; f.values[1][3]=0;
    sources[0]=a.view.Get(); f.Draw(blur5.Get(),b,sources,1);
    if (!f.Expect(b,16,8,4,"two-pass planar reflection blur")) return false;
    memset(f.values,0,sizeof(f.values)); f.values[2][0]=1; f.values[3][0]=1; f.values[3][1]=.75f; f.values[3][2]=.25f;
    sources[0]=bright.Get(); for (UINT i=1;i<5;++i) sources[i]=black.Get(); f.Draw(bloom.Get(),a,sources,5);
    if (!f.Expect(a,16,8,4,"bloom current")) return false;
    memset(f.values,0,sizeof(f.values)); f.values[1][0]=1; sources[0]=a.view.Get(); f.Draw(blur5.Get(),history,sources,1);
    memset(f.values,0,sizeof(f.values)); f.values[3][1]=.75f; f.values[3][2]=.25f;
    for (UINT i=0;i<4;++i) sources[i]=black.Get(); sources[4]=history.view.Get(); f.Draw(bloom.Get(),b,sources,5);
    if (!f.Expect(b,11.75f,5.75f,2.75f,"bloom feedback next frame")) return false;

    // Exercise the production CPU encoder through a real six-face FP16 cube sample.
    const float radiance[]={0,.5f,4,8,16,64}; std::vector<unsigned short> pixels[6]; D3D11_SUBRESOURCE_DATA initial[6]={};
    for (UINT face=0;face<6;++face) {
        pixels[face].resize(8*40,0xffff); float encoded[4];
        Game::EncodeExtendedVehicleLightRGBM(radiance[face],radiance[face]*.5f,radiance[face]*.25f,encoded);
        for (UINT y=0;y<8;++y) for (UINT x=0;x<8;++x) for (UINT c=0;c<4;++c)
            pixels[face][y*40+x*4+c]=DirectX::PackedVector::XMConvertFloatToHalf(encoded[c]);
        initial[face].pSysMem=pixels[face].data(); initial[face].SysMemPitch=80;
    }
    D3D11_TEXTURE2D_DESC td={}; td.Width=td.Height=8; td.MipLevels=td.SampleDesc.Count=1; td.ArraySize=6;
    td.Format=DXGI_FORMAT_R16G16B16A16_FLOAT; td.BindFlags=D3D11_BIND_SHADER_RESOURCE; td.MiscFlags=D3D11_RESOURCE_MISC_TEXTURECUBE;
    ComPtr<ID3D11Texture2D> cube; ComPtr<ID3D11ShaderResourceView> cubeView; ComPtr<ID3D11PixelShader> decode;
    if (FAILED(device->CreateTexture2D(&td,initial,&cube))||FAILED(device->CreateShaderResourceView(cube.Get(),nullptr,&cubeView))||
        !f.Compile("TextureCube<float4> t:register(t0);SamplerState s:register(s0);cbuffer P:register(b0){float4 dir;}float4 main():SV_Target{float4 v=t.Sample(s,dir.xyz);return float4(v.rgb*v.a*8,1);}",decode)) return false;
    const float directions[6][4]={{1,0,0,0},{-1,0,0,0},{0,1,0,0},{0,-1,0,0},{0,0,1,0},{0,0,-1,0}};
    sources[0]=cubeView.Get();
    for (UINT face=0;face<6;++face) {
        memcpy(f.values[0],directions[face],16); f.Draw(decode.Get(),a,sources,1);
        if (!f.Expect(a,radiance[face],radiance[face]*.5f,radiance[face]*.25f,"vehicle floating RGBM cube")) return false;
    }

    // Compile the production common material helpers; cover glass and lower bounds.
    std::ifstream header(root+"/../../../../code/mercury/library/graphics/shader/include/shader/common/HDRRange.h");
    const std::string shared((std::istreambuf_iterator<char>(header)),{}); ComPtr<ID3D11PixelShader> glass,sh;
    if (shared.empty()||!f.Compile("#define BLUR_DX11 1\n"+shared+"\nfloat4 main():SV_Target{return float4(BlurHDRReflection(float3(16,4,.25),1),1);}",glass)||
        !f.Compile("#define BLUR_DX11 1\n"+shared+"\nfloat4 main():SV_Target{return BlurHDRLighting(float4(4,2,.5,-1));}",sh)) return false;
    C_HDROutput hdr(window); ComPtr<ID3D11Texture2D> frame;
    if (FAILED(hdr.CreateTargets(device,chain,&frame))) return false;
    sources[0]=black.Get();
    for (bool active : {false,true}) {
        hdr.Calibrate(active,1000,200); hdr.Refresh(device,true); hdr.Bind(f.context.Get());
        f.Draw(glass.Get(),a,sources,1); if (!f.Expect(a,active?16.f:1.f,active?4.f:1.f,.25f,active?"glass HDR":"glass SDR")) return false;
        f.Draw(sh.Get(),a,sources,1); if (!f.Expect(a,active?4.f:1.f,active?2.f:1.f,.5f,active?"common SH HDR":"common SH SDR")) return false;
    }
    f.context->ClearState();
    return CheckVehicleSHRange(device,chain,window,vehicle) && CheckFixtureErrors(queue.Get());
}
