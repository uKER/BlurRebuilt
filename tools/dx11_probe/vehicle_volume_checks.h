// [Blur1PCDX11] Exercise the retail FX volume name through native metadata,
// then capture damage alpha from the actual recovered paint vertex shader.
static bool CheckVehicleVolumeBinding(Display::D3D11::C_D3D11Device& device,
                                     Display::D3D11::C_D3D11EffectLibrary& library)
{
    using Microsoft::WRL::ComPtr;
    const char* retailName = "g_sVehicleDamageVolumeTexture3d";
    const char* nativeName = "g_sVehicleDamageVolumeTexture";
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ComPtr<ID3D11SamplerState> sampler;
    if (FAILED(device.Device()->CreateSamplerState(&samplerDesc, &sampler))) return false;
    std::ifstream index("_work/build/gen/fx11/effects/effects.recovered");
    if (!index) { std::puts("Retail vehicle volume checks: recovered package unavailable"); return true; }
    std::string key;
    unsigned aliases = 0;
    while (std::getline(index, key))
    {
        Display::D3D11::C_D3D11Effect* effect = nullptr;
        if (FAILED(library.Get(device.Device(), key.c_str(), &effect))) return false;
        if (!effect->HasParameter(nativeName)) continue;
        if (!effect->HasParameter(retailName))
        { std::fprintf(stderr, "Retail volume reflection failed: %s\n", key.c_str()); return false; }
        const char* texture = library.TextureForSampler(key.c_str(), retailName);
        if (!texture || std::strcmp(texture, nativeName))
        { std::fprintf(stderr, "Retail volume binding failed: %s\n", key.c_str()); return false; }
        ++aliases;
    }
    if (!aliases || index.bad()) return false;
    Display::D3D11::C_D3D11Effect* paint = nullptr;
    if (FAILED(library.Get(device.Device(), "veh_Paint_a_b_c_LN", &paint))) return false;
    const float identity[] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    float vehicle[32] = {}; vehicle[24] = vehicle[25] = vehicle[26] = 1;
    const float damageScale[4] = {1,0,0,0};
    if (FAILED(paint->SetRawParameter("g_mWorld", identity, sizeof(identity))) ||
        FAILED(paint->SetRawParameter("g_mViewProj", identity, sizeof(identity))) ||
        FAILED(paint->SetRawParameter("g_VehicleVtxParams", vehicle, sizeof(vehicle))) ||
        FAILED(paint->SetRawParameter("g_vCodeDamageScale", damageScale, sizeof(damageScale)))) return false;
    const void* bytes = nullptr; SIZE_T size = 0;
    if (FAILED(paint->GetPassVertexShaderBytecode("MaterialAlphaLuminance", 0, &bytes, &size))) return false;
    D3D11_SO_DECLARATION_ENTRY output = {0,"TEXCOORD",2,3,1,0};
    const UINT stride = sizeof(float);
    ComPtr<ID3D11GeometryShader> capture;
    if (FAILED(device.Device()->CreateGeometryShaderWithStreamOutput(bytes, size, &output, 1,
        &stride, 1, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, &capture))) return false;
    struct Vertex { float position[4], normal[4], uv[4], morph[4], normalMorph[4], colour[4]; };
    const Vertex vertex = {{0,0,0,1},{.5f,.5f,1,0},{0,0,0,0},
        {.5f,.5f,.5f,0},{.5f,.5f,.5f,0},{.5f,.5f,.5f,1}};
    const D3DVERTEXELEMENT9 elements[] = {
        {0,0,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_POSITION,0},
        {0,16,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_NORMAL,0},
        {0,32,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_TEXCOORD,0},
        {0,48,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_TEXCOORD,2},
        {0,64,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_TEXCOORD,3},
        {0,80,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_COLOR,0}
    };
    ComPtr<ID3D11InputLayout> layout; ComPtr<ID3D11Buffer> vb, so, staging;
    if (FAILED(Display::D3D11::CreateGameInputLayout(device.Device(), elements, 6, bytes, size, layout)) ||
        FAILED(device.CreateVertexBuffer(sizeof(vertex), &vertex, false, vb))) return false;
    D3D11_BUFFER_DESC bufferDesc = {}; bufferDesc.ByteWidth = stride; bufferDesc.BindFlags = D3D11_BIND_STREAM_OUTPUT;
    if (FAILED(device.Device()->CreateBuffer(&bufferDesc, nullptr, &so))) return false;
    bufferDesc.BindFlags = 0; bufferDesc.Usage = D3D11_USAGE_STAGING; bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(device.Device()->CreateBuffer(&bufferDesc, nullptr, &staging))) return false;
    auto context = device.Context();
    for (unsigned alpha : {0u,64u,255u})
    {
        const unsigned char texel[] = {0,0,0,static_cast<unsigned char>(alpha)};
        D3D11_TEXTURE3D_DESC desc = {}; desc.Width = desc.Height = desc.Depth = desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA data = {}; data.pSysMem = texel; data.SysMemPitch = data.SysMemSlicePitch = 4;
        ComPtr<ID3D11Texture3D> volume; ComPtr<ID3D11ShaderResourceView> view;
        const char* name = library.TextureForSampler("veh_Paint_a_b_c_LN", retailName);
        if (!name || FAILED(device.Device()->CreateTexture3D(&desc, &data, &volume)) ||
            FAILED(device.Device()->CreateShaderResourceView(volume.Get(), nullptr, &view)) ||
            FAILED(paint->SetTexture(retailName, view.Get())) ||
            FAILED(paint->SetSampler("DamageVolumeTextureSampler3d", sampler.Get()))) return false;
        context->ClearState(); D3DX11InvalidateBoundState(3,15);
        if (FAILED(paint->Apply("MaterialAlphaLuminance", 0, context))) return false;
        UINT offset = 0, vertexStride = sizeof(Vertex); ID3D11Buffer* buffer = vb.Get();
        context->IASetVertexBuffers(0, 1, &buffer, &vertexStride, &offset);
        context->IASetInputLayout(layout.Get()); context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        context->GSSetShader(capture.Get(), nullptr, 0); buffer = so.Get(); context->SOSetTargets(1, &buffer, &offset);
        context->Draw(1, 0); context->SOSetTargets(0, nullptr, nullptr); context->CopyResource(staging.Get(), so.Get());
        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (FAILED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped))) return false;
        const float actual = *static_cast<const float*>(mapped.pData);
        context->Unmap(staging.Get(), 0);
        const float expected = 2.f * alpha / 255.f;
        if (!std::isfinite(actual) || std::abs(actual - expected) > 1e-5f)
        { std::fprintf(stderr, "Vehicle volume alpha %u: %.9g expected %.9g\n", alpha, actual, expected); return false; }
    }
    context->ClearState(); D3DX11InvalidateBoundState(3,15);
    std::printf("Retail vehicle volume: %u metadata aliases, 3 actual VS samples passed\n", aliases);
    return true;
}
