// [Blur1PCDX11] Sample with the actual live sampler obtained after FX11 Apply.
// Distinct mip colours expose bias scaling, NONE and trilinear interpolation.
static bool CheckLiveSamplers(Display::D3D11::C_D3D11Device& device,
                              Display::D3D11::C_D3D11EffectLibrary& library)
{
    using Microsoft::WRL::ComPtr;
    if (FAILED(device.CreateOffscreenTarget(64,64))) return false;
    Display::D3D11::C_D3D11Effect* effect = nullptr;
    if (FAILED(library.Get(device.Device(), "MeshEffect", &effect, "Material"))) { std::fprintf(stderr,"Sampler effect load failed\n"); return false; }
    const void* bytes = nullptr;
    SIZE_T size = 0;
    ComPtr<ID3D11ShaderReflection> reflection;
    if (FAILED(effect->GetPassPixelShaderBytecode("Material", 0, &bytes, &size)) ||
        FAILED(D3DReflect(bytes, size, IID_PPV_ARGS(&reflection)))) { std::fprintf(stderr,"Sampler effect reflection failed\n"); return false; }
    D3D11_SHADER_INPUT_BIND_DESC binding = {};
    D3D11_SHADER_DESC shaderDesc = {};
    reflection->GetDesc(&shaderDesc);
    bool found = false;
    for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
    {
        reflection->GetResourceBindingDesc(i, &binding);
        if (binding.Type == D3D_SIT_SAMPLER && std::string(binding.Name) == "TextureSampler") { found = true; break; }
    }
    if (!found) { std::fprintf(stderr,"MeshEffect TextureSampler absent\n"); return false; }

    const char* vsSource = "float4 main(uint id:SV_VertexID):SV_Position {"
        "float2 uv=float2((id<<1)&2,id&2); return float4(uv*float2(2,-2)+float2(-1,1),0,1);}";
    const char* psSource = "Texture2D T:register(t0); SamplerState S:register(s0);"
        "cbuffer C:register(b0) {float4 coord;}"
        "float4 main(float4 p:SV_Position):SV_Target {return T.Sample(S,p.xy*coord.xy+coord.zw);}";
    ComPtr<ID3DBlob> vsCode, psCode, errors;
    if (FAILED(D3DCompile(vsSource, std::strlen(vsSource), nullptr, nullptr, nullptr,
        "main", "vs_5_0", 0, 0, &vsCode, &errors)) ||
        FAILED(D3DCompile(psSource, std::strlen(psSource), nullptr, nullptr, nullptr,
        "main", "ps_5_0", 0, 0, &psCode, &errors))) { std::fprintf(stderr,"Sampler shader compile: %s\n",errors ? static_cast<const char*>(errors->GetBufferPointer()) : "unknown"); return false; }
    ComPtr<ID3D11VertexShader> vs;
    ComPtr<ID3D11PixelShader> ps;
    if (FAILED(device.Device()->CreateVertexShader(vsCode->GetBufferPointer(), vsCode->GetBufferSize(), nullptr, &vs)) ||
        FAILED(device.Device()->CreatePixelShader(psCode->GetBufferPointer(), psCode->GetBufferSize(), nullptr, &ps))) return false;
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = 16; cbDesc.Usage = D3D11_USAGE_DEFAULT; cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ComPtr<ID3D11Buffer> cb;
    if (FAILED(device.Device()->CreateBuffer(&cbDesc, nullptr, &cb))) return false;
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = textureDesc.Height = 8; textureDesc.MipLevels = 4; textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE; textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    std::vector<unsigned> pixels[4];
    D3D11_SUBRESOURCE_DATA data[4] = {};
    for (unsigned mip = 0; mip < 4; ++mip)
    {
        const unsigned edge = 8 >> mip;
        pixels[mip].resize(edge * edge);
        for (unsigned y = 0; y < edge; ++y)
            for (unsigned x = 0; x < edge; ++x)
                pixels[mip][y * edge + x] = mip == 0 ? (x < 4 ? 0xff0000ff : 0xffff0000) :
                    mip == 1 ? 0xff00ff00 : mip == 2 ? 0xffff0000 : 0xffffffff;
        data[mip].pSysMem = pixels[mip].data(); data[mip].SysMemPitch = edge * 4;
    }
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> view;
    if (FAILED(device.Device()->CreateTexture2D(&textureDesc, data, &texture)) ||
        FAILED(device.Device()->CreateShaderResourceView(texture.Get(), nullptr, &view))) return false;
    D3D11_RASTERIZER_DESC rasterDesc = {};
    if (!Display::D3D11::MakeRasterizerDesc(D3DFILL_SOLID, D3DCULL_NONE, false, &rasterDesc)) return false;
    ComPtr<ID3D11RasterizerState> raster;
    if (FAILED(device.Device()->CreateRasterizerState(&rasterDesc, &raster))) return false;
    struct Test { const char* name; D3DTEXTUREADDRESS address; D3DTEXTUREFILTERTYPE filter, mip;
        int bias; float scale, u; unsigned colour[4]; };
    const Test tests[] = {
        {"wrap",D3DTADDRESS_WRAP,D3DTEXF_POINT,D3DTEXF_POINT,0,0,1.25f,{255,0,0,255}},
        {"clamp",D3DTADDRESS_CLAMP,D3DTEXF_POINT,D3DTEXF_POINT,0,0,1.25f,{0,0,255,255}},
        {"mirror",D3DTADDRESS_MIRROR,D3DTEXF_POINT,D3DTEXF_POINT,0,0,1.25f,{0,0,255,255}},
        {"mirror once",D3DTADDRESS_MIRRORONCE,D3DTEXF_POINT,D3DTEXF_POINT,0,0,-.25f,{255,0,0,255}},
        {"border ARGB",D3DTADDRESS_BORDER,D3DTEXF_POINT,D3DTEXF_POINT,0,0,1.25f,{64,32,16,128}},
        {"linear mag",D3DTADDRESS_WRAP,D3DTEXF_LINEAR,D3DTEXF_POINT,0,0,.5f,{128,0,128,255}},
        {"mip one",D3DTADDRESS_WRAP,D3DTEXF_POINT,D3DTEXF_POINT,0,.25f,0,{0,255,0,255}},
        {"bias +16",D3DTADDRESS_WRAP,D3DTEXF_POINT,D3DTEXF_POINT,16,.25f,0,{0,0,255,255}},
        {"bias -16",D3DTADDRESS_WRAP,D3DTEXF_POINT,D3DTEXF_POINT,-16,.25f,0,{255,0,0,255}},
        {"bias +8 trilinear",D3DTADDRESS_WRAP,D3DTEXF_POINT,D3DTEXF_LINEAR,8,.25f,0,{0,128,128,255}},
        {"mips disabled",D3DTADDRESS_WRAP,D3DTEXF_POINT,D3DTEXF_NONE,32,.25f,0,{255,0,0,255}},
    };
    unsigned checked = 0;
    for (const auto& test : tests)
    {
        D3D11_SAMPLER_DESC desc = {};
        if (!Display::D3D11::MakeSamplerDesc(test.filter, test.filter, test.mip,
            test.address, D3DTADDRESS_CLAMP, D3DTADDRESS_WRAP, 1, &desc, 0x80402010, test.bias))
            { std::fprintf(stderr,"Sampler descriptor %s failed\n",test.name); return false; }
        ComPtr<ID3D11SamplerState> sampler, bound;
        if (FAILED(device.Device()->CreateSamplerState(&desc, &sampler)) ||
            FAILED(library.SetTextureSampler(device.Device(), "MeshEffect",
                checked & 1 ? "g_tTexture" : "TextureSampler", sampler.Get(), "Material")))
            { std::fprintf(stderr,"Live sampler creation/binding %s failed\n",test.name); return false; }
        // The second apply models the immediate parameter path. Read what the
        // actual game effect bound, then use that object for the GPU samples.
        if (FAILED(effect->Apply("Material", 0, device.Context())) ||
            FAILED(effect->Apply("Material", 0, device.Context()))) return false;
        device.Context()->PSGetSamplers(binding.BindPoint, 1, &bound);
        if (bound.Get() != sampler.Get()) { std::fprintf(stderr,"Live sampler %s not preserved by apply\n",test.name); return false; }
        device.Context()->ClearState(); D3DX11InvalidateBoundState(3,15);
        ID3D11RenderTargetView* target = device.RenderTargetView();
        device.Context()->OMSetRenderTargets(1, &target, nullptr);
        const D3D11_VIEWPORT viewport = {0,0,64,64,0,1};
        device.Context()->RSSetViewports(1, &viewport);
        device.Context()->RSSetState(raster.Get());
        device.Context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device.Context()->VSSetShader(vs.Get(), nullptr, 0);
        device.Context()->PSSetShader(ps.Get(), nullptr, 0);
        ID3D11ShaderResourceView* rawView = view.Get();
        device.Context()->PSSetShaderResources(0, 1, &rawView);
        ID3D11SamplerState* rawSampler = bound.Get();
        device.Context()->PSSetSamplers(0, 1, &rawSampler);
        const float coords[] = {test.scale,test.scale,test.u,.5f};
        device.Context()->UpdateSubresource(cb.Get(), 0, nullptr, coords, 0, 0);
        ID3D11Buffer* rawBuffer = cb.Get();
        device.Context()->PSSetConstantBuffers(0, 1, &rawBuffer);
        device.Context()->Draw(3,0);
        unsigned char rgba[4] = {};
        if (FAILED(device.ReadPixel(32,32,rgba))) return false;
        for (unsigned c = 0; c < 4; ++c)
            if (std::abs(int(rgba[c]) - int(test.colour[c])) > 1)
            { std::fprintf(stderr, "Live sampler %s: RGBA %u %u %u %u\n", test.name,rgba[0],rgba[1],rgba[2],rgba[3]); return false; }
        ++checked;
    }
    D3D11_SAMPLER_DESC defaults = {};
    ComPtr<ID3D11SamplerState> sampler;
    if (!Display::D3D11::MakeSamplerDesc(D3DTEXF_LINEAR,D3DTEXF_LINEAR,D3DTEXF_LINEAR,
        D3DTADDRESS_WRAP,D3DTADDRESS_WRAP,D3DTADDRESS_WRAP,1,&defaults) ||
        FAILED(device.Device()->CreateSamplerState(&defaults,&sampler)) ||
        FAILED(library.SetTextureSampler(device.Device(),"MeshEffect","TextureSampler",sampler.Get(),"Material"))) return false;
    device.Context()->ClearState(); D3DX11InvalidateBoundState(3,15);
    std::printf("Live FX11 samplers: %u GPU samples passed (address/filter/border/encoded bias/NONE/immediate)\n",checked);
    return true;
}
