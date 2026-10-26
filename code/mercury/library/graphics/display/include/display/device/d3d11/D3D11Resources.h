#ifndef DISPLAY_DEVICE_D3D11_RESOURCES_H
#define DISPLAY_DEVICE_D3D11_RESOURCES_H

// [Blur1PCDX11] The engine's resource names now refer to Display2 objects
// backed by ID3D11 resources. The authored PC format enums remain unchanged.
#include <Display2.h>
#include "Display/Device/DeviceTypesEnums.h"
#include "Display/Buffer/VertexFormat.h"
#include "Display/Texture/TextureParams.h"

namespace Display
{
typedef Display2::C_VertexDeclaration C_VertexDeclaration;
typedef Display2::C_VertexBuffer C_VertexBuffer;
typedef Display2::C_IndexBuffer C_IndexBuffer;
typedef Display2::C_VertexShader C_VertexShader;
typedef Display2::C_PixelShader C_PixelShader;
typedef Display2::C_VertexShaderParam C_VertexShaderParameter;
typedef Display2::C_PixelShaderParam C_PixelShaderParameter;
typedef Display2::C_PixelShaderParam C_PixelShaderParameterArray;
typedef Display2::C_Surface C_Surface;
typedef Display2::C_SurfaceParams C_SurfaceParams;
typedef Display2::C_Texture C_Texture;
typedef Display2::C_Texture C_Texture2D;
typedef Display2::C_Texture C_Texture3D;
typedef Display2::C_Texture C_TextureCube;

struct C_PixelShaderConstantLock
{
    U32 startRegister, endRegister, salt;
    bool enabled;
};
struct C_PixelShaderConstantLockKey { U32 index, salt; };
enum E_PixelShaderConstantLock { E_PixelShaderConstantLock_Count = 4 };

namespace D3D11
{
class C_RenderTexture2D : public Display2::C_RenderTexture2D
{
public:
    void Resolve(const U32 level = 0) const;
};
class C_RenderTextureCube : public Display2::C_RenderTextureCube {};
}

typedef D3D11::C_RenderTexture2D C_RenderTexture2D;
typedef D3D11::C_RenderTextureCube C_RenderTextureCube;
}

struct C_ShaderProfile { int m_Size; int* m_pData; };
struct C_ShaderData
{
    C_ShaderProfile m_Profiles[Display::E_ShaderProfile_Count];
    const C_ShaderProfile& GetProfile(Display::E_ShaderProfile profile) const
    { return m_Profiles[profile]; }
};

#endif
