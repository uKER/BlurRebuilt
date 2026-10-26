// [Blur1PCDX11] Actual sampling plus byte-exact CPU round trips through padded
// native staging rows/slices, using the production upload/lock conversion.
#include "Display2/Device/D3D11TextureFormats.h"

static bool CheckLuminanceTextures(Display::D3D11::C_D3D11Device& device)
{
    using Microsoft::WRL::ComPtr;
    using namespace Display2::D3D11TextureFormats;
    const D3DFORMAT formats[] = {D3DFMT_L8,D3DFMT_A8L8,D3DFMT_L16};
    unsigned samples = 0, roundTrips = 0;
    for (unsigned dimension=0; dimension<3; ++dimension)
    {
        const char* types[] = {"Texture2D","TextureCube","Texture3D"};
        const char* coords[] = {"float2(.5,.5)","float3(1,0,0)","float3(.5,.5,.5)"};
        std::wstring paths[2];
        for (unsigned mip=0; mip<2; ++mip)
        {
            const std::string source = std::string(types[dimension])+" image:register(t0); SamplerState state:register(s0); "
                "float4 main():SV_Target { return image.SampleLevel(state,"+coords[dimension]+","+std::to_string(mip)+"); }";
            ComPtr<ID3DBlob> code, errors;
            if (FAILED(D3DCompile(source.data(),source.size(),nullptr,nullptr,nullptr,"main","ps_5_0",0,0,&code,&errors))) return false;
            paths[mip] = L"_work/build/gen/probe_luminance_"+std::to_wstring(dimension)+L"_"+std::to_wstring(mip)+L".cso";
            if (FAILED(D3DWriteBlobToFile(code.Get(),paths[mip].c_str(),TRUE))) return false;
        }
        for (D3DFORMAT format : formats)
        {
            DXGI_FORMAT storage; unsigned pcBytes,nativeBytes;
            if (!LuminanceLayout(format,&storage,&pcBytes,&nativeBytes)) return false;
            const unsigned faces = dimension == 1 ? 6 : 1;
            std::vector<std::vector<unsigned char>> payload(faces*2);
            std::vector<const void*> data(faces*2);
            std::vector<UINT> sizes(faces*2);
            for (unsigned face=0; face<faces; ++face)
                for (unsigned mip=0; mip<2; ++mip)
                {
                    const unsigned index=face*2+mip, edge=2>>mip;
                    const unsigned texels=edge*edge*(dimension == 2 ? edge : 1);
                    payload[index].resize(texels*pcBytes);
                    for (unsigned texel=0; texel<texels; ++texel)
                    {
                        const unsigned short luma = static_cast<unsigned short>((mip ? 203 : 74)*257+13);
                        unsigned char* p=payload[index].data()+texel*pcBytes;
                        if (format == D3DFMT_L16) std::memcpy(p,&luma,2);
                        else { p[0]=mip ? 203 : 74; if (pcBytes==2) p[1]=mip ? 91 : 173; }
                    }
                    data[index]=payload[index].data(); sizes[index]=static_cast<UINT>(payload[index].size());
                }
            ComPtr<ID3D11Texture2D> texture2D;
            ComPtr<ID3D11Texture3D> texture3D;
            ComPtr<ID3D11ShaderResourceView> view;
            HRESULT result;
            if (dimension == 0) result=device.CreateTexture2DFromPCData(2,2,2,format,data.data(),sizes.data(),texture2D,view);
            else if (dimension == 1) result=device.CreateTextureCubeFromPCData(2,2,format,data.data(),sizes.data(),texture2D,view);
            else result=device.CreateTexture3DFromPCData(2,2,2,2,format,data.data(),sizes.data(),texture3D,view);
            ID3D11Resource* resource=dimension == 2 ? static_cast<ID3D11Resource*>(texture3D.Get()) : texture2D.Get();
            if (FAILED(result) || GetPCFormat(resource)!=format) return false;
            for (unsigned mip=0; mip<2; ++mip)
            {
                for (unsigned pass=0; pass<3; ++pass)
                {
                    view.Reset();
                    if (FAILED(Display::D3D11::CreateTextureView(device.Device(),resource,pass==1,&view)) ||
                        FAILED(device.DrawTexturedQuadView(L"_work/build/gen/dx11_builtins/display/vsBasicRenderTextured2D.cso",
                            paths[mip].c_str(),view.Get()))) return false;
                    unsigned char pixel[4] = {};
                    if (FAILED(device.ReadPixel(32,32,pixel))) return false;
                    const float luma=(mip ? 203.f : 74.f)/255;
                    const bool gamma=pass==1 && format!=D3DFMT_L16;
                    const int expected=static_cast<int>(std::lround(255*(gamma ? std::pow((luma+.055f)/1.055f,2.4f) : luma)));
                    const int alpha=format==D3DFMT_A8L8 ? (mip ? 91 : 173) : 255;
                    if (std::abs(int(pixel[0])-expected)>1 || std::abs(int(pixel[1])-expected)>1 ||
                        std::abs(int(pixel[2])-expected)>1 || pixel[3]!=alpha)
                    { std::fprintf(stderr,"Luminance format=%u dimension=%u mip=%u gamma=%u RGBA %u %u %u %u expected=%d/%d\n",
                        format,dimension,mip,gamma,pixel[0],pixel[1],pixel[2],pixel[3],expected,alpha); return false; }
                    ++samples;
                }
                for (unsigned face=0; face<faces; ++face)
                {
                    const unsigned edge=2>>mip, depth=dimension==2 ? edge : 1;
                    ComPtr<ID3D11Resource> staging;
                    if (dimension==2)
                    {
                        D3D11_TEXTURE3D_DESC desc={}; texture3D->GetDesc(&desc);
                        desc.Width=desc.Height=desc.Depth=edge; desc.MipLevels=1; desc.Usage=D3D11_USAGE_STAGING;
                        desc.BindFlags=0; desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
                        ComPtr<ID3D11Texture3D> copy;
                        if (FAILED(device.Device()->CreateTexture3D(&desc,nullptr,&copy))) return false;
                        staging=copy;
                    }
                    else
                    {
                        D3D11_TEXTURE2D_DESC desc={}; texture2D->GetDesc(&desc);
                        desc.Width=desc.Height=edge; desc.MipLevels=desc.ArraySize=1; desc.Usage=D3D11_USAGE_STAGING;
                        desc.BindFlags=desc.MiscFlags=0; desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
                        ComPtr<ID3D11Texture2D> copy;
                        if (FAILED(device.Device()->CreateTexture2D(&desc,nullptr,&copy))) return false;
                        staging=copy;
                    }
                    device.Context()->CopySubresourceRegion(staging.Get(),0,0,0,0,resource,face*2+mip,nullptr);
                    D3D11_MAPPED_SUBRESOURCE mapped={};
                    if (FAILED(device.Context()->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped))) return false;
                    std::vector<unsigned char> locked(edge*edge*depth*pcBytes);
                    ConvertLuminance(format,edge,edge,depth,mapped.pData,mapped.RowPitch,mapped.DepthPitch,
                        locked.data(),edge*pcBytes,edge*edge*pcBytes,false);
                    device.Context()->Unmap(staging.Get(),0);
                    if (locked!=payload[face*2+mip]) { std::fprintf(stderr,"Luminance round trip format=%u dimension=%u face=%u mip=%u\n",format,dimension,face,mip); return false; }
                    ++roundTrips;
                }
            }
        }
    }
    // A midpoint between unlike texels distinguishes decoding before filtering
    // from a shader-side decode after filtering. Alpha remains linear.
    const char* filteredSource = "Texture2D image:register(t0); SamplerState state:register(s0); "
        "float4 main():SV_Target {return image.SampleLevel(state,float2(.5,.5),0);}";
    ComPtr<ID3DBlob> code, errors;
    const wchar_t* filteredPath=L"_work/build/gen/probe_luminance_filtered.cso";
    if (FAILED(D3DCompile(filteredSource,strlen(filteredSource),nullptr,nullptr,nullptr,"main","ps_5_0",0,0,&code,&errors)) ||
        FAILED(D3DWriteBlobToFile(code.Get(),filteredPath,TRUE))) return false;
    for (unsigned withAlpha=0; withAlpha<2; ++withAlpha)
    {
        const unsigned char l8[]={64,192}, al8[]={64,91,192,173};
        const void* data[]={withAlpha ? al8 : l8}; const UINT sizes[]={withAlpha ? 4u : 2u};
        ComPtr<ID3D11Texture2D> texture; ComPtr<ID3D11ShaderResourceView> view;
        if (FAILED(device.CreateTexture2DFromPCData(2,1,1,withAlpha ? D3DFMT_A8L8 : D3DFMT_L8,data,sizes,texture,view))) return false;
        for (unsigned gamma=0; gamma<2; ++gamma)
        {
            view.Reset();
            if (FAILED(Display::D3D11::CreateTextureView(device.Device(),texture.Get(),gamma!=0,&view)) ||
                FAILED(device.DrawTexturedQuadView(L"_work/build/gen/dx11_builtins/display/vsBasicRenderTextured2D.cso",
                    filteredPath,view.Get(),D3DTEXF_LINEAR))) return false;
            const float expected=gamma ? (std::pow((64.f/255+.055f)/1.055f,2.4f)+std::pow((192.f/255+.055f)/1.055f,2.4f))*.5f : 128.f/255;
            const int rgb=static_cast<int>(std::lround(expected*255)), alpha=withAlpha ? 132 : 255;
            unsigned char pixel[4]={};
            if (FAILED(device.ReadPixel(32,32,pixel)) || std::abs(int(pixel[0])-rgb)>1 ||
                std::abs(int(pixel[1])-rgb)>1 || std::abs(int(pixel[2])-rgb)>1 || std::abs(int(pixel[3])-alpha)>1)
            { std::fprintf(stderr,"Filtered luminance alpha=%u gamma=%u RGBA %u %u %u %u expected=%d/%d\n",withAlpha,gamma,pixel[0],pixel[1],pixel[2],pixel[3],rgb,alpha); return false; }
            ++samples;
        }
    }
    std::printf("PC luminance: %u GPU sample checks (including decode-before-filter); %u byte-exact mip/face/volume PC-layout readback round trips\n",samples,roundTrips);
    return true;
}
