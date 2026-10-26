// [Blur1PCDX11] Verify road MRT depth independently of packed normals.
#include <cstring>
static bool CheckRoadPrepass(Display::D3D11::C_D3D11Device& device,
                             Display::D3D11::C_D3D11EffectLibrary& library)
{
    using Microsoft::WRL::ComPtr;
    HRESULT targetResult = device.CreateOffscreenTarget(64,64);
    if (FAILED(targetResult)) { std::fprintf(stderr,"Road prepass offscreen target: %08lx\n",targetResult); return false; }
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = desc.Height = 64; desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R32_FLOAT; desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    ComPtr<ID3D11Texture2D> depth, staging;
    ComPtr<ID3D11RenderTargetView> depthView;
    if (FAILED(device.Device()->CreateTexture2D(&desc,nullptr,&depth)) ||
        FAILED(device.Device()->CreateRenderTargetView(depth.Get(),nullptr,&depthView))) { std::fprintf(stderr,"Road prepass depth target creation failed\n"); return false; }
    desc.BindFlags = 0; desc.Usage = D3D11_USAGE_STAGING; desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(device.Device()->CreateTexture2D(&desc,nullptr,&staging))) { std::fprintf(stderr,"Road prepass staging creation failed\n"); return false; }
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID; rasterDesc.CullMode = D3D11_CULL_NONE; rasterDesc.DepthClipEnable = TRUE;
    ComPtr<ID3D11RasterizerState> raster;
    if (FAILED(device.Device()->CreateRasterizerState(&rasterDesc,&raster))) { std::fprintf(stderr,"Road prepass raster creation failed\n"); return false; }
    struct Vertex { float position[3], normal[3], uv[4]; };
    Vertex vertices[] = { {{-1,-1,.25f},{.5f,1,.1f},{}}, {{0,1,.25f},{.5f,1,.1f},{}}, {{1,-1,.25f},{.5f,1,.1f},{}} };
    const D3DVERTEXELEMENT9 elements[] = {
        {0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0},
        {0,12,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_NORMAL,0},
        {0,24,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_TEXCOORD,0}
    };
    const float identity[] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    unsigned checked = 0;
    std::ifstream index("_work/build/gen/fx11/effects/effects.index");
    std::string line;
    while (std::getline(index,line))
    {
        const std::string key = line.substr(0,line.find('\t'));
        if (key.compare(0,5,"road_") != 0 || key.find('@') != std::string::npos ||
            key.size()<3 || key.compare(key.size()-3,3,"_nw") != 0) continue;
        Display::D3D11::C_D3D11Effect* effect = nullptr;
        // [Blur1PCDX11] Name early failures so a full-suite stop can be attributed.
        HRESULT result = library.Get(device.Device(),key.c_str(),&effect);
        if (FAILED(result)) { std::fprintf(stderr,"Road prepass %s load: %08lx\n",key.c_str(),result); return false; }
        const void* bytes = nullptr; SIZE_T size = 0;
        ComPtr<ID3D11InputLayout> layout;
        result = effect->GetPassVertexShaderBytecode("MaterialPrepass",0,&bytes,&size);
        if (FAILED(result)) { std::fprintf(stderr,"Road prepass %s shader: %08lx\n",key.c_str(),result); return false; }
        result = Display::D3D11::CreateGameInputLayout(device.Device(),elements,3,bytes,size,layout);
        if (FAILED(result)) { std::fprintf(stderr,"Road prepass %s layout: %08lx\n",key.c_str(),result); return false; }
        for (unsigned test = 0; test < 2; ++test)
        {
            float projection[16]; std::memcpy(projection,identity,sizeof(identity)); projection[15] = test ? 2.f : 1.f;
            for (auto& v : vertices) v.normal[2] = test ? .9f : .1f;
            ComPtr<ID3D11Buffer> vb;
            result = device.CreateVertexBuffer(sizeof(vertices),vertices,false,vb);
            if (FAILED(result)) { std::fprintf(stderr,"Road prepass %s vertex buffer: %08lx\n",key.c_str(),result); return false; }
            result = effect->SetRawParameter("g_mWorld",identity,sizeof(identity));
            if (FAILED(result)) { std::fprintf(stderr,"Road prepass %s world: %08lx\n",key.c_str(),result); return false; }
            result = effect->SetRawParameter("g_mViewProj",projection,sizeof(projection));
            if (FAILED(result)) { std::fprintf(stderr,"Road prepass %s projection: %08lx\n",key.c_str(),result); return false; }
            if (effect->HasParameter("g_mView"))
            {
                result = effect->SetRawParameter("g_mView",identity,sizeof(identity));
                if (FAILED(result)) { std::fprintf(stderr,"Road prepass %s view: %08lx\n",key.c_str(),result); return false; }
            }
            UINT alpha = 0;
            if (effect->HasParameter("BlurPCAlphaEnabled"))
            {
                result = effect->SetRawParameter("BlurPCAlphaEnabled",&alpha,sizeof(alpha));
                if (FAILED(result)) { std::fprintf(stderr,"Road prepass %s alpha: %08lx\n",key.c_str(),result); return false; }
            }
            auto context = device.Context(); context->ClearState(); D3DX11InvalidateBoundState(3,15);
            ID3D11RenderTargetView* targets[] = {device.RenderTargetView(),depthView.Get()};
            context->OMSetRenderTargets(2,targets,nullptr);
            const float clear[] = {-7,-7,-7,-7}; context->ClearRenderTargetView(depthView.Get(),clear);
            const D3D11_VIEWPORT viewport = {0,0,64,64,0,1}; context->RSSetViewports(1,&viewport); context->RSSetState(raster.Get());
            UINT stride = sizeof(Vertex), offset = 0; ID3D11Buffer* buffer = vb.Get();
            context->IASetVertexBuffers(0,1,&buffer,&stride,&offset); context->IASetInputLayout(layout.Get());
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            result = effect->Apply("MaterialPrepass",0,context);
            if (FAILED(result)) { std::fprintf(stderr,"Road prepass %s apply: %08lx\n",key.c_str(),result); return false; }
            context->Draw(3,0); context->OMSetRenderTargets(0,nullptr,nullptr); context->CopyResource(staging.Get(),depth.Get());
            D3D11_MAPPED_SUBRESOURCE mapped = {};
            result = context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped);
            if (FAILED(result)) { std::fprintf(stderr,"Road prepass %s readback: %08lx\n",key.c_str(),result); return false; }
            const float value = *(const float*)((const char*)mapped.pData + 32*mapped.RowPitch + 32*sizeof(float));
            context->Unmap(staging.Get(),0);
            const float expected = .25f / projection[15];
            if (!std::isfinite(value) || std::abs(value-expected)>1e-6f)
            { std::fprintf(stderr,"Road prepass %s case%u depth %.9g expected %.9g\n",key.c_str(),test,value,expected); return false; }
            ++checked;
        }
    }
    device.Context()->ClearState(); D3DX11InvalidateBoundState(3,15);
    std::printf("Road prepass: %u GPU depth samples independent of normal, including clip w=2\n",checked);
    return checked > 0;
}
