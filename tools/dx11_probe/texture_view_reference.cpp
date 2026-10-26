// [Blur1PCDX11] Retained views must match fresh views across resource/gamma changes.
#include "Display/Device/D3D11/D3D11Device.h"
#include "Display/Device/D3D11/D3D11Effect.h"
#include "Display/Device/D3D11/D3D11Formats.h"
#include <d3d11sdklayers.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// [Blur1PCDX11] Raw context resets must also forget FX binding memoization.
extern void D3DX11InvalidateBoundState(unsigned stages, unsigned what);
extern void D3DX11InvalidateBoundConstantBuffers(unsigned stages, unsigned start, unsigned count);
using Microsoft::WRL::ComPtr;

// [Blur1PCDX11] A resource-owned marker proves that the registry does not
// extend texture lifetime after its engine owner releases the retained views.
static volatile LONG g_destroyedTextures = 0;
static const GUID g_lifetimeMarker = {0x47d7219b,0x2132,0x4a32,{0x97,0xd3,0x56,0x38,0x51,0x0f,0xf4,0x73}};
class C_LifetimeMarker : public IUnknown
{
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override
    {
        if (!object) return E_POINTER;
        *object = nullptr;
        if (iid != __uuidof(IUnknown)) return E_NOINTERFACE;
        *object = static_cast<IUnknown*>(this); AddRef(); return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_refs); }
    ULONG STDMETHODCALLTYPE Release() override
    {
        const ULONG refs = InterlockedDecrement(&m_refs);
        if (!refs) { InterlockedIncrement(&g_destroyedTextures); delete this; }
        return refs;
    }
private:
    volatile LONG m_refs = 1;
};

struct C_ViewOwner
{
    explicit C_ViewOwner(ID3D11Resource* texture) : resource(texture)
    { Display::D3D11::RegisterTextureViewOwner(resource); }
    ~C_ViewOwner() { Display::D3D11::ReleaseTextureViewOwner(resource); }
    ID3D11Resource* resource;
};

