// [Blur1PCDX11] Verify the measured PC zero-depth billboard prepass result.
#include <cstring>
static bool CheckBillboardPrepass(Display::D3D11::C_D3D11Device& device,
                             Display::D3D11::C_D3D11EffectLibrary& library)
{
    using Microsoft::WRL::ComPtr;
    if (FAILED(device.CreateOffscreenTarget(64,64))) return false;
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = desc.Height = 64; desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R32_FLOAT; desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    ComPtr<ID3D11Texture2D> depth, staging;
    ComPtr<ID3D11RenderTargetView> depthView;
    if (FAILED(device.Device()->CreateTexture2D(&desc,nullptr,&depth)) ||
        FAILED(device.Device()->CreateRenderTargetView(depth.Get(),nullptr,&depthView))) return false;
    desc.BindFlags = 0; desc.Usage = D3D11_USAGE_STAGING; desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(device.Device()->CreateTexture2D(&desc,nullptr,&staging))) return false;
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID; rasterDesc.CullMode = D3D11_CULL_NONE; rasterDesc.DepthClipEnable = TRUE;
    ComPtr<ID3D11RasterizerState> raster;
    if (FAILED(device.Device()->CreateRasterizerState(&rasterDesc,&raster))) return false;
    struct Vertex { float position[3], normal[3], uv[4], pivot[2]; };
    Vertex vertices[] = { {{-1,-1,.25f},{.5f,1,.1f},{}}, {{0,1,.25f},{.5f,1,.1f},{}}, {{1,-1,.25f},{.5f,1,.1f},{}} };
    const D3DVERTEXELEMENT9 elements[] = {
        {0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0},
        {0,12,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_NORMAL,0},
        {0,24,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_TEXCOORD,0},
        {0,40,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,3}
    };
    const float identity[] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    unsigned checked = 0;
    std::ifstream index("_work/build/gen/fx11/effects/effects.index");
    std::string line;
    while (std::getline(index,line))
    {
        const std::string key = line.substr(0,line.find('\t'));
        if ((key.compare(0,9,"billboard") != 0 && key.compare(0,16,"foliagebillboard") != 0) ||
            key.compare(0,18,"billboardsizelimit") == 0 || key.find('@') != std::string::npos) continue;
        Display::D3D11::C_D3D11Effect* effect = nullptr;
        if (FAILED(library.Get(device.Device(),key.c_str(),&effect))) return false;
        const void* bytes = nullptr; SIZE_T size = 0;
        ComPtr<ID3D11InputLayout> layout;
        if (FAILED(effect->GetPassVertexShaderBytecode("MaterialPrepass",0,&bytes,&size)) ||
            FAILED(Display::D3D11::CreateGameInputLayout(device.Device(),elements,4,bytes,size,layout))) return false;
        for (unsigned test = 0; test < 2; ++test)
        {
            float projection[16]; std::memcpy(projection,identity,sizeof(identity)); projection[15] = test ? 2.f : 1.f;
            for (auto& v : vertices) v.normal[2] = test ? .9f : .1f;
            ComPtr<ID3D11Buffer> vb;
            if (FAILED(device.CreateVertexBuffer(sizeof(vertices),vertices,false,vb))) return false;
            if (FAILED(effect->SetRawParameter("g_mWorld",identity,sizeof(identity))) ||
                FAILED(effect->SetRawParameter("g_mViewProj",projection,sizeof(projection)))) return false;
            if (effect->HasParameter("g_mView") && FAILED(effect->SetRawParameter("g_mView",identity,sizeof(identity)))) return false;
            const float view[] = {0,0,10,1}, hero[] = {10,0,0,1}, lod[] = {0,0,1,1}, scale[] = {1,1,1,1};
            for (const auto& parameter : {std::pair<const char*,const float*>{"g_vViewPosition",view},
                {"g_vHeroCarPos",hero},{"g_lodFade",lod},{"g_lodDistanceScale",scale}})
                if (effect->HasParameter(parameter.first) && FAILED(effect->SetRawParameter(parameter.first,parameter.second,16))) return false;
            const unsigned char white[] = {255,255,255,255}; const void* mips[] = {white}; const UINT sizes[] = {4};
            ComPtr<ID3D11Texture2D> albedo; ComPtr<ID3D11ShaderResourceView> albedoView;
            if (FAILED(device.CreateTexture2DFromPCData(1,1,1,D3DFMT_A8R8G8B8,mips,sizes,albedo,albedoView)) ||
                FAILED(effect->SetTexture("g_tAlbedo",albedoView.Get()))) return false;
            UINT alpha = 0;
            if (effect->HasParameter("BlurPCAlphaEnabled") && FAILED(effect->SetRawParameter("BlurPCAlphaEnabled",&alpha,sizeof(alpha)))) return false;
            auto context = device.Context(); context->ClearState(); D3DX11InvalidateBoundState(3,15);
            ID3D11RenderTargetView* targets[] = {device.RenderTargetView(),depthView.Get()};
            context->OMSetRenderTargets(2,targets,nullptr);
            const float clear[] = {-7,-7,-7,-7}; context->ClearRenderTargetView(depthView.Get(),clear);
            const D3D11_VIEWPORT viewport = {0,0,64,64,0,1}; context->RSSetViewports(1,&viewport); context->RSSetState(raster.Get());
            UINT stride = sizeof(Vertex), offset = 0; ID3D11Buffer* buffer = vb.Get();
            context->IASetVertexBuffers(0,1,&buffer,&stride,&offset); context->IASetInputLayout(layout.Get());
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            if (FAILED(effect->Apply("MaterialPrepass",0,context))) return false;
            context->Draw(3,0); context->OMSetRenderTargets(0,nullptr,nullptr); context->CopyResource(staging.Get(),depth.Get());
            D3D11_MAPPED_SUBRESOURCE mapped = {};
            if (FAILED(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped))) return false;
            const float value = *(const float*)((const char*)mapped.pData + 32*mapped.RowPitch + 32*sizeof(float));
            context->Unmap(staging.Get(),0);
            const float expected = 0.f;
            if (!std::isfinite(value) || std::abs(value-expected)>1e-6f)
            { std::fprintf(stderr,"Billboard prepass %s case%u depth %.9g expected %.9g\n",key.c_str(),test,value,expected); return false; }
            ++checked;
        }
    }
    device.Context()->ClearState(); D3DX11InvalidateBoundState(3,15);
    std::printf("Billboard prepass: %u GPU zero-depth samples including clip w=2\n",checked);
    return checked == 8;
}
