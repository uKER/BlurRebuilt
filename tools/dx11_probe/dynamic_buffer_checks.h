// [Blur1PCDX11] Queued draws must survive append locks and later discards.
#pragma once
#include <cstdio>
#include <cstring>
#include "Display/Device/D3D11/D3D11InputLayout.h"
// [Blur1PCDX11] This fixture also builds in the standalone buffer reference.
extern void D3DX11InvalidateBoundState(unsigned stages, unsigned what);

static bool CheckDynamicBufferLocks(Display::D3D11::C_D3D11Device& device)
{
    using Microsoft::WRL::ComPtr;
    const char* source = R"(
struct Input { float2 position : POSITION; float4 colour : COLOR0; };
struct Output { float4 position : SV_Position; float4 colour : COLOR0; };
Output VS(Input v) { Output o; o.position = float4(v.position,0,1); o.colour = v.colour; return o; }
float4 PS(Output v) : SV_Target { return v.colour; }
technique11 Material { pass p0 {
 SetVertexShader(CompileShader(vs_5_0, VS()));
 SetPixelShader(CompileShader(ps_5_0, PS()));
} }
)";
    ComPtr<ID3DBlob> code, errors;
    if (FAILED(D3DCompile(source,std::strlen(source),nullptr,nullptr,nullptr,nullptr,"fx_5_0",0,0,&code,&errors))) return false;
    Display::D3D11::C_D3D11Effect effect;
    if (FAILED(effect.LoadMemory(device.Device(),code->GetBufferPointer(),code->GetBufferSize()))) return false;
    const unsigned rounds = 16, draws = rounds * 4;
    ComPtr<ID3D11Texture2D> target, staging;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<ID3D11ShaderResourceView> srv;
    if (FAILED(device.CreateRenderTexture2D(4,4*draws,1,1,DXGI_FORMAT_R8G8B8A8_UNORM,target,rtv,srv))) return false;
    D3D11_TEXTURE2D_DESC textureDesc = {};
    target->GetDesc(&textureDesc);
    textureDesc.Usage = D3D11_USAGE_STAGING; textureDesc.BindFlags = 0;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(device.Device()->CreateTexture2D(&textureDesc,nullptr,&staging))) return false;
    struct Vertex { float x,y,colour[4]; };
    Display2::C_VertexBuffer vb;
    Display2::C_IndexBuffer ib;
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = 6*sizeof(Vertex); desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    ComPtr<ID3D11Buffer> vertices, indices;
    if (FAILED(device.Device()->CreateBuffer(&desc,nullptr,&vertices))) return false;
    desc.ByteWidth = 6*sizeof(unsigned short); desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    if (FAILED(device.Device()->CreateBuffer(&desc,nullptr,&indices))) return false;
    vb.m_pd3dVertexBuffer = vertices.Get(); ib.m_pd3dIndexBuffer = indices.Get();
    const D3DVERTEXELEMENT9 elements[] = {
        {0,0,D3DDECLTYPE_FLOAT2,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_POSITION,0},
        {0,8,D3DDECLTYPE_FLOAT4,D3DDECLMETHOD_DEFAULT,D3DDECLUSAGE_COLOR,0}
    };
    auto* context = device.Context();
    context->ClearState(); D3DX11InvalidateBoundState(3,15);
    const void* bytecode = nullptr; SIZE_T bytecodeSize = 0;
    ComPtr<ID3D11InputLayout> layout;
    if (FAILED(effect.GetPassVertexShaderBytecode("Material",0,&bytecode,&bytecodeSize)) ||
        FAILED(Display::D3D11::CreateGameInputLayout(device.Device(),elements,2,bytecode,bytecodeSize,layout))) return false;
    if (FAILED(effect.Apply("Material",0,context))) return false;
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID; rasterDesc.CullMode = D3D11_CULL_NONE;
    ComPtr<ID3D11RasterizerState> raster;
    if (FAILED(device.Device()->CreateRasterizerState(&rasterDesc,&raster))) return false;
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    ComPtr<ID3D11DepthStencilState> depth;
    if (FAILED(device.Device()->CreateDepthStencilState(&depthDesc,&depth))) return false;
    context->RSSetState(raster.Get()); context->OMSetDepthStencilState(depth.Get(),0);
    context->IASetInputLayout(layout.Get());
    const UINT stride = sizeof(Vertex), base = 0;
    ID3D11Buffer* rawVertices = vertices.Get();
    context->IASetVertexBuffers(0,1,&rawVertices,&stride,&base);
    context->IASetIndexBuffer(indices.Get(),DXGI_FORMAT_R16_UINT,0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ID3D11RenderTargetView* rawTarget = rtv.Get();
    context->OMSetRenderTargets(1,&rawTarget,nullptr);
    const float clear[] = {1,0,1,1}; context->ClearRenderTargetView(rtv.Get(),clear);
    // Submit all writes/draws before readback, including discards while prior
    // draws can still use the old allocations. NOOVERWRITE must retain slot0.
    for (unsigned draw=0;draw<draws;++draw)
    {
        const unsigned kind = draw%4, offset = kind==1 ? 3 : 0;
        const unsigned colour = kind==3 ? 2 : kind==2 ? 0 : kind;
        const U32 flags = kind==1 ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD;
        Vertex triangle[] = {{-1,-1,{0,0,0,1}},{-1,3,{0,0,0,1}},{3,-1,{0,0,0,1}}};
        if (kind != 2)
        {
            for (auto& v : triangle) v.colour[colour] = 1;
            void* data = vb.Lock(offset*sizeof(Vertex),sizeof(triangle),flags);
            if (!data) return false;
            std::memcpy(data,triangle,sizeof(triangle)); vb.Unlock();
            unsigned short order[] = {static_cast<unsigned short>(offset),static_cast<unsigned short>(offset+1),static_cast<unsigned short>(offset+2)};
            data = ib.Lock(offset*sizeof(unsigned short),sizeof(order),flags);
            if (!data) return false;
            std::memcpy(data,order,sizeof(order)); ib.Unlock();
        }
        D3D11_VIEWPORT viewport = {0,static_cast<float>(4*draw),4,4,0,1};
        context->RSSetViewports(1,&viewport);
        context->DrawIndexed(3,offset,0);
    }
    context->CopyResource(staging.Get(),target.Get());
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (FAILED(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped))) return false;
    bool valid = true;
    for (unsigned draw=0;draw<draws;++draw)
    {
        const auto* pixel = static_cast<const unsigned char*>(mapped.pData)+(4*draw+2)*mapped.RowPitch+2*4;
        const unsigned kind = draw%4;
        const unsigned colour = kind==3 ? 2 : kind==2 ? 0 : kind;
        for (unsigned channel=0;channel<4;++channel)
            valid &= pixel[channel] == (channel==3 || channel==colour ? 255 : 0);
    }
    context->Unmap(staging.Get(),0);
    context->ClearState(); D3DX11InvalidateBoundState(3,15);
    std::printf("Dynamic VB/IB locks: %u queued discard/append/retained/discard draws %s\n",draws,valid?"passed":"FAILED");
    return valid;
}
