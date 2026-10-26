#ifndef __GRAPHICS_FX_FX_H__
#define __GRAPHICS_FX_FX_H__

#include <Btl/Utility/String.h>
#ifdef FX_INCLUDE_DEVICE_TOOLS_ENUMS
#include <Display/Device/DeviceToolsEnums.h>
#else
#include <Display/Device/DeviceTypesEnums.h>
#endif
#include <Display/Utils/Matrix.h>
#include <fx/I_Effect.h>
#include <fx/RenderFlags.h>
#include <btl/Containers/bitvector.h>

namespace Sys
{
	class I_FileSystem;
}

namespace FX
{

typedef Btl::BitVectorTemplate<Btl::BitArrayBase<Btl::Vector<U32, Btl::VectorParameters_Array<4> > > > SharedBindingBits;

enum E_ShaderBuildType
{
E_ShaderBuildType_Debug,
E_ShaderBuildType_Release,
E_ShaderBuildType_Profile,
E_ShaderBuildType_Speed,
E_ShaderBuildType_Master,
};

struct Parameter;

typedef U32 ParameterHandle;
typedef U32 TechniqueHandle;
typedef int SamplerStateHandle;

/// initialise the FX system
void Initialise();

/// free the FX system
void Shutdown();

void SetFileSystem(Sys::I_FileSystem* pFs);

/// set a parameter value
void SetParameter(ParameterHandle param, const void *pData);

/// set a parameter value
void SetParameter(Parameter *pParam, const void *pData);

/// gets parameter data
void* GetParameterData(Parameter *pParam);

/// gets parameter data
void* GetParameterData(ParameterHandle param);

/// set render state
void SetRenderFlags(RenderFlags renderFlags);

/// get the path that shaders are supposed to be loaded from
Btl::String GetShaderPath();

/// get sampler state handle for the given sampler state
SamplerStateHandle GetSamplerState(Display::E_TextureAddress u, Display::E_TextureAddress v, Display::E_TextureAddress w,
	Display::E_TextureFilter mip, Display::E_TextureFilter min, Display::E_TextureFilter mag,
	U32 MaxAnisotropy, bool SRGBTexture, S8 MipMapLodBias);

/// set sampler states on a texture parameter
void SetSamplerState(ParameterHandle param, SamplerStateHandle state);

/// set sampler states on a texture parameter
void SetSamplerState(Parameter *pParam, SamplerStateHandle state);

/// set the value of the parameter with the given name (SLOW - to be used infrequently)
void SetSharedParameterByName(const char *pszName, const void *pData);

/// set the value of a matrix parameter with the given name (SLOW - to be used infrequently)
void SetSharedParameterByName(const char *pszName, const Display::C_Matrix &mtx);

/// returns the default value for the given shared parameter
void *GetSharedParameterDefaultValue(const char *pszName, int *piSize);

/// returns the default value for the given parameter
void *GetParameterDefaultValue(I_Effect *pEffect, const char *pszName, int *piSize);

/// returns the default value for the given parameter
void *GetParameterDefaultValue(I_Effect *pEffect, Parameter *p, int *piSize);

/// return a default 4x4 white texture
void *GetDefaultGPUTexture();

/// get shared parameter
/// the returned FX::Parameter pointer is only valid until a new shader is loaded, so you shouldn't hang on to it
ParameterHandle GetSharedParameter(const char *pszName);

/// get combined usage/mask bits for an effect
void GetSharedBindingUsage(const I_Effect *pEffect, SharedBindingBits *pUsage, SharedBindingBits *pMask);

/// load an effect
I_Effect *LoadEffect(const char *pszFilename);

/// preload a list of effects
void LoadEffects(const Btl::Vector<const char *> &pszFilename, Btl::Vector<FX::I_Effect*> &pointers);

/// get the original filename for an effect
Util::C_String GetFilename(I_Effect *pEffect);

/// delete an effect
void ReleaseEffect(I_Effect *pEffect);

/// check whether any effects need reloading
void ReloadEffects();

/// restore default data
void RestoreDefaultData();

/// get reload count
U32 GetReloadCount();

/// cast handle to parameter *
Parameter *GetParameterPtr(ParameterHandle handle);

/// register a subtechnique
/// parameters:
///		pszName		- used to create the dynamic debug variable. also used to identify the technique
TechniqueHandle RegisterSubtechnique(const char *pszName);

} // namespace FX

#endif