static bool CheckViews(Display::D3D11::C_D3D11Device& device, unsigned kind)
{
    const char* types[] = {"Texture2D", "Texture3D", "TextureCube", "Texture2DArray", "TextureCubeArray", "Texture2D"};
    const char* coords[] = {"float2(.5,.5)", "float3(.5,.5,.5)", "float3(1,0,0)", "float3(.5,.5,0)", "float4(1,0,0,0)", "float2(.5,.5)"};
    const char* nativeName=kind==1?"g_sVehicleDamageVolumeTexture":"sampleTexture";
    const char* bindingName=kind==1?"g_sVehicleDamageVolumeTexture3d":"sampleTexture";
    std::string source = std::string(types[kind]) + " " + nativeName + ";\n" + R"(
float4 tint;
float4 secondTint;
bool uniformOnly = false;
SamplerState pointSampler { Filter=MIN_MAG_MIP_POINT; AddressU=Clamp; AddressV=Clamp; AddressW=Clamp; };
float4 VS(uint id:SV_VertexID):SV_Position { return float4(id==2?3:-1,id==1?3:-1,0,1); }
float4 PS():SV_Target { if (uniformOnly) return tint + secondTint; return )" + nativeName + R"(.Sample(pointSampler,)" + coords[kind] + R"(); }
technique11 Material { pass p0 {
 SetVertexShader(CompileShader(vs_5_0,VS())); SetPixelShader(CompileShader(ps_5_0,PS()));
} }
)";
    ComPtr<ID3DBlob> bytecode, errors;
    if (FAILED(D3DCompile(source.data(),source.size(),nullptr,nullptr,nullptr,nullptr,"fx_5_0",0,0,&bytecode,&errors)))
    { if (errors) std::fprintf(stderr,"%s",static_cast<const char*>(errors->GetBufferPointer())); return false; }
    Display::D3D11::C_D3D11Effect effect;
    if (FAILED(effect.LoadMemory(device.Device(),bytecode->GetBufferPointer(),bytecode->GetBufferSize()))) return false;
    if (!effect.HasParameter("tint") || effect.HasParameter("addedTint") || effect.HasParameter("missing")) return false;
    if (!effect.HasParameter(nativeName) || !effect.HasParameter(bindingName)) return false;
    std::string replacement=source;
    for (size_t pos=0;(pos=replacement.find("tint",pos))!=std::string::npos;pos+=9)
        replacement.replace(pos,4,"addedTint");
    ComPtr<ID3DBlob> replacementCode;
    if (FAILED(D3DCompile(replacement.data(),replacement.size(),nullptr,nullptr,nullptr,nullptr,"fx_5_0",0,0,&replacementCode,&errors))) return false;
    ComPtr<ID3D11Resource> textures[2];
    const unsigned char pixels[2][4] = {{128,64,240,255},{24,80,176,255}};
    const float scalarPixels[] = {.25f,.75f};
    for (unsigned i=0;i<2;++i)
    {
        D3D11_SUBRESOURCE_DATA data[12] = {};
        for (auto& face : data) { face.pSysMem=kind==5?static_cast<const void*>(&scalarPixels[i]):pixels[i]; face.SysMemPitch=4; face.SysMemSlicePitch=4; }
        if (kind==1)
        {
            D3D11_TEXTURE3D_DESC desc = {};
            desc.Width=desc.Height=desc.Depth=desc.MipLevels=1;
            desc.Format=DXGI_FORMAT_R8G8B8A8_TYPELESS; desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
            ComPtr<ID3D11Texture3D> texture;
            if (FAILED(device.Device()->CreateTexture3D(&desc,data,&texture))) return false;
            textures[i]=texture;
        }
        else
        {
            D3D11_TEXTURE2D_DESC desc = {};
            desc.Width=desc.Height=desc.MipLevels=desc.SampleDesc.Count=1;
            desc.ArraySize=kind==2?6:kind==3?2:kind==4?12:1;
            desc.Format=kind==5?DXGI_FORMAT_R32_FLOAT:DXGI_FORMAT_R8G8B8A8_TYPELESS; desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
            desc.MiscFlags=kind==2||kind==4?D3D11_RESOURCE_MISC_TEXTURECUBE:0;
            ComPtr<ID3D11Texture2D> texture;
            if (FAILED(device.Device()->CreateTexture2D(&desc,data,&texture))) return false;
            textures[i]=texture;
        }
    }
    ComPtr<ID3D11Texture2D> target, staging;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<ID3D11ShaderResourceView> targetView;
    if (FAILED(device.CreateRenderTexture2D(1,1,1,1,DXGI_FORMAT_R32G32B32A32_FLOAT,target,rtv,targetView))) return false;
    D3D11_TEXTURE2D_DESC targetDesc = {};
    target->GetDesc(&targetDesc); targetDesc.Usage=D3D11_USAGE_STAGING;
    targetDesc.BindFlags=0; targetDesc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    if (FAILED(device.Device()->CreateTexture2D(&targetDesc,nullptr,&staging))) return false;
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode=D3D11_FILL_SOLID; rasterDesc.CullMode=D3D11_CULL_NONE;
    ComPtr<ID3D11RasterizerState> raster;
    if (FAILED(device.Device()->CreateRasterizerState(&rasterDesc,&raster))) return false;
    auto* context=device.Context();
    const D3D11_VIEWPORT viewport={0,0,1,1,0,1};
    auto render = [&](ID3D11ShaderResourceView* view, float* pixel) {
        if (FAILED(effect.SetTexture(bindingName,view)) || FAILED(effect.Apply("Material",0,context))) return false;
        context->RSSetState(raster.Get()); context->RSSetViewports(1,&viewport);
        auto* raw=rtv.Get(); context->OMSetRenderTargets(1,&raw,nullptr);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->Draw(3,0); context->CopyResource(staging.Get(),target.Get());
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (FAILED(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped))) return false;
        std::memcpy(pixel,mapped.pData,4*sizeof(float)); context->Unmap(staging.Get(),0);
        return true;
    };
    context->ClearState(); D3DX11InvalidateBoundState(3,15);
    const unsigned resources[]={0,0,0,0,1,1,1,1,0,0,0,0,0,0};
    const bool gamma[]={false,false,true,true,true,true,false,false,false,false,false,false,false,false};
    for (unsigned step=0;step<14;++step)
    {
        if (step==10 && FAILED(effect.SetTexture(bindingName,nullptr))) return false;
        if (step==12)
        {
            if (FAILED(effect.LoadMemory(device.Device(),replacementCode->GetBufferPointer(),replacementCode->GetBufferSize()))) return false;
            if (effect.HasParameter("tint") || !effect.HasParameter("addedTint") || effect.HasParameter("missing")) return false;
        }
        ComPtr<ID3D11ShaderResourceView> view, fresh;
        const HRESULT result=effect.CreateTextureView(bindingName,device.Device(),textures[resources[step]].Get(),gamma[step],&view);
        if (result != (step%2 || (kind==5 && (step==2 || step==6)) ? S_FALSE : S_OK))
        {
            std::fprintf(stderr,"%s step%u unexpected reuse result %08x\n",types[kind],step,static_cast<unsigned>(result));
            return false;
        }
        if (FAILED(Display::D3D11::CreateTextureView(device.Device(),textures[resources[step]].Get(),gamma[step],&fresh))) return false;
        float reused[4], expected[4];
        if (!render(view.Get(),reused) || !render(fresh.Get(),expected) || std::memcmp(reused,expected,sizeof(reused))!=0) return false;
    }
    const int uniformOnly=1;
    if (FAILED(effect.SetRawParameter("uniformOnly",&uniformOnly,sizeof(uniformOnly)))) return false;
    const float secondTint[]={1,2,4,8};
    for (unsigned change=0;change<2;++change)
    {
        float tint[4],expected[4],actual[4];
        for (unsigned channel=0;channel<4;++channel)
        { tint[channel]=secondTint[channel]*(change+1); expected[channel]=tint[channel]+secondTint[channel]; }
        if (FAILED(effect.SetRawParameter("addedTint",tint,sizeof(tint))) ||
            FAILED(effect.SetRawParameter("secondTint",secondTint,sizeof(secondTint)))) return false;
        ComPtr<ID3D11ShaderResourceView> view;
        if (FAILED(effect.CreateTextureView(bindingName,device.Device(),textures[0].Get(),false,&view)) ||
            !render(view.Get(),actual) || std::memcmp(actual,expected,sizeof(actual))!=0) return false;
    }
    // Switching materials/resources, gamma and an effect reload must reuse
    // each owned view without changing pixels or retaining a dead texture.
    const int sampleTexture=0;
    if (FAILED(effect.SetRawParameter("uniformOnly",&sampleTexture,sizeof(sampleTexture)))) return false;
    context->ClearState(); D3DX11InvalidateBoundState(3,15);
    if (FAILED(effect.SetTexture(bindingName,nullptr))) return false;
    const LONG destroyedBefore=g_destroyedTextures;
    for (auto& texture : textures)
    {
        auto* marker=new C_LifetimeMarker;
        const HRESULT result=texture->SetPrivateDataInterface(g_lifetimeMarker,marker);
        marker->Release();
        if (FAILED(result)) return false;
    }
    ComPtr<ID3D11ShaderResourceView> identity[2][2];
    {
        C_ViewOwner owners[]={C_ViewOwner(textures[0].Get()),C_ViewOwner(textures[1].Get())};
        for (unsigned step=0;step<24;++step)
        {
            const unsigned material=step%2;
            const bool srgb=(step/2)%2 != 0;
            const unsigned slot=kind==5?0:srgb;
            if (step==8 && FAILED(effect.SetTexture(bindingName,nullptr))) return false;
            if (step==12)
            {
                const unsigned char changed[2][4]={{40,200,64,255},{192,32,128,255}};
                const float changedScalar[]={.125f,.875f};
                const unsigned subresources=kind==2?6:kind==3?2:kind==4?12:1;
                for (unsigned material=0;material<2;++material)
                    for (unsigned subresource=0;subresource<subresources;++subresource)
                        context->UpdateSubresource(textures[material].Get(),subresource,nullptr,
                            kind==5?static_cast<const void*>(&changedScalar[material]):changed[material],4,4);
            }
            if (step==16)
            {
                if (FAILED(effect.LoadMemory(device.Device(),replacementCode->GetBufferPointer(),replacementCode->GetBufferSize()))) return false;
            }
            ComPtr<ID3D11ShaderResourceView> view,fresh;
            const HRESULT result=effect.CreateTextureView(bindingName,device.Device(),textures[material].Get(),srgb,&view);
            if (result != (identity[material][slot] ? S_FALSE : S_OK))
            { std::fprintf(stderr,"%s owned step%u unexpected result %08x\n",types[kind],step,static_cast<unsigned>(result)); return false; }
            if (identity[material][slot] && identity[material][slot].Get()!=view.Get())
            { std::fprintf(stderr,"%s owned step%u changed view identity\n",types[kind],step); return false; }
            identity[material][slot]=view;
            D3D11_SHADER_RESOURCE_VIEW_DESC desc={};
            view->GetDesc(&desc);
            if (FAILED(device.Device()->CreateShaderResourceView(textures[material].Get(),&desc,&fresh))) return false;
            float actual[4],expected[4];
            if (!render(view.Get(),actual) || !render(fresh.Get(),expected) || std::memcmp(actual,expected,sizeof(actual)))
            { std::fprintf(stderr,"%s owned step%u pixel mismatch\n",types[kind],step); return false; }
        }
        context->ClearState(); D3DX11InvalidateBoundState(3,15);
        if (FAILED(effect.SetTexture(bindingName,nullptr))) return false;
        for (auto& material : identity) for (auto& view : material) view.Reset();
        if (g_destroyedTextures != destroyedBefore) return false;
    }
    // The owner registry is now empty for these resources. Releasing the last
    // external texture references must destroy both, without a COM cycle.
    for (auto& texture : textures) texture.Reset();
    // D3D11 defers object destruction; flush only this lifetime test after
    // dropping all application references, as required by the API contract.
    context->ClearState(); D3DX11InvalidateBoundState(3,15);
    context->Flush();
    if (g_destroyedTextures != destroyedBefore+2)
    { std::fprintf(stderr,"%s: retained texture lifetime leak\n",types[kind]); return false; }
    std::printf("%s: 14 retained/fresh + 24 alternating owned GPU views, 2 uniforms and destruction passed\n",types[kind]);
    return true;
}

