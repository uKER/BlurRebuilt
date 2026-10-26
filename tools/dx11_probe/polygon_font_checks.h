// [Blur1PCDX11] Capture the actual built-in font VS output from PC packed
// SHORT2/UBYTE4 vertices. This checks fetch conversion independently of pixels.
static bool CheckPackedPolygonFont(Display::D3D11::C_D3D11Device& device)
{
    using Microsoft::WRL::ComPtr;
    std::ifstream file("_work/build/gen/dx11_builtins/ui/vsPolygonFont_FinnedOuter.cso", std::ios::binary);
    std::vector<char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (bytes.empty()) return false;
    ComPtr<ID3D11VertexShader> vs;
    ComPtr<ID3D11GeometryShader> capture;
    ComPtr<ID3D11InputLayout> layout;
    // /Gec retains POSITION in SM5 bytecode; reflection supplies its spelling.
    ComPtr<ID3D11ShaderReflection> reflection;
    if (FAILED(D3DReflect(bytes.data(), bytes.size(), IID_PPV_ARGS(&reflection)))) return false;
    D3D11_SHADER_DESC shader = {};
    reflection->GetDesc(&shader);
    std::vector<D3D11_SO_DECLARATION_ENTRY> outputs;
    std::vector<std::string> names;
    names.reserve(shader.OutputParameters);
    for (UINT i = 0; i < shader.OutputParameters; ++i)
    {
        D3D11_SIGNATURE_PARAMETER_DESC output = {};
        reflection->GetOutputParameterDesc(i, &output);
        if (_stricmp(output.SemanticName, "COLOR") == 0 ||
            output.SystemValueType == D3D_NAME_POSITION || _stricmp(output.SemanticName, "POSITION") == 0)
        {
            names.emplace_back(output.SemanticName);
            D3D11_SO_DECLARATION_ENTRY entry = {};
            entry.SemanticName = names.back().c_str(); entry.SemanticIndex = output.SemanticIndex;
            entry.ComponentCount = 4;
            outputs.push_back(entry);
        }
    }
    if (outputs.size() != 2) return false;
    const UINT stride = 32;
    const D3DVERTEXELEMENT9 elements[] = {
        {0,0,D3DDECLTYPE_SHORT2,0,D3DDECLUSAGE_POSITION,0},
        {0,4,D3DDECLTYPE_UBYTE4,0,D3DDECLUSAGE_COLOR,0}
    };
    if (FAILED(device.Device()->CreateVertexShader(bytes.data(), bytes.size(), nullptr, &vs)) ||
        FAILED(device.Device()->CreateGeometryShaderWithStreamOutput(bytes.data(), bytes.size(),
            outputs.data(), (UINT)outputs.size(), &stride, 1, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, &capture)) ||
        FAILED(Display::D3D11::CreateGameInputLayout(device.Device(), elements, 2, bytes.data(), bytes.size(), layout)))
        return false;
    struct Vertex { short position[2]; unsigned char vector[4]; };
    const Vertex vertices[] = { {{-16384,8192},{127,127,1,0}}, {{16384,-8192},{127,127,1,0}} };
    ComPtr<ID3D11Buffer> vb, cb, so, staging;
    if (FAILED(device.CreateVertexBuffer(sizeof(vertices), vertices, false, vb))) return false;
    float constants[256 * 4] = {};
    for (UINT start : {32u,39u}) for (UINT i = 0; i < 4; ++i) constants[(start + i) * 4 + i] = 1;
    for (UINT i = 0; i < 4; ++i) constants[37 * 4 + i] = 1;
    constants[51 * 4] = 1;
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(constants); desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    D3D11_SUBRESOURCE_DATA data = {}; data.pSysMem = constants;
    if (FAILED(device.Device()->CreateBuffer(&desc, &data, &cb))) return false;
    desc.ByteWidth = 2 * stride; desc.BindFlags = D3D11_BIND_STREAM_OUTPUT;
    if (FAILED(device.Device()->CreateBuffer(&desc, nullptr, &so))) return false;
    desc.BindFlags = 0; desc.Usage = D3D11_USAGE_STAGING; desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(device.Device()->CreateBuffer(&desc, nullptr, &staging))) return false;
    auto context = device.Context();
    context->ClearState(); D3DX11InvalidateBoundState(3,15);
    UINT offset = 0, vertexStride = sizeof(Vertex);
    ID3D11Buffer* buffer = vb.Get(); context->IASetVertexBuffers(0,1,&buffer,&vertexStride,&offset);
    context->IASetInputLayout(layout.Get()); context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context->VSSetShader(vs.Get(),nullptr,0); buffer = cb.Get(); context->VSSetConstantBuffers(0,1,&buffer);
    context->GSSetShader(capture.Get(),nullptr,0); buffer = so.Get(); context->SOSetTargets(1,&buffer,&offset);
    context->Draw(2,0); context->SOSetTargets(0,nullptr,nullptr);
    context->CopyResource(staging.Get(),so.Get());
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (FAILED(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped))) return false;
    bool valid = true;
    for (UINT vertex = 0; vertex < 2; ++vertex)
    {
        const float* result = (const float*)mapped.pData + vertex * 8;
        for (UINT output = 0; output < 2; ++output)
        {
            const bool colour = _stricmp(outputs[output].SemanticName,"COLOR") == 0;
            const float expected[] = { vertices[vertex].position[0] / 32767.f,
                vertices[vertex].position[1] / 32767.f,0,1 };
            for (UINT component = 0; component < 4; ++component)
            {
                const float value = result[output * 4 + component];
                const float target = colour ? 1.f : expected[component];
                if (!std::isfinite(value) || std::fabs(value - target) > .00001f)
                { std::fprintf(stderr,"Packed font vertex %u %s[%u]: %.9g expected %.9g\n",
                    vertex,outputs[output].SemanticName,component,value,target); valid = false; }
            }
        }
    }
    context->Unmap(staging.Get(),0); context->ClearState(); D3DX11InvalidateBoundState(3,15);
    std::printf("Packed polygon font SHORT2/UBYTE4 fetch: %s\n",valid ? "passed" : "FAILED");
    return valid;
}
