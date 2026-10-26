
#include "stdafx.h"

#include <display.h>

#include "fx/Effect.h"
#include "fx/System.h"
#include "fx/Shared.h"
#include "fx/FxContext.h"
#include "fx/SamplerStates.h"
#include "fx/DebugVariables.h"
#include "fx/Technique.h"
#include "fx/ShaderImpl.h"
#include "fx/NativeFX11.h"
#include <system/debug/Blur2PCAutomation.h>
#include <btl/containers/vector.h>
#include <Display/Device/DeviceTypesEnums.h>
#include <Display/Context/Context.h>

namespace FX
{

FX::System g_System;

// [Blur2PC] Opt-in shader dumps (BLUR_DX11_DUMP_FX) go under dx11_trace\ in the working folder;
// they used to name one developer's worktree.
static const char* FxDumpDir() { CreateDirectoryA("dx11_trace", nullptr); return "dx11_trace"; }

// Point unbindable parameters at these globals, to avoid a branch in the update parameter loop.
Parameter STOREALIGN16 gFXNullParameter;
Maths::HMat44 gFXIdentityMatrix = Maths::HMat44();
Display::C_Texture2D* g_pFXDefaultTexture = 0; 
// [Blur2PC] default for vertex samplers: vehicle shaders read their damage volume through a vertex
// sampler, and when FX has no texture for it the 2D default was bound, which the shader read as full
// damage (dents and scratches on showroom and player cars). A zeroed volume means "no damage".
Display::C_Texture3D* g_pFXDefaultVertexTexture = 0;

#ifdef PLATFORM_PS3
Display::C_Texture2D* g_pFXDefaultFPTexture = 0; 
#endif

const Display::E_TextureAddress TranslateFXTextureAddressToDisplay(const U32 textureAddress)
{
	switch(textureAddress)
	{
	case 0: return Display::E_TextureAddress_Wrap;
	case 1: return Display::E_TextureAddress_Clamp;
	case 2: return Display::E_TextureAddress_Mirror;
	case 3: return Display::E_TextureAddress_Border;
	default:
		BIZ_ASSERT(0 && "Unsupported texture address");
		return Display::E_TextureAddress_Wrap;
	}
}

const Display::E_TextureFilter TranslateFXTextureFilterToDisplay(const U32 textureFilter)
{
	switch(textureFilter)
	{
	case 0: return Display::E_TextureFilter_None;
	case 1: return Display::E_TextureFilter_Point;
	case 2: return Display::E_TextureFilter_Linear;
	case 3: return Display::E_TextureFilter_Anisotropic;
	default:
		BIZ_ASSERT(0 && "Unsupported texture filter");
		return Display::E_TextureFilter_None;
	}
}

/// initialisation code
void System::Initialise()
{
	memset(&gFXNullParameter, 0, sizeof(gFXNullParameter));

	// create default texture
	assert(g_pFXDefaultTexture == 0);
	g_pFXDefaultTexture = new Display::C_Texture2D;
	
	if(!Display::Device()->CreateTexture2D(1, 1, 1, 0, Display::E_Format_A8R8G8B8, g_pFXDefaultTexture))
	{
		LOGFATAL(g_Log, "Failed to create the default FX system texture\n");
	}
	else
	{
#if !PLATFORM_PC
		U32 *pData = (U32*)g_pFXDefaultTexture->GetAddress();

		for (int i = 0; i < 4; i++)
			*(pData + i) = -1;
#endif
	}


	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetTextureName(g_pFXDefaultTexture, "_FxSystemDefault");
	#endif

	// [Blur2PC] see g_pFXDefaultVertexTexture
	g_pFXDefaultVertexTexture = new Display::C_Texture3D;
	if (Display::Device()->CreateTexture3D(1, 1, 1, 1, Display::E_Format_Linear, Display::E_Format_A32B32G32R32F, g_pFXDefaultVertexTexture))
	{
		Display::C_LockedBox box = g_pFXDefaultVertexTexture->LockBox3D(0);
		if (box.pBits)
			memset(box.pBits, 0, 16);
		g_pFXDefaultVertexTexture->UnlockBox3D(0);
	}

#if PLATFORM_PS3
	g_pFXDefaultFPTexture = new Display::C_Texture2D;
	if(!Display::Device()->CreateTexture2D(1, 1, 1, 0, Display::E_Format_A32B32G32R32F, g_pFXDefaultFPTexture))
	{
		LOGFATAL(g_Log, "Failed to create the default FX system FP texture\n");
	}
	else
	{
		U32 *pData = (U32*)g_pFXDefaultFPTexture->GetAddress();
		*pData = 0xffffffff;
	}

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetTextureName(g_pFXDefaultFPTexture, "_FxSystemDefaultFP");
	#endif
#endif

	m_iMaxParameters = 0;
}

/// set a parameter value
void System::SetParameter(Parameter *pParam, const void *pData)
{
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

/// shutdown code
void System::Shutdown()
{
	Display::Device()->DestroyTexture2D(g_pFXDefaultTexture);
	delete g_pFXDefaultTexture;
	g_pFXDefaultTexture = 0;

#if PLATFORM_PS3
	Display::Device()->DestroyTexture2D(g_pFXDefaultFPTexture);
	delete g_pFXDefaultFPTexture;
	g_pFXDefaultFPTexture = 0;
#endif
}

/// returns the NULL parameter which is returned for parameters that weren't found when calling GetParameter()
Parameter *System::GetNullParameter()
{
	return &gFXNullParameter;
}

/// get pointer to default texture which is assigned instead of NULL textures
gpuTexture *System::GetDefaultTexture()
{
	return (gpuTexture*)g_pFXDefaultTexture->GetGPUTexture();
}

// [Blur2PC] vertex-sampler replacement for the default texture (see g_pFXDefaultVertexTexture)
const void *GetDefaultVertexTextureFor(const void *pTexture)
{
	if (g_pFXDefaultVertexTexture && g_pFXDefaultTexture && pTexture == g_pFXDefaultTexture->GetGPUTexture())
		return g_pFXDefaultVertexTexture->GetGPUTexture();
	return pTexture;
}

#if PLATFORM_PS3
/// get pointer to default texture which is assigned instead of NULL textures to vertex texture samplers on the PS3
gpuTexture *System::GetDefaultFPTexture()
{
	return (gpuTexture*)g_pFXDefaultFPTexture->GetGPUTexture();
}
#endif

/// delete all shared bindings off a particular pixel shader of vertex shader
void System::DeleteSharedBindings(Effect *pEffect, Function *pFunc, bool bSamplersOnly = false)
{
	for (int i=0; i<pFunc->Bindings.Size(); i++)
	{
		// delete any bindings that are not float constants or textures
		if (pFunc->Bindings[i].Type != NGLFX_BIND_SAMPLER &&
			pFunc->Bindings[i].Type != NGLFX_BIND_FLOAT)
		{
			pFunc->Bindings.EraseByIndex(i);
			--i;
			continue;
		}

		// delete shared bindings and copy them to the 'SharedBindings' array
		if (pEffect->Parameters[pFunc->Bindings[i].ParameterIdx].bShared)
		{
			if (!bSamplersOnly || pFunc->Bindings[i].Type == NGLFX_BIND_SAMPLER)
			{
				pFunc->SharedBindings.PushBack(pFunc->Bindings[i]);
				pFunc->Bindings.EraseByIndex(i);
				--i;
			}
		}
	}

	pFunc->SharedBindings.TrimCapacity();
}

/// for each function binding calculate the offset into the parameter data block
void System::BindFunctionToParameters(Effect* effect, Function* function)
{
	for (int i = 0; i < function->Bindings.Size(); i++)
	{
		FunctionBinding* binding = &function->Bindings[i];
		Parameter* param = &effect->Parameters[binding->ParameterIdx];

		if (!param->bShared)
		{
#if !PLATFORM_PS3
			assert(binding->Offset == 0);
#endif
			binding->Source = NGLFX_BIND_PARAMETER;
			binding->Offset = (size_t)param->Data - (size_t)&effect->ParameterData[0] + binding->Offset*sizeof(F32);
		}
	}
}

/// for each function binding determine whether it is being set by the scene, or whether to use the effect defaults
void System::DetermineBindingsExportFlag(Effect* effect, Function* function)
{
	for (int i = 0; i < function->Bindings.Size(); i++)
	{
		FunctionBinding* binding = &function->Bindings[i];
		Parameter* param = &effect->Parameters[binding->ParameterIdx];

		binding->Exported = param->bExported && !param->bShared;
	}
}

#if PLATFORM_PS3

/// lookup pixel shader bindings
void System::EvaluatePixelShaderBindings(Effect *pEffect, Function *pShader)
{
	for (int i=0; i<pShader->Bindings.Size(); i++)
	{
		FunctionBinding *pBinding = &pShader->Bindings[i];
		if (pBinding->Type == NGLFX_BIND_SAMPLER)
			continue;
		assert(pBinding->Type == NGLFX_BIND_FLOAT);

		Parameter *pParam = &pEffect->Parameters[pBinding->ParameterIdx];

		Display2::C_PixelShaderParam p;
		Shader *pS1 = pShader->pShader;
		PixelShader *pS = (PixelShader*)pS1;

		p = pS->GetParamFromName(pBinding->Name);
		if (p == 0)
		{
			LOGWARN( g_Log, "Couldn't find parameter '%s' on CGB effect '%s.\n", pBinding->Name, pEffect->Name.Chars());
		}

		// while we're here, point shared parameter bindings to the shared parameter data list
		if (pParam->bShared)
		{
			g_SharedEffect.BindSharedParameter(pBinding, pParam);
		}

		if (p)
		{
			// convert FX binding into pixel shader patching offset
			for (int x=0; x<p->m_Offsets.Size(); x++)
			{
				pShader->PatchBindings.PushBack();
				PatchBinding *pPB = &pShader->PatchBindings.Back();
				pPB->Source = pBinding->Source;
				pPB->Offset = pBinding->Offset;
				pPB->PatchOffset = p->m_Offsets[x];
			}

			// make the offset point to the start of the parameter instead of to the subparameter inside it
			// this is so when a parameter is set, the pImm points to the start of the parameter
			pBinding->Offset = (size_t)pParam->Data - (size_t)&pEffect->ParameterData[0];
		}
	}

	pShader->PatchBindings.TrimCapacity();
}
#endif

#ifndef CFG_SPEED
/// dump shader bindings to debuglog on load (helper function)
void System::DumpBindings(Effect *pEffect, const Btl::Vector<FunctionBinding> &bindings)
{
	for (int i=0; i<bindings.Size(); i++)
	{
		const FunctionBinding *p = &bindings[i];
		const char *pszName = pEffect->Parameters[p->ParameterIdx].Name;
		const char *pszType = (p->Type==NGLFX_BIND_SAMPLER)?"Texture ":"Constant";
		const char *pszShared = (p->Shared)?"(Shared)":"";
		LOGDEBUG( g_Log, "%s %d - %s %s\n", pszType, p->Handle, pszName, pszShared);
	}
}

/// dump shader bindings to debuglog on load
void System::DumpBindings(Effect *pEffect)
{
	// Free all vertex and pixel shaders.
	LOGDEBUG( g_Log, "*** Bindings for %s ***\n", pEffect->Name.Chars());
	for (int i = 0; i < pEffect->Techniques.Size(); i++)
	{
		for (int j = 0; j < pEffect->Techniques[i].Passes.Size(); j++)
		{
			Pass* Pass = &pEffect->Techniques[i].Passes[j];

			LOGDEBUG( g_Log, "--- Vertex Shader Bindings\n");
			DumpBindings(pEffect, Pass->VertexShader.Bindings);
			LOGDEBUG( g_Log, "--- Pixel Shader Bindings\n");
			DumpBindings(pEffect, Pass->PixelShader.Bindings);
		}
	}
}
#endif

/// set the flags for which samplers are used in the given bindings
void System::DetermineSamplerUsageFlags(Display::C_SamplerUsageFlags *pFlags, const Btl::Vector<FunctionBinding> &bindings, int iShaderType)
{
	for (int i=0; i<bindings.Size(); i++)
	{
		const FunctionBinding *p = &bindings[i];
		if (p->Type == NGLFX_BIND_SAMPLER)
		{
			pFlags->SetEnabled(iShaderType, p->Handle);
		}
	}
}

/// set the flags for which vertex input attributes are used
void System::DetermineVertexInputAttributes(U32 *pFlags, Display2::C_VertexShader *pShader)
{
	Btl::Vector<Display2::C_ShaderSemantic> semantics;
	pShader->GetSemantics(semantics);

	U32 flags = 0;
	for (int i=0; i<semantics.Size(); i++)
	{
		flags |= (1<<(semantics[i].m_Usage + semantics[i].m_UsageIndex));
	}
	*pFlags = flags;
}

/// Function that is to be called when a new effect is loaded
void System::OnEffectLoad(Effect *pEffect)
{
	// Create sampler states
	for (int i = 0; i < pEffect->Parameters.Size(); i++)
	{
		Parameter *p = &pEffect->Parameters[i];
		if (p->Type != NGLFX_PARAM_SAMPLER)
			continue;

		int x = p->Sampler->iSamplerState;
		p->Sampler->iSamplerState = g_SamplerStateMgr.CreateSamplerState(
			TranslateFXTextureAddressToDisplay((x>>0)&7),
			TranslateFXTextureAddressToDisplay((x>>3)&7),
			TranslateFXTextureAddressToDisplay((x>>6)&7),
			TranslateFXTextureFilterToDisplay((x>>9)&3),
			TranslateFXTextureFilterToDisplay((x>>11)&3),
			TranslateFXTextureFilterToDisplay((x>>13)&3),
			((x>>16)&15),
			((x>>20)&1),
			((x>>21)&127));
	}

#ifndef CFG_SPEED
	// dump bindings
	DumpBindings(pEffect);
#endif

	assert(g_System.GetDefaultTexture()); // make sure you call FX::Initialise() before loading any FX files

	// point all sampler parameters to the default texture to avoid NULL pointer dereferences
	for (int i=0; i<pEffect->Parameters.Size(); i++)
	{
		Parameter *pParam = &pEffect->Parameters[i];
		if (pParam->Type == NGLFX_PARAM_SAMPLER)
			pParam->Sampler->Texture = Core::PointerToSlot32(g_System.GetDefaultTexture());	// [Blur2PC][x64]
	}

#if PLATFORM_PS3
	// for PS3 if there are any vertex texture bindings, set the default for those parameters to the fp default texture
	for (int i=0; i<pEffect->Techniques.Size(); i++)
	{
		for (int j=0; j<pEffect->Techniques[i].Passes.Size(); j++)
		{
			Pass *pPass = &pEffect->Techniques[i].Passes[j];
			for (int x=0; x<pPass->VertexShader.Bindings.Size(); x++)
			{
				FunctionBinding *pBind = &pPass->VertexShader.Bindings[x];
				if (pBind->Type == NGLFX_BIND_SAMPLER)
				{
					pEffect->Parameters[pBind->ParameterIdx].Sampler->Texture = Core::PointerToSlot32(g_System.GetDefaultFPTexture());	// [Blur2PC][x64]
				}
			}
		}
	}
#endif

	{
		static unsigned effectTraces = 0;
		const char* effectName = pEffect->Name.Chars();
		const bool vehicle = effectName && (strstr(effectName, "veh") != 0 || strstr(effectName, "Veh") != 0);
		if (effectTraces++ < 40 || vehicle)
		{
			Blur2PCAutomation::LogF("DX11 FX techniques %s count=%d", pEffect->Name.Chars(), pEffect->Techniques.Size());
			for (int t = 0; t < pEffect->Techniques.Size(); ++t)
				Blur2PCAutomation::LogF("DX11 FX techniques %s [%d] %s", pEffect->Name.Chars(), t,
					pEffect->Techniques[t].Name.Chars());
		}
	}

	// Bind all vertex and pixel shaders.
	for (int i = 0; i < pEffect->Techniques.Size(); i++)
	{
		for (int j = 0; j < pEffect->Techniques[i].Passes.Size(); j++)
		{
			Pass* pPass = &pEffect->Techniques[i].Passes[j];

			g_SharedEffect.RegisterShader(pEffect, pPass->PixelShader.Bindings, 0);
			g_SharedEffect.RegisterShader(pEffect, pPass->VertexShader.Bindings, 1);

			DetermineSamplerUsageFlags(&pPass->m_SamplerUsageFlags, pPass->VertexShader.Bindings, 1);
			DetermineSamplerUsageFlags(&pPass->m_SamplerUsageFlags, pPass->PixelShader.Bindings, 0);

			DeleteSharedBindings(pEffect, &pPass->VertexShader);
#if PLATFORM_PS3
			DeleteSharedBindings(pEffect, &pPass->PixelShader, true);
#else
			DeleteSharedBindings(pEffect, &pPass->PixelShader);
#endif
			DetermineBindingsExportFlag(pEffect, &pPass->VertexShader);
			DetermineBindingsExportFlag(pEffect, &pPass->PixelShader);

			if (pPass->VertexShader.Microcode)
				BindFunctionToParameters(pEffect, &pPass->VertexShader);
			if (pPass->PixelShader.Microcode)
				BindFunctionToParameters(pEffect, &pPass->PixelShader);

			// [Blur1PCDX11] FXB contains material metadata; FX5 supplies the native pass shaders.
			for (int shaderType = 0; shaderType < 2; ++shaderType)
			{
				Function& function = shaderType ? pPass->VertexShader : pPass->PixelShader;
				const E_ShaderType type = shaderType ? E_ShaderType_VertexShader : E_ShaderType_PixelShader;
				// [Blur1PCDX11] Opt-in complete retail evidence, including matched passes,
				// for recovering later shader revisions ("*" dumps every effect). Never substitute a pass.
				if (((getenv("BLUR_DX11_DUMP_VEHICLE_FX") &&
					strncmp(pEffect->Name.Chars(), "veh_", 4) == 0) ||
					(getenv("BLUR_DX11_DUMP_FX") && (strcmp(getenv("BLUR_DX11_DUMP_FX"), "*") == 0 ||
						strcmp(pEffect->Name.Chars(), getenv("BLUR_DX11_DUMP_FX")) == 0))) &&
					function.Microcode && function.cMicrocode)
				{
					char path[MAX_PATH] = {};
					char variant[MAX_PATH] = {};
					const char* filename = NativeFX11EffectName(pEffect);
					const char* basename = filename;
					for (const char* c = filename; *c; ++c) if (*c == '/' || *c == '\\') basename = c + 1;
					strcpy_s(variant, basename);
					if (char* extension = strrchr(variant, '.')) *extension = 0;
					sprintf_s(path, "%s\\fxdump_variants_%s_%s_%d_%s.bin", FxDumpDir(),
						variant, pEffect->Techniques[i].Name.Chars(), j,
						shaderType ? "vs" : "ps");
					FILE* file = nullptr;
					if (fopen_s(&file, path, "wb") == 0)
					{
						fwrite(function.Microcode, 1, function.cMicrocode, file);
						fclose(file);
					}
				}
				unsigned byteCount = 0;
				const void* bytecode = NativeFX11Shader(pEffect, i, j, type, &byteCount);
				if (bytecode && byteCount)
					function.pShader = CreateShader(const_cast<void*>(bytecode), byteCount, type);
				else if (function.Microcode && function.cMicrocode)
				{
					// [Blur1PCDX11] The retail FXB has passes our recompiled FX5 does not; keep
					// their D3D9 microcode on disk so the pass can be reconstructed.
					static unsigned dumps = 0;
					if (dumps++ < 40)
					{
						char path[MAX_PATH] = {};
						sprintf_s(path, "%s\\fxdump_%s_%s_%d_%s.bin", FxDumpDir(),
							pEffect->Name.Chars(), pEffect->Techniques[i].Name.Chars(), j,
							type == E_ShaderType_VertexShader ? "vs" : "ps");
						FILE* file = nullptr;
						if (fopen_s(&file, path, "wb") == 0)
						{
							fwrite(function.Microcode, 1, function.cMicrocode, file);
							fclose(file);
						}
						Blur2PCAutomation::LogF("DX11 FX dump %s / %s pass=%d %s bytes=%u", pEffect->Name.Chars(),
							pEffect->Techniques[i].Name.Chars(), j,
							type == E_ShaderType_VertexShader ? "vs" : "ps", function.cMicrocode);
					}
				}
				delete (U8*)function.Microcode;
				function.Microcode = NULL;
			}
			if (pPass->VertexShader.pShader)
				DetermineVertexInputAttributes(&pPass->m_VtxAttrUsageFlags, (VertexShader*)pPass->VertexShader.pShader);
		}
	}

#ifndef CFG_GOLD
	DebugVariables::g_Manager.AddEffect(pEffect);
#endif

	if (pEffect->Parameters.Size() > m_iMaxParameters)
		m_iMaxParameters = pEffect->Parameters.Size();

	assert(m_iMaxParameters < FX_SYSTEM_MAX_LOCAL_PARAMETERS);
}

/// Function that is to be called when an effect is deleted
void System::OnEffectRelease(Effect *Effect)
{
	// Free all vertex and pixel shaders.
	for (int i = 0; i < Effect->Techniques.Size(); i++)
	{
		for (int j = 0; j < Effect->Techniques[i].Passes.Size(); j++)
		{
			Pass* Pass = &Effect->Techniques[i].Passes[j];

			if (Pass->VertexShader.pShader)
				FreeShader(Pass->VertexShader.pShader, E_ShaderType_VertexShader);
			if (Pass->PixelShader.pShader)
				FreeShader(Pass->PixelShader.pShader, E_ShaderType_PixelShader);

		}
	}

#ifndef CFG_GOLD
	DebugVariables::g_Manager.ReleaseEffect(Effect);
#endif
}

/// get maximum number of used parameters in any of the loaded effects
int System::GetMaxParameters()
{
	return m_iMaxParameters;
}

/// on effect load global callback function
void OnEffectLoad(Effect *p)
{
	g_System.OnEffectLoad(p);
	g_TechniqueManager.CalculateTechniqueHandles(p);
}

// on effect release callback function
void OnEffectRelease(Effect *p)
{
	g_System.OnEffectRelease(p);
}

} // namespace FX