// [Blur1PCDX11] Queue all draws before readback. Two independently updated
// effects (including a clone) must retain each draw's complete constants,
// including b13 restoration after device-owned constant-buffer bindings.
static bool CheckQueuedConstants(Display::D3D11::C_D3D11Device& device)
{
    const char* source = R"(
cbuffer MaterialState : register(b0) { float4 changing; float4 preserved; };
cbuffer DeviceState : register(b13) { float4 deviceValue; };
float4 VS(uint id:SV_VertexID):SV_Position { return float4(id==2?3:-1,id==1?3:-1,0,1); }
float4 PS():SV_Target { return changing + preserved + deviceValue; }
technique11 Material { pass p0 {
 SetVertexShader(CompileShader(vs_5_0,VS())); SetPixelShader(CompileShader(ps_5_0,PS()));
} }
)";
    ComPtr<ID3DBlob> code, errors;
    if (FAILED(D3DCompile(source,std::strlen(source),nullptr,nullptr,nullptr,nullptr,"fx_5_0",0,0,&code,&errors))) return false;
    ComPtr<ID3DX11Effect> effects[2];
    if (FAILED(D3DX11CreateEffectFromMemory(code->GetBufferPointer(),code->GetBufferSize(),0,device.Device(),&effects[0])) ||
        FAILED(effects[0]->CloneEffect(0,&effects[1]))) return false;
    const float preserved[2][4] = {{1,2,4,8},{16,32,64,128}};
    const float deviceValues[3][4] = {{2,4,8,16},{32,64,128,256},{512,256,128,64}};
    for (unsigned i=0;i<2;++i)
        if (FAILED(effects[i]->GetVariableByName("preserved")->SetRawValue(preserved[i],0,sizeof(preserved[i]))) ||
            FAILED(effects[i]->GetVariableByName("deviceValue")->SetRawValue(deviceValues[i],0,sizeof(deviceValues[i])))) return false;
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = sizeof(deviceValues[2]); bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    D3D11_SUBRESOURCE_DATA initial = {}; initial.pSysMem = deviceValues[2];
    ComPtr<ID3D11Buffer> deviceBuffer;
    if (FAILED(device.Device()->CreateBuffer(&bufferDesc,&initial,&deviceBuffer))) return false;
    const unsigned count = 128;
    ComPtr<ID3D11Texture2D> target, staging;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<ID3D11ShaderResourceView> srv;
    if (FAILED(device.CreateRenderTexture2D(count,1,1,1,DXGI_FORMAT_R32G32B32A32_FLOAT,target,rtv,srv))) return false;
    D3D11_TEXTURE2D_DESC desc = {};
    target->GetDesc(&desc); desc.Usage=D3D11_USAGE_STAGING; desc.BindFlags=0; desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    if (FAILED(device.Device()->CreateTexture2D(&desc,nullptr,&staging))) return false;
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode=D3D11_FILL_SOLID; rasterDesc.CullMode=D3D11_CULL_NONE;
    ComPtr<ID3D11RasterizerState> raster;
    if (FAILED(device.Device()->CreateRasterizerState(&rasterDesc,&raster))) return false;
    auto* context = device.Context();
    context->ClearState(); D3DX11InvalidateBoundState(3,15);
    ID3D11RenderTargetView* raw = rtv.Get(); context->OMSetRenderTargets(1,&raw,nullptr);
    context->RSSetState(raster.Get()); context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    for (unsigned draw=0;draw<count;++draw)
    {
        auto* effect=effects[(draw/4)%2].Get();
        const float value[4] = {float(draw),float(draw+1),float(draw+2),float(draw+3)};
        if (FAILED(effect->GetVariableByName("changing")->SetRawValue(value,0,sizeof(value))) ||
            FAILED(effect->GetTechniqueByName("Material")->GetPassByIndex(0)->Apply(0,context))) return false;
        if (draw%3 == 0)
        {
            ID3D11Buffer* buffer = deviceBuffer.Get();
            context->PSSetConstantBuffers(13,1,&buffer);
            D3DX11InvalidateBoundConstantBuffers(2,13,1);
        }
        const D3D11_VIEWPORT viewport={float(draw),0,1,1,0,1}; context->RSSetViewports(1,&viewport);
        context->Draw(3,0);
    }
    context->CopyResource(staging.Get(),target.Get());
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (FAILED(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped))) return false;
    bool passed=true;
    const float* pixels=static_cast<const float*>(mapped.pData);
    for (unsigned draw=0;draw<count;++draw)
        for (unsigned channel=0;channel<4;++channel)
            if (pixels[draw*4+channel] != float(draw+channel)+preserved[(draw/4)%2][channel]+
                deviceValues[draw%3 == 0 ? 2 : (draw/4)%2][channel]) passed=false;
    context->Unmap(staging.Get(),0); context->ClearState(); D3DX11InvalidateBoundState(3,15);
    std::printf("Queued constants: %u grouped original/clone draws, partial updates and external b13 interop: %s\n",count,passed?"passed":"FAILED");
    return passed;
}

int main()
{
    Display::D3D11::C_D3D11Device device;
    if (FAILED(device.Initialise())) return 1;
    ComPtr<ID3D11InfoQueue> queue;
    device.Device()->QueryInterface(IID_PPV_ARGS(&queue));
    if (queue) queue->ClearStoredMessages();
    for (unsigned kind=0;kind<6;++kind) if (!CheckViews(device,kind))
    { std::fprintf(stderr,"Texture view kind%u failed\n",kind); return 1; }
    if (!CheckQueuedConstants(device)) return 1;
    if (queue) for (UINT64 i=0;i<queue->GetNumStoredMessages();++i)
    {
        SIZE_T size=0; queue->GetMessage(i,nullptr,&size);
        std::vector<unsigned char> bytes(size);
        auto* message=reinterpret_cast<D3D11_MESSAGE*>(bytes.data());
        if (SUCCEEDED(queue->GetMessage(i,message,&size)) && message->Severity<=D3D11_MESSAGE_SEVERITY_ERROR)
        { std::fprintf(stderr,"Texture view GPU error: %s\n",message->pDescription); return 1; }
    }
    return 0;
}
