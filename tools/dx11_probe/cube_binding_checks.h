// [Blur1PCDX11] Match PC cube-sampler/2D-resource reads while retaining valid cubes.
#pragma once
#include <d3d11sdklayers.h>
#include <cstring>

static bool CheckCubeTextureBinding(Display::D3D11::C_D3D11Device& device)
{
    using Microsoft::WRL::ComPtr;
    const char* source = R"(
TextureCube image;
float4 direction;
SamplerState pointSampler { Filter = MIN_MAG_MIP_POINT; AddressU = Clamp; AddressV = Clamp; AddressW = Clamp; };
float4 VS(uint id : SV_VertexID) : SV_Position {
    return float4(id == 2 ? 3 : -1, id == 1 ? 3 : -1, 0, 1);
}
float4 PS() : SV_Target { return image.SampleLevel(pointSampler, direction.xyz, 0); }
technique11 Material { pass p0 {
    SetVertexShader(CompileShader(vs_5_0, VS()));
    SetPixelShader(CompileShader(ps_5_0, PS()));
} }
)";
    ComPtr<ID3DBlob> code, errors;
    HRESULT hr = D3DCompile(source, std::strlen(source), nullptr, nullptr, nullptr,
        nullptr, "fx_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &code, &errors);
    if (FAILED(hr)) return false;
    Display::D3D11::C_D3D11Effect effect;
    if (FAILED(effect.LoadMemory(device.Device(), code->GetBufferPointer(), code->GetBufferSize()))) return false;
    ComPtr<ID3D11Texture2D> target, staging;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<ID3D11ShaderResourceView> targetView;
    if (FAILED(device.CreateRenderTexture2D(4,4,1,1,DXGI_FORMAT_R8G8B8A8_UNORM,target,rtv,targetView))) return false;
    D3D11_TEXTURE2D_DESC readDesc = {};
    target->GetDesc(&readDesc);
    readDesc.Usage = D3D11_USAGE_STAGING; readDesc.BindFlags = 0;
    readDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(device.Device()->CreateTexture2D(&readDesc,nullptr,&staging))) return false;
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID; rasterDesc.CullMode = D3D11_CULL_NONE;
    ComPtr<ID3D11RasterizerState> raster;
    if (FAILED(device.Device()->CreateRasterizerState(&rasterDesc,&raster))) return false;
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    ComPtr<ID3D11DepthStencilState> depth;
    if (FAILED(device.Device()->CreateDepthStencilState(&depthDesc,&depth))) return false;
    auto* context = device.Context();
    context->ClearState(); D3DX11InvalidateBoundState(3,15);
    ID3D11RenderTargetView* rawTarget = rtv.Get();
    context->OMSetRenderTargets(1,&rawTarget,nullptr);
    context->OMSetDepthStencilState(depth.Get(),0);
    context->RSSetState(raster.Get());
    const D3D11_VIEWPORT viewport = {0,0,4,4,0,1};
    context->RSSetViewports(1,&viewport);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ComPtr<ID3D11InfoQueue> queue;
    device.Device()->QueryInterface(IID_PPV_ARGS(&queue));
    if (queue) queue->ClearStoredMessages();
    const unsigned char colours[][4] = {{0,0,0,0},{255,255,255,255},{102,51,34,153}};
    unsigned checked = 0;
    for (const auto& colour : colours)
    {
        const void* data[] = {colour}; const UINT bytes[] = {4};
        ComPtr<ID3D11Texture2D> image2D, cube;
        ComPtr<ID3D11ShaderResourceView> view2D, cubeView;
        if (FAILED(device.CreateTexture2DFromPCData(1,1,1,D3DFMT_A8B8G8R8,data,bytes,image2D,view2D))) return false;
        const void* faces[] = {colour,colour,colour,colour,colour,colour};
        const UINT faceBytes[] = {4,4,4,4,4,4};
        if (FAILED(device.CreateTextureCubeFromPCData(1,1,D3DFMT_A8B8G8R8,faces,faceBytes,cube,cubeView))) return false;
        // Test both transitions so a valid cube cannot retain the zero binding.
        for (unsigned kind = 0; kind < 3; ++kind)
            for (unsigned axis = 0; axis < 6; ++axis)
            {
                float direction[4] = {}; direction[axis/2] = axis%2 ? -1.f : 1.f;
                if (FAILED(effect.SetRawParameter("direction",direction,sizeof(direction))) ||
                    FAILED(effect.SetTexture("image",kind == 1 ? cubeView.Get() : view2D.Get())) ||
                    FAILED(effect.Apply("Material",0,context))) return false;
                const float clear[] = {1,0,1,1};
                context->ClearRenderTargetView(rtv.Get(),clear);
                context->Draw(3,0);
                context->CopyResource(staging.Get(),target.Get());
                D3D11_MAPPED_SUBRESOURCE mapped = {};
                if (FAILED(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped))) return false;
                const auto* actual = static_cast<const unsigned char*>(mapped.pData);
                bool valid = true;
                for (unsigned channel = 0; channel < 4; ++channel)
                    valid &= actual[channel] == (kind == 1 ? colour[channel] : 0);
                context->Unmap(staging.Get(),0);
                if (!valid) { std::fprintf(stderr,"Cube binding failed colour=%u kind=%u axis=%u\n",checked/18,kind,axis); return false; }
                ++checked;
            }
    }
    if (queue)
        for (UINT64 i=0;i<queue->GetNumStoredMessages();++i)
        {
            SIZE_T size=0;queue->GetMessage(i,nullptr,&size);
            std::vector<unsigned char> bytes(size);
            auto* message=reinterpret_cast<D3D11_MESSAGE*>(bytes.data());
            if (SUCCEEDED(queue->GetMessage(i,message,&size)) && message->Severity <= D3D11_MESSAGE_SEVERITY_ERROR)
            { std::fprintf(stderr,"Cube binding GPU error: %s\n",message->pDescription);return false; }
        }
    context->ClearState(); D3DX11InvalidateBoundState(3,15);
    std::printf("Native cube binding: %u GPU checks passed (2D/cube/2D, all faces, RGBA)\n",checked);
    return checked == 54;
}
