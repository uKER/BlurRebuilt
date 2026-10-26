// [Blur1PCDX11] Capture actual packed UI shader outputs before rasterization.
static bool CheckPackedPolygonShaders(Display::D3D11::C_D3D11Device& device)
{
    using Microsoft::WRL::ComPtr;
    const char* shaders[] = {"vsPolygonStroke","vsPolygonTrail","vsPolygonGradient","vsPolygonMultiply"};
    unsigned checked = 0;
    for (unsigned type=0;type<4;++type)
    {
        const std::string path=std::string("_work/build/gen/dx11_builtins/ui/")+shaders[type]+".cso";
        std::ifstream file(path,std::ios::binary);
        std::vector<char> bytes((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
        if (bytes.empty()) return false;
        ComPtr<ID3D11ShaderReflection> reflection;
        if (FAILED(D3DReflect(bytes.data(),bytes.size(),IID_PPV_ARGS(&reflection)))) return false;
        D3D11_SHADER_DESC shader={}; reflection->GetDesc(&shader);
        std::vector<D3D11_SO_DECLARATION_ENTRY> outputs;
        std::vector<std::string> names; names.reserve(shader.OutputParameters);
        UINT stride=0;
        for (UINT i=0;i<shader.OutputParameters;++i)
        {
            D3D11_SIGNATURE_PARAMETER_DESC parameter={};
            if (FAILED(reflection->GetOutputParameterDesc(i,&parameter))) return false;
            names.emplace_back(parameter.SemanticName);
            D3D11_SO_DECLARATION_ENTRY entry={}; entry.SemanticName=names.back().c_str();
            entry.SemanticIndex=parameter.SemanticIndex;
            for (UINT c=0;c<4;++c) entry.ComponentCount+=(parameter.Mask>>c)&1;
            stride+=entry.ComponentCount*sizeof(float); outputs.push_back(entry);
        }
        const D3DVERTEXELEMENT9 strokeElements[] = {
            {0,0,D3DDECLTYPE_SHORT2,0,D3DDECLUSAGE_POSITION,0},
            {0,4,D3DDECLTYPE_SHORT2,0,D3DDECLUSAGE_TEXCOORD,0}
        };
        const D3DVERTEXELEMENT9 flatElements[] = {
            {0,0,D3DDECLTYPE_SHORT2,0,D3DDECLUSAGE_POSITION,0},
            {0,4,D3DDECLTYPE_UBYTE4,0,D3DDECLUSAGE_COLOR,0}
        };
        ComPtr<ID3D11VertexShader> vs; ComPtr<ID3D11GeometryShader> capture;
        ComPtr<ID3D11InputLayout> layout;
        if (FAILED(device.Device()->CreateVertexShader(bytes.data(),bytes.size(),nullptr,&vs)) ||
            FAILED(device.Device()->CreateGeometryShaderWithStreamOutput(bytes.data(),bytes.size(),outputs.data(),
                (UINT)outputs.size(),&stride,1,D3D11_SO_NO_RASTERIZED_STREAM,nullptr,&capture)) ||
            FAILED(Display::D3D11::CreateGameInputLayout(device.Device(),type<2?strokeElements:flatElements,2,bytes.data(),bytes.size(),layout))) return false;
        struct Vertex { short position[2], uv[2]; };
        const Vertex vertices[]={{{-16384,8192},{-8192,16384}},{{16384,-8192},{8192,-16384}}};
        float constants[256*4]={};
        for (UINT start:{32u,39u,43u,47u}) for (UINT i=0;i<4;++i) constants[(start+i)*4+i]=1;
        constants[47*4]=2; constants[48*4+1]=3;
        const float colour[]={.2f,.4f,.6f,.8f}; std::memcpy(constants+37*4,colour,sizeof(colour));
        const float texture[]={.125f,-.25f,.5f,2.f}; std::memcpy(constants+38*4,texture,sizeof(texture));
        const float wave[]={.7f,.37f,.3f,0}; std::memcpy(constants+52*4,wave,sizeof(wave));
        const float defaultValue[]={0,0,0,1};
        ComPtr<ID3D11Buffer> vb, cb, defaults, so, staging;
        if (FAILED(device.CreateVertexBuffer(sizeof(vertices),vertices,false,vb)) ||
            FAILED(device.CreateVertexBuffer(sizeof(defaultValue),defaultValue,false,defaults))) return false;
        D3D11_BUFFER_DESC desc={}; desc.ByteWidth=sizeof(constants); desc.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
        D3D11_SUBRESOURCE_DATA data={}; data.pSysMem=constants;
        if (FAILED(device.Device()->CreateBuffer(&desc,&data,&cb))) return false;
        desc.ByteWidth=2*stride; desc.BindFlags=D3D11_BIND_STREAM_OUTPUT;
        if (FAILED(device.Device()->CreateBuffer(&desc,nullptr,&so))) return false;
        desc.BindFlags=0; desc.Usage=D3D11_USAGE_STAGING; desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
        if (FAILED(device.Device()->CreateBuffer(&desc,nullptr,&staging))) return false;
        auto context=device.Context(); context->ClearState(); D3DX11InvalidateBoundState(3,15);
        UINT offset=0,vertexStride=sizeof(Vertex); ID3D11Buffer* buffer=vb.Get();
        context->IASetVertexBuffers(0,1,&buffer,&vertexStride,&offset);
        buffer=defaults.Get(); const UINT zeroStride=0;
        context->IASetVertexBuffers(31,1,&buffer,&zeroStride,&offset);
        context->IASetInputLayout(layout.Get()); context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        context->VSSetShader(vs.Get(),nullptr,0); buffer=cb.Get(); context->VSSetConstantBuffers(0,1,&buffer);
        context->GSSetShader(capture.Get(),nullptr,0); buffer=so.Get(); context->SOSetTargets(1,&buffer,&offset);
        context->Draw(2,0); context->SOSetTargets(0,nullptr,nullptr); context->CopyResource(staging.Get(),so.Get());
        D3D11_MAPPED_SUBRESOURCE mapped={};
        if (FAILED(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped))) return false;
        bool valid=true;
        for (UINT vertex=0;vertex<2;++vertex)
        {
            const float divisor=type==0?32767.f:1.f;
            const float position[]={vertices[vertex].position[0]/divisor,vertices[vertex].position[1]/divisor,0,1};
            const float uv[]={texture[0]+vertices[vertex].uv[0]/divisor*texture[2],texture[1]+vertices[vertex].uv[1]/divisor*texture[3]};
            const float* result=(const float*)((const char*)mapped.pData+vertex*stride);
            for (const auto& output:outputs)
            {
                for (UINT c=0;c<output.ComponentCount;++c)
                {
                    float expected=position[c];
                    if (_stricmp(output.SemanticName,"COLOR")==0)
                    {
                        expected=type<2?colour[c]:position[c];
                        if (type==3 && output.SemanticIndex==1 && c<2) expected*=c==0?2.f:3.f;
                    }
                    else if (_stricmp(output.SemanticName,"TEXCOORD")==0 && output.SemanticIndex==0) expected=uv[c];
                    if (!std::isfinite(result[c]) || std::abs(result[c]-expected)>1e-4f)
                    { std::fprintf(stderr,"Packed %s vertex%u %s%u[%u]: %.9g expected %.9g\n",shaders[type],vertex,output.SemanticName,output.SemanticIndex,c,result[c],expected); valid=false; }
                    ++checked;
                }
                result+=output.ComponentCount;
            }
        }
        context->Unmap(staging.Get(),0); context->ClearState(); D3DX11InvalidateBoundState(3,15);
        if (!valid) return false;
    }
    std::printf("Packed stroke/trail/gradient/multiply: %u GPU components passed\n",checked);
    return true;
}
