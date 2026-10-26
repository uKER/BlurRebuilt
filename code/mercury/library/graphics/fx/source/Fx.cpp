#include "stdafx.h"

#include <display.h>

#include "fx/Effect.h"
#include "fx/Shared.h"
#include "fx/System.h"
#include "fx/SamplerStates.h"
#include "fx/Fx.h"
#include "fx/FxContext.h"
#include "fx/Manager.h"
#include "fx/DebugVariables.h"
#include "fx/ReloadableEffect.h"

#include <system/DebugLog.h>
#include <system/Memory.h>

DEBUG_ENUM(FX::E_ShaderBuildType)
// add support for extra shader build types by adding them into this debug variable enum
DE_OPTION(FX::E_ShaderBuildType_Debug,	"Debug")
DE_OPTION(FX::E_ShaderBuildType_Speed,	"Speed")
END_DEBUG_ENUM

namespace FX
{

Debug::Logger &g_Log = Debug::GetLogger("graphics.fx");

Debug::Enum<E_ShaderBuildType> g_ShaderType("Graphics.FX.ShaderType", E_ShaderBuildType_Speed, Debug::Persist());
E_ShaderBuildType g_CurrentShaderType = E_ShaderBuildType_Speed;

/// global render flags
RenderFlags g_RenderFlags;

/// cast handle to parameter *
Parameter *GetParameterPtr(ParameterHandle handle)
{
	if (handle == -1)
		return g_System.GetNullParameter();
	else
		return &g_SharedEffect.Parameters[handle];
}

/// preload a list of effects
void LoadEffects(const Btl::Vector<const char *> &list, Btl::Vector<FX::I_Effect*> &pointers)
{
	g_Manager.LoadEffects(list, pointers);
}

/// load an effect
I_Effect *LoadEffect(const char *pszFilename)
{
	RESOURCE_MEMORY_ID("FX");

	MEMORY_TRACKER_PUSH("graphics.fx");
	I_Effect *pFX = g_Manager.LoadEffect(pszFilename);
	MEMORY_TRACKER_POP("graphics.fx");
	return pFX;
}

/// delete an effect
void ReleaseEffect(I_Effect *pEffect)
{
	g_Manager.ReleaseEffect((Effect*)pEffect);
}

/// get reload count
U32 GetReloadCount()
{
	return g_Manager.GetReloadCount();
}

/// check whether any effects need reloading
void ReloadEffects()
{
	PERF_EVENT("FX.ReloadEffects");
	MEMORY_TRACKER_PUSH("graphics.fx");
	g_Manager.ReloadEffects(g_ShaderType != g_CurrentShaderType);
	g_CurrentShaderType = g_ShaderType;
#ifndef CFG_GOLD
	::FX::DebugVariables::g_Manager.Update();
#endif
	MEMORY_TRACKER_POP("graphics.fx");
}

void RestoreDefaultData()
{
	g_SharedEffect.RestoreDefaultData();
}

/// initialise the FX system
void Initialise()
{
	RESOURCE_MEMORY_ID("FX");
	MEMORY_TRACKER_PUSH("graphics.fx");
	g_System.Initialise();
	MEMORY_TRACKER_POP("graphics.fx");
}

/// free the FX system
void Shutdown()
{
	g_System.Shutdown();
}

void SetFileSystem(Sys::I_FileSystem* pFs)
{
	g_Manager.SetFileSystem(pFs);
}

/// set a parameter value
void SetParameter(Parameter *pParam, const void *pData)
{
	BIZ_ASSERT(pParam);
	BIZ_ASSERT(pData);

	
	if (pParam->Type == NGLFX_PARAM_SAMPLER)
	{
		// [Blur2PC][x64] pData points at a texture pointer; the sampler word holds its
		// Core::PointerToSlot32 slot (the pointer itself on x86). Any further words follow it.
		pParam->Sampler->Texture = Core::PointerToSlot32(*(void* const*)pData);
		if (pParam->Size > 1)
			memcpy(&pParam->Sampler->iSamplerState, (const U8*)pData + sizeof(void*), (pParam->Size - 1)*4);
		return;
	}
	memcpy(pParam->Data, pData, pParam->Size*4);
}

/// gets parameter data
void* GetParameterData(Parameter *pParam)
{
	BIZ_ASSERT(pParam);

	return pParam->Data;
}

/// set a parameter value
void SetParameter(ParameterHandle param, const void *pData)
{
	SetParameter(GetParameterPtr(param), pData);
}

/// gets parameter data
void* GetParameterData(ParameterHandle param)
{
	return GetParameterData(GetParameterPtr(param));
}

/// sets the global render flags
void SetRenderFlags(RenderFlags renderFlags)
{
	g_RenderFlags = renderFlags;
}

/// set sampler states on a texture parameter
void SetSamplerState(ParameterHandle param, SamplerStateHandle state)
{
	SetSamplerState(GetParameterPtr(param), state);
}

/// set the value of the parameter with the given name (SLOW - to be used infrequently)
void SetSharedParameterByName(const char *pszName, const void *pData)
{
	ParameterHandle p = GetSharedParameter(pszName);
	SetParameter(p, pData);
}

/// set the value of a matrix parameter with the given name (SLOW - to be used infrequently)
void SetSharedParameterByName(const char *pszName, const Display::C_Matrix &mtx)
{
	MathsStorage::MatrixRow44A16 fxMtx;

	Maths::HMat44 tmp = Maths::Load<Maths::HMat44>(&mtx);
	Maths::Store(tmp, &fxMtx);

	FX::SetSharedParameterByName(pszName, (void*)&fxMtx);
}

/// set sampler states on a texture parameter
void SetSamplerState(Parameter *pParam, SamplerStateHandle state)
{
	if (pParam->Sampler)
		pParam->Sampler->iSamplerState = state;
}

/// get sampler state handle for the given sampler state
SamplerStateHandle GetSamplerState(Display::E_TextureAddress u, Display::E_TextureAddress v, Display::E_TextureAddress w,
	Display::E_TextureFilter mip, Display::E_TextureFilter min, Display::E_TextureFilter mag,
	U32 MaxAnisotropy, bool SRGBTexture, S8 MipMapLodBias)
{
	return g_SamplerStateMgr.CreateSamplerState(u,v,w,mip,min,mag,MaxAnisotropy,SRGBTexture,MipMapLodBias);
}

/// get combined usage/mask bits for an effect
void GetSharedBindingUsage(const I_Effect *_pEffect, SharedBindingBits *pUsage, SharedBindingBits *pMask)
{
	const Effect *pEffect = (const Effect*)_pEffect;
	//Effect could be NULL of loading failed
	if (pEffect)
		for (int iTech=0; iTech<pEffect->Techniques.Size(); iTech++)
		{
			for (int iPass=0; iPass<pEffect->Techniques[iTech].Passes.Size(); iPass++)
			{
				const Pass *pPass = &pEffect->Techniques[iTech].Passes[iPass];
				*pUsage |= pPass->SharedBindingUsage;
				*pMask |= pPass->SharedBindingMask;
			}
		}
}

/// get shared parameter
ParameterHandle GetSharedParameter(const char *pszName)
{
	return (ParameterHandle)g_SharedEffect.GetSharedParameterIdxByName(pszName);
}

/// returns the default value for the given shared parameter
void *GetSharedParameterDefaultValue(const char *pszName, int *piSize)
{
	return GetParameterDefaultValue(&g_SharedEffect, GetParameterPtr(GetSharedParameter(pszName)), piSize);
}

/// returns the default value for the given parameter
void *GetParameterDefaultValue(I_Effect *pEffect, const char *pszName, int *piSize)
{
	return GetParameterDefaultValue(pEffect, pEffect->GetParameter(pszName), piSize);
}

/// returns the default value for the given parameter
void *GetParameterDefaultValue(I_Effect *pEffect, Parameter *p, int *piSize)
{
	int cSize = 0;
	void *pRtn = NULL;
	if (((Effect*)pEffect)->ParameterData.Size())
	{
		int iOffset = (((U32*)p->Data - &((Effect*)pEffect)->ParameterData[0]));
		pRtn = &((Effect*)pEffect)->DefaultParameterData[iOffset];
		cSize = p->Size;
	}
	if (piSize)
		*piSize = cSize;
	return pRtn;
}

/// return a default 4x4 white texture
void *GetDefaultGPUTexture()
{
	return g_System.GetDefaultTexture();
}

/// return the path that shader are supposed to be loaded from
Btl::String GetShaderPath()
{
	switch (g_ShaderType)
	{
	case E_ShaderBuildType_Debug:
		return "graphics\\shaders\\debug\\";
	case E_ShaderBuildType_Release:
		return "graphics\\shaders\\release\\";
	case E_ShaderBuildType_Profile:
		return "graphics\\shaders\\profile\\";
	case E_ShaderBuildType_Speed:
		return "graphics\\shaders\\speed\\";
	case E_ShaderBuildType_Master:
		return "graphics\\shaders\\master\\";
	}
	assert(0);
	return "";
}


} // namespace FX
