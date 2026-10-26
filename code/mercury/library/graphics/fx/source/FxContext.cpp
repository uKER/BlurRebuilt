#include "stdafx.h"
namespace FX { const void *GetDefaultVertexTextureFor(const void *pTexture); }	// [Blur2PC] System.cpp

#include <Display.h>

#include "fx/FxContext.h"
#include "fx/Effect.h"
#include "fx/System.h"
#include "fx/Shared.h"
#include "fx/RenderStates.h"
#include "display/context/Context.h"
#include "fx/SamplerStates.h"
#include "fx/FxStatistics.h"
#if ENABLE_DEBUGVARIABLES
#include "fx/DebugVariables.h"
#endif
#include "fx/ShaderImpl.h"
#include "fx/NativeFX11.h"
#include <Btl/Utility/hash.h>

namespace FX
{

DEFINE_CONTEXT(Context, Context);

#define XMemCpy memcpy

#if !PLATFORM_SPU
	Debug::Bool g_bUploadAllSharedBindings("Graphics.FX.UploadAllSharedBindings", false);
	Debug::Bool g_bOverrideSubtechniques("Graphics.Techniques.Override", false);
	Debug::Bool g_bPatchedShaderCaching("Graphics.FX.PatchedShaderCaching", true);
	extern int g_iOverrideTechnique;

	#if PLATFORM_PS3
	Debug::Bool g_bDisableShaderPatching("Graphics.FX.DisableFragmentPatching", false);
	#endif
#else
	#define g_bUploadAllSharedBindings 0
	#define g_bDisableShaderPatching 0
	#define g_bPatchedShaderCaching 1
#endif

DebugSamplerStates g_DebugSamplerStates;

static bool gbFirstAssert = true;

extern RenderFlags g_RenderFlags;


#if !PLATFORM_SPU

//// convert a normal FX::Parameter to an FX::ImmediateParameter
ImmediateParameter GetImmediateParameter(const I_Effect *pEffect, Parameter *pParam, U32 cMaxSize = UINT_MAX, const SharedBindingBits *pUsage = NULL);ImmediateParameter GetImmediateParameter(const I_Effect *_pEffect, const Parameter *pParam, U32 cMaxSize, const SharedBindingBits *pUsage)
{
	U32 iIdx = 0;
	U32 cSize = 0;
	U32 iImmOffset = FX_SYSTEM_MAX_LOCAL_PARAMETERS -1;
	U8 iResourceType = 0;
	U32 iOffset = 0xffffff;
	U8 iDoCopy = 0;

	if (pParam != g_System.GetNullParameter() && pParam->Data != NULL && pParam->Size != 0)
	{
		Effect *pEffect = (Effect*)_pEffect;
		if (pParam->bShared)
			pEffect = &g_SharedEffect;
		iIdx = (pParam - &pEffect->Parameters[0]);
		cSize = pParam->Size/4;
		iResourceType = (pParam->Type==NGLFX_PARAM_SAMPLER)?IMMEDIATEPARAM_TEXTURE:IMMEDIATEPARAM_CONSTANT;
		if (!pParam->bShared)
		{
			iImmOffset = iIdx;
		}
		else
		{
			// [Blur1PCDX11] D3D9's global c-registers retain streamed values even
			// when no shared parameter conflicts. Each FX11 effect has its own
			// reflected buffers; retain every shared immediate value so a later
			// BeginPass can seed those buffers instead of resetting them to zero.
			iDoCopy = 1;
			iImmOffset = iIdx + FX_SYSTEM_MAX_LOCAL_PARAMETERS;
			iOffset = (U32*)pParam->Data - &g_SharedEffect.ParameterData[0];
		}
	}
	else
	{
		LOGDEBUG( g_Log, "Warning: GetImmediateParameter failed for '%s'\n", pParam->Name.Chars());
	}

	cMaxSize = cMaxSize/(sizeof(F32)*4);
	if (cSize > cMaxSize)
		cSize = cMaxSize;

	BIZ_ASSERT(iImmOffset<0xfffff);
	BIZ_ASSERT(iIdx<=255);
	BIZ_ASSERT(cSize<=255);
	BIZ_ASSERT(iOffset<=0xffffff);

	return (((U64)iIdx)<<(24+32)) + (((U64)cSize)<<20) + ((U64)iImmOffset) + ((U64)iResourceType<<28) + ((U64)iDoCopy<<29) + ((U64)iOffset<<32);
}

/// returns the current value of the given shared parameter in this context
void *Context::GetSharedParameterValue(ParameterHandle &handle)
{
	if (handle == -1)
		return g_System.GetNullParameter()->Data;
	int iOffset = (U32*)g_SharedEffect.Parameters[handle].Data - &g_SharedEffect.ParameterData[0];
	return &m_SharedParameterData[iOffset];
}
#endif

#define UNPACKPARAM1 \
	const U32 iIdx = (param>>(24+32))&255; \
	const U32 iSize = (param>>20)&255; \
	const U32 iImmOffset = param&0xfffff; \
	const U64 iResourceType = param&0x10000000L; \
	const U64 iDoCopy = param&0x20000000L; \
	const U32 iOffset = (param>>32)&0xffffff; \
	const ImmediateParameterData * const pImm = &m_Imm[iImmOffset]; \
	const U32 iHandle1 = pImm->iHandle[0]; \
	const U32 iHandle2 = pImm->iHandle[1];

#ifndef PLATFORM_SPU
#define UNPACKPARAM UNPACKPARAM1 \
	const Parameter * const pParam = (iOffset==0xffffff&&m_pCurrentEffect)?&m_pCurrentEffect->Parameters[iIdx]:&m_pSharedEffect->Parameters[iIdx];
#else
#define UNPACKPARAM UNPACKPARAM1
#endif


/// constructor
Context::Context()
{
	m_bNativePass = false;
	m_iCurrentTechnique = -1;
	m_pPixelBindings = NULL;
	m_pVertexBindings = NULL;
#if PLATFORM_SPU
	m_Allocator[0].Initialise(8*1024);
	m_Allocator[1].Initialise(1024);
#endif
}

/// destructor
Context::~Context()
{
	//delete m_pDisplay;
}

/// one-time initialisation
void Context::Initialise()
{
	m_pDisplay = Display::GetContext(m_pParent);

	// dependant on Display
	m_Flags = m_Id | m_pDisplay->GetId();
}

/// find a matching technique index on an effect for a given technique handle
int Context::TechniqueFromTechniqueHandle(Effect *pEffect, TechniqueHandle iTechnique)
{
	Spu::MemBaseMarker marker;

#if !PLATFORM_SPU
	if (g_bOverrideSubtechniques)
		iTechnique = g_iOverrideTechnique;
#endif

	// no subtechniques are set - always use the first technique
	if (iTechnique == 0)
		return 0;

	Btl::Vector<Technique> *pTechniques = &pEffect->Techniques;
	Spu::FetchAlloc(pTechniques);
	Btl::SpuFetch(*pTechniques);

	// return the last technique that doesn't contain any subtechniques that we don't want
	int n = pTechniques->Size();
	for (int i=n-1; i>=0; i--)
	{
		if (!(pTechniques->At(i).hTechnique&(~iTechnique)))
			return i;
	}
	return 0;
}

/// clone the state of another context into this context
void Context::CloneState(const I_Context *_pContext)
{
	if (_pContext == NULL)
	{
#if PLATFORM_SPU
		BIZ_ASSERT(0);
#else
		m_pSharedEffect = &g_SharedEffect;
		m_pRenderStateMgr = &g_RenderStateMgr;
		m_pSamplerStateMgr = &g_SamplerStateMgr;

		m_nSharedParams = m_pSharedEffect->GetNumSharedParameters();
		m_iSharedParamDataSize = m_pSharedEffect->ParameterData.Size();

		m_Imm.Resize(FX_SYSTEM_MAX_LOCAL_PARAMETERS + m_nSharedParams);

		// copy current shared parameter values from the global state
		m_SharedParameterData.Resize(m_iSharedParamDataSize);
		if (m_iSharedParamDataSize>0)
			memcpy(&m_SharedParameterData[0], &m_pSharedEffect->ParameterData[0], sizeof(F32)*m_iSharedParamDataSize);

		m_RenderFlags = g_RenderFlags;
#endif
	}
	else
	{
		Context *pContext = (FX::Context*)_pContext;
		F32 *pSharedParamData;
		{
			Spu::MemBaseMarker marker;
			Spu::FetchAlloc(pContext);
			m_nSharedParams = pContext->m_nSharedParams;
			m_iSharedParamDataSize = tl_align(pContext->m_iSharedParamDataSize, 4);
			m_pSharedEffect = pContext->m_pSharedEffect;
			m_pRenderStateMgr = pContext->m_pRenderStateMgr;
			m_pSamplerStateMgr = pContext->m_pSamplerStateMgr;
			m_RenderFlags = pContext->m_RenderFlags;
			pSharedParamData = pContext->m_SharedParameterData.Size() > 0 ? 
				&pContext->m_SharedParameterData[0] : NULL;
		}

		if (m_Imm.Size() < FX_SYSTEM_MAX_LOCAL_PARAMETERS + m_nSharedParams)
			m_Imm.Resize(FX_SYSTEM_MAX_LOCAL_PARAMETERS + m_nSharedParams);

		m_SharedParameterData.Resize(m_iSharedParamDataSize);

		// copy current shared parameter values from the global state
		if (m_iSharedParamDataSize>0)
			Spu::Fetch(&m_SharedParameterData[0], pSharedParamData, sizeof(F32)*m_iSharedParamDataSize);

		SharedEffect::SpuFetch(m_pSharedEffect);
		BlendStateManager::SpuFetch(m_pRenderStateMgr);
		SamplerStateManager::SpuFetch(m_pSamplerStateMgr);
	}

#if PLATFORM_PPU_SPU
	for (int i=0; i<PIXELSHADER_CACHE_SIZE; i++)
		m_Ucode[i] = 0;
	m_bChangedPixelShader = false;
#endif

	InvalidateState();

	SetStaticSources();
}

/// invalidate the cached render state
void Context::InvalidateState()
{
	m_pCurrentEffect = NULL;
	m_pCurrentPass = NULL;
	m_pCurrentPixelShader = NULL;
	m_bPatchPixelShaderOnCommit = false;
#if PLATFORM_SPU
	m_pLastTextureEA = NULL;
	m_pLastTextureLS = NULL;
	m_pPixelShaderUCode = NULL;
#endif

	m_CurrentBindings.ClearBits();

	for (int i=0; i<MAX_PIXEL_SAMPLERS; i++)
		m_pCurrentPixelSamplerState[i] = -1;
	
	for (int i=0; i<MAX_VERTEX_SAMPLERS; i++)
		m_pCurrentVertexSamplerState[i] = -1;

	if (m_Imm.Size() > 0)
		memset(&m_Imm[0], -1, sizeof(ImmediateParameterData)*m_Imm.Size());
}

#if PLATFORM_SPU
const void *Context::FetchTexture(const void *pTexture)
{
	if (!pTexture || (U32)pTexture<(256*1024))
		return pTexture;
	U8 *pDest = (U8*)Spu::Alloc(tl_align(sizeof(CellGcmTexture),16));
	static U8 m_data[sizeof(CellGcmTexture)+32];
	U32 align = ((U32)pTexture)&0xf;
	void* addr = Spu::Fetch(m_data, (const void*)((U32)pTexture - align), tl_align(sizeof(CellGcmTexture)+align, 16));
	memcpy(pDest, (void*)(((U32)addr) + align), sizeof(CellGcmTexture));
	return pDest;
}
#endif

/// set a texture parameter value during Begin-End pass
void Context::SetTextureParameterImmediate(ImmediateParameter param, const void *pTexture, U32 iSamplerState)
{
	UNPACKPARAM

#if ENABLE_DEBUGVARIABLES
	if (_g_bEnableFXDebugVariables && pParam->bOverride)
		return;
#endif

	if (iHandle2 != (U32)-1)
	{
		FxStatistics::Instance().SetTexture();
	}
	if (iHandle1 != (U32)-1)
	{
		FxStatistics::Instance().SetTexture();
	}
	if (iDoCopy)
	{
		*((U32*)&m_SharedParameterData[iOffset]) = Core::PointerToSlot32(pTexture);	// [Blur2PC][x64] sampler slot
#ifndef PLATFORM_SPU
		FxStatistics::Instance().CopyParameter(pParam->Name);
#endif
	}
	if (m_pCurrentEffect && m_pCurrentPass)
		NativeFX11SetImmediate(m_pCurrentEffect, m_pCurrentPass, m_iCurrentTechnique,
			pParam->Name.Chars(), &pTexture, 0, true, iSamplerState);
}

/// set a parameter value during Begin-End pass
void Context::SetParameterImmediate(ImmediateParameter param, const void *pData)
{
	UNPACKPARAM
	nglFXParam_Sampler *pSampler;

#if ENABLE_DEBUGVARIABLES
	if (_g_bEnableFXDebugVariables && pParam->bOverride)
		return;
#endif

	if(iResourceType)
	{
		// texture
		BIZ_ASSERT(iOffset != 0xffffff);	// this function doesn't support setting non-shared texture parameters. use SetTextureParameterImmediate instead
		pSampler = (nglFXParam_Sampler*)&m_SharedParameterData[iOffset];
		pSampler->Texture = Core::PointerToSlot32(*(void* const*)pData);	// [Blur2PC][x64] pData points at a texture pointer
		if (m_pCurrentEffect && m_pCurrentPass)
			NativeFX11SetImmediate(m_pCurrentEffect, m_pCurrentPass, m_iCurrentTechnique,
				pParam->Name.Chars(), pData, 0, true, pSampler->iSamplerState);
		return;

#if !CFG_SPEED && !PLATFORM_SPU
		if (pSampler->Texture == NULL)
		{
			LOGWARN( g_Log, "Context::SetParameterImmediate setting NULL texture to %s on %s\n",
				pParam->Name.Chars(), m_pCurrentEffect?m_pCurrentEffect->Name.Chars():"<unknown>");
		}
#endif

		if (iHandle2 != (U32)-1)
		{
			m_pDisplay->SetTexture(iHandle2, FetchTexture(*(void**)pData));
			SetSamplerState(iHandle2, pSampler->iSamplerState);
			FxStatistics::Instance().SetTexture();
			BIZ_ASSERT(iHandle1 == (U32)-1);	// we don't support FX parameters that are used on the pixel and vertex shader at the same time
			return;
		}
		if (iHandle1 != (U32)-1)
		{
			m_pDisplay->SetVertexTexture(iHandle1, FetchTexture(GetDefaultVertexTextureFor(*(void**)pData)));	// [Blur2PC]
			SetVertexSamplerState(iHandle1, pSampler->iSamplerState);
			FxStatistics::Instance().SetTexture();
			BIZ_ASSERT(iHandle2 == (U32)-1);	// we don't support FX parameters that are used on the pixel and vertex shader at the same time
			return;
		}
	}
	else
	{
		// constant
		if (iHandle1 != (U32)-1)
		{
			m_pDisplay->SetVertexShaderConstantF(iHandle1, (F32*)pData, iSize);
			FxStatistics::Instance().SetVSConstant(iHandle1);
//			BIZ_ASSERT(iHandle2 == (U32)-1);	// we don't support FX parameters that are used on the pixel and vertex shader at the same time
//			return;							// forego this slight optimisation because g_mViewProj is sometimes needed in both PS and VS
		}
		if (iHandle2 != (U32)-1)
		{
#if PLATFORM_PPU_SPU
			m_bPatchPixelShaderOnCommit = true;
			if (iOffset != 0xffffff)
				memcpy(&m_SharedParameterData[iOffset], pData, iSize*16);
			else
				memcpy(&m_LocalParameterData[iHandle2], pData, iSize*16);
#else
			m_pDisplay->SetPixelShaderConstantF(iHandle2, (F32*)pData, iSize);
#endif
			FxStatistics::Instance().SetPSConstant(iHandle2);
//			BIZ_ASSERT(iHandle1 == (U32)-1);	// we don't support FX parameters that are used on the pixel and vertex shader at the same time
		}
		if (iDoCopy)
		{
#ifndef PLATFORM_SPU
			FxStatistics::Instance().CopyParameter(pParam->Name);
#endif
			XMemCpy(&m_SharedParameterData[iOffset], pData, iSize*16);
		}
		if (m_pCurrentEffect && m_pCurrentPass)
			NativeFX11SetImmediate(m_pCurrentEffect, m_pCurrentPass, m_iCurrentTechnique,
				pParam->Name.Chars(), pData, iSize * 16, false);
	}
}


#if PLATFORM_PPU_SPU
#if !PLATFORM_SPU
INLINE void memcpy128(void *__restrict _pDest, const void *__restrict _pSrc, U32 _cSize)
{
#if defined(VEC_LD) || defined(VEC_LDL) || defined(COMPILER_FENCE)
#error "VEC_LD, VEC_LDL, or COMPILER_FENCE already defined"
#endif

#define VEC_LD(_vec, _offset, _addr) _vec = __builtin_vec_ld(_offset, _addr)
#define VEC_ST(_vec, _offset, _addr) __builtin_vec_st(_vec, _offset, _addr)
#define COMPILER_FENCE asm("")

#if 0 // Set to 1 to hint at cache that the cache line is no longer needed - adds pipeline stalls
#define VEC_LDL(_vec, _offset, _addr) COMPILER_FENCE; _vec = __builtin_vec_ldl(_offset, _addr)
#else
#define VEC_LDL(_vec, _offset, _addr) _vec = __builtin_vec_ld(_offset, _addr)
#endif

	BIZ_ASSERT((((U32)_pDest)&127) == 0);
	BIZ_ASSERT((((U32)_pSrc)&127) == 0);
	BIZ_ASSERT((_cSize&127) == 0);

	// Use 64bit arithmetic to reduce clrldi instructions
	U64 pSrc = (U64)_pSrc;
	U64 pDest = (U64)_pDest;
	U64 cSize = (_cSize/128)-1;

	// Together with builtin functions these help force indexed loads and stores.
	// Annoyingly the compiler regenerates load and store indices despite having enough registers.
	const U64 r1=0x10, r2=0x20, r3=0x30, r4=0x40, r5=0x50, r6=0x60, r7=0x70;

	const qword* __restrict pSrc128 = (const qword*__restrict)pSrc;
	qword* __restrict pDest128 = (qword*__restrict )pDest;

	qword A, B, C, D, E, F, G, H;

	VEC_LD(A, 0x00, pSrc128);
	VEC_LD(B, r1, pSrc128);
	VEC_LD(C, r2, pSrc128);
	VEC_LD(D, r3, pSrc128);
	VEC_LD(E, r4, pSrc128);
	VEC_LD(F, r5, pSrc128);
	VEC_LD(G, r6, pSrc128);
	VEC_LDL(H, r7, pSrc128);
	
	while(cSize--)
	{
		pSrc += 8*16;
		pSrc128 = (const qword*__restrict)pSrc;
		__dcbt(pSrc128);
		
		VEC_ST(A, 0, pDest128);
		VEC_ST(B, r1, pDest128);
		VEC_ST(C, r2, pDest128);
		VEC_ST(D, r3, pDest128);
		VEC_ST(E, r4, pDest128);
		VEC_ST(F, r5, pDest128);
		VEC_ST(G, r6, pDest128);
		VEC_ST(H, r7, pDest128);
		
		COMPILER_FENCE; // for dcbt
		
		VEC_LD(A, 0x00, pSrc128);
		VEC_LD(B, r1, pSrc128);
		VEC_LD(C, r2, pSrc128);
		VEC_LD(D, r3, pSrc128);
		VEC_LD(E, r4, pSrc128);
		VEC_LD(F, r5, pSrc128);
		VEC_LD(G, r6, pSrc128);
		VEC_LDL(H, r7, pSrc128);
		
		pDest += 8*16;
		pDest128 = (qword*__restrict)pDest;
	}

	VEC_ST(A, 0, pDest128);
	VEC_ST(B, r1, pDest128);
	VEC_ST(C, r2, pDest128);
	VEC_ST(D, r3, pDest128);
	VEC_ST(E, r4, pDest128);
	VEC_ST(F, r5, pDest128);
	VEC_ST(G, r6, pDest128);
	VEC_ST(H, r7, pDest128);

#undef VEC_LD
#undef VEC_LDL
#undef VEC_ST
#undef COMPILER_FENCE
}

#else
INLINE void memcpy128(void *__restrict _pDest, const void *__restrict _pSrc, U32 cSize)
{
	BIZ_ASSERT((((U32)_pDest)&127) == 0);
	BIZ_ASSERT((((U32)_pSrc)&127) == 0);
	BIZ_ASSERT((cSize&127) == 0);

	const vector float * __restrict pSrc = (const vector float* __restrict)_pSrc;
	vector float * __restrict pDest = (vector float* __restrict)_pDest;
	cSize = cSize/128;
	for (int i=0; i<cSize-1; i++, pSrc+=8, pDest+=8)
	{
		DCBT(128, pSrc);
		const vector float A = pSrc[0];
		const vector float B = pSrc[1];
		const vector float C = pSrc[2];
		const vector float D = pSrc[3];
		const vector float E = pSrc[4];
		const vector float F = pSrc[5];
		const vector float G = pSrc[6];
		const vector float H = pSrc[7];
		pDest[0] = A;
		pDest[1] = B;
		pDest[2] = C;
		pDest[3] = D;
		pDest[4] = E;
		pDest[5] = F;
		pDest[6] = G;
		pDest[7] = H;
	}

	const vector float A = pSrc[0];
	const vector float B = pSrc[1];
	const vector float C = pSrc[2];
	const vector float D = pSrc[3];
	const vector float E = pSrc[4];
	const vector float F = pSrc[5];
	const vector float G = pSrc[6];
	const vector float H = pSrc[7];
	pDest[0] = A;
	pDest[1] = B;
	pDest[2] = C;
	pDest[3] = D;
	pDest[4] = E;
	pDest[5] = F;
	pDest[6] = G;
	pDest[7] = H;
}
#endif

// patch pixel shader parameters
void Context::PatchPixelShaderConstants(Effect* pEffect, Btl::Vector<FunctionBinding> *pBindings, int iShaderType)
{
	if (g_bDisableShaderPatching)
		return;

	U32 ucodeaddr = (U32)m_pCurrentPixelShader->GetAddress();
	U32 iSlot = (ucodeaddr >> 8) & (PIXELSHADER_CACHE_SIZE-1);
	
	U32 iHash = 0;
	U64 bCalcHash = g_bPatchedShaderCaching;
	
	if (g_bPatchedShaderCaching && m_bChangedPixelShader && m_Ucode[iSlot] == ucodeaddr)
	{
		S64 count = m_pPixelPatchBindings->Size();
#if !CFG_SPEED
		if (count > 0)
#endif
		{
			const PatchBinding *pBinding = &m_pPixelPatchBindings->At(0);
			for (; count--; pBinding++)
			{
				const F32* src = (const F32*)((size_t)m_Sources[pBinding->Source] + pBinding->Offset);
				iHash = Btl::HashRaw((const S8*)src, 16, iHash);
			}
		}

		if (m_Hash[iSlot] == iHash)
		{
			m_pDisplay->SetPixelShader(m_pCurrentPixelShader, m_Addr[iSlot]);
			return;
		}
		
		// Calculated hash, no need to recalculate it below.
		bCalcHash = 0;
	}
	m_Ucode[iSlot] = ucodeaddr;

	Spu::Wait(3);

	// allocate buffer space for the patched shader
	U32 cSize = m_pCurrentPixelShader->GetSize();
	U32 offset;
	U8 *pMemory = (U8*)m_pDisplay->Alloc(cSize, &offset);

#if PLATFORM_SPU
	// on the SPU we patch the local copy of the UCode and then DMA that down
	U8 *pUCode = m_pPixelShaderUCode;
#else
	// on the PPU we copy the UCode to the ringbuffer and then patch it in-place
	U8 *pUCode = pMemory;
	{
		void* pShaderAddr = (void*)m_pCurrentPixelShader->GetAddress();
		DCBT(0, pShaderAddr);
		memcpy128(pMemory, pShaderAddr, cSize);
	}
#endif

	// perform patching
	S64 count = m_pPixelPatchBindings->Size();
#if !CFG_SPEED
	if (count > 0)
#endif
	{
		const PatchBinding *pBinding = &m_pPixelPatchBindings->At(0);
	
		if (bCalcHash)
		{
			for (; count--; pBinding++)
			{
				const U32* src = (const U32*)((size_t)m_Sources[pBinding->Source] + pBinding->Offset);
				const U32 x = (src[0]<<16) | (src[0]>>16);
				const U32 y = (src[1]<<16) | (src[1]>>16);
				const U32 z = (src[2]<<16) | (src[2]>>16);
				const U32 w = (src[3]<<16) | (src[3]>>16);

				iHash = Btl::HashRaw((S8*)src, 16, iHash);

				U32* __restrict dst = (U32*__restrict)(pUCode+pBinding->PatchOffset);
				dst[0] = x; dst[1] = y; dst[2] = z; dst[3] = w;
			}
		}
		else
		{
			for (; count--; pBinding++)
			{
				const U32* __restrict src = (const U32*__restrict )((size_t)m_Sources[pBinding->Source] + pBinding->Offset);
				const U32 x = (src[0]<<16) | (src[0]>>16);
				const U32 y = (src[1]<<16) | (src[1]>>16);
				const U32 z = (src[2]<<16) | (src[2]>>16);
				const U32 w = (src[3]<<16) | (src[3]>>16);
				U32* __restrict dst = (U32*__restrict)(pUCode+pBinding->PatchOffset);
				dst[0] = x; dst[1] = y; dst[2] = z; dst[3] = w;
			}
		}
	}

	m_Addr[iSlot] = offset;
	m_Hash[iSlot] = iHash;

#if PLATFORM_SPU
	Spu::Store(pMemory, m_pPixelShaderUCode, cSize);
#endif

	m_pDisplay->SetPixelShader(m_pCurrentPixelShader, offset);
}

/// commit changed parameters during Begin-End pass
void Context::Commit()
{
#if CFG_SPEED
	if (m_bPatchPixelShaderOnCommit)
#else
	if (g_bUploadAllSharedBindings || m_bPatchPixelShaderOnCommit)
#endif
	{
		PatchPixelShaderConstants(m_pCurrentEffect, m_pPixelBindings, 0);
		m_bPatchPixelShaderOnCommit = false;
		m_bChangedPixelShader = false;
	}
}
#endif

/// set texture and sampler states
void Context::SetSamplerState(int iHandle, U32 iSamplerState)
{
	BIZ_ASSERT(iHandle >= 0 && iHandle < MAX_PIXEL_SAMPLERS);
	if (m_pCurrentPixelSamplerState[iHandle] == iSamplerState)
		return;
	m_pCurrentPixelSamplerState[iHandle] = iSamplerState;
	Display::C_SamplerState *pS = m_pSamplerStateMgr->GetSamplerState(iSamplerState);
	FxStatistics::Instance().ChangeSamplerState();

	#if !CFG_SPEED && !PLATFORM_SPU
	if( g_DebugSamplerStates.bEnable )
	{
		S32 iDebugStates = iHandle;
		Display::C_SamplerState tmp = *pS;
		pS = &tmp;

		if(g_DebugSamplerStates.bOverrideAllWithAlbedo)
		{
			iDebugStates = 0;
		}

		if( iDebugStates >=0 && iDebugStates < 4 )
		{
			DebugSamplerStates::DebugSamplerState& debugSamplerState = g_DebugSamplerStates.m_Sampler[iDebugStates];
			if(debugSamplerState.bEnable)
			{
				switch( debugSamplerState.m_FilterMode )
				{
					default:
					case Display::E_TextureFilter_None:
						// do nothing
					break;
					case Display::E_TextureFilter_Point:
						pS->SetTextureFilter(Display2::E_TextureFilter_Point, Display2::E_TextureFilter_Linear, Display2::E_TextureFilter_Linear);
					break;
					case Display::E_TextureFilter_Linear:
						pS->SetTextureFilter(Display2::E_TextureFilter_Linear, Display2::E_TextureFilter_Linear, Display2::E_TextureFilter_Linear);
					break;
					case Display::E_TextureFilter_Anisotropic:
						pS->SetTextureFilter(Display2::E_TextureFilter_Linear, Display2::E_TextureFilter_Anisotropic, Display2::E_TextureFilter_Linear);
					break;
				}

				if( debugSamplerState.m_MaxAniso > 0 )
				{
					pS->SetMaxAnisotropy(debugSamplerState.m_MaxAniso);
				}

				pS->SetTextureLodBias(debugSamplerState.m_MipLodBias);
			}
		}
	}
	#endif

	m_pDisplay->SetSamplerState(iHandle, pS);
}

/// set vertex texture and sampler states
void Context::SetVertexSamplerState(int iHandle, U32 iSamplerState)
{
	BIZ_ASSERT(iHandle >= 0 && iHandle < MAX_VERTEX_SAMPLERS);
	if (m_pCurrentVertexSamplerState[iHandle] == iSamplerState)
		return;
	m_pCurrentVertexSamplerState[iHandle] = iSamplerState;
	Display::C_SamplerState *pS = m_pSamplerStateMgr->GetSamplerState(iSamplerState);
	FxStatistics::Instance().ChangeSamplerState();

	m_pDisplay->SetVertexSamplerState(iHandle, pS);
}

/// set a blend state block (index supplied by g_RenderStateMgr)
void Context::SetRenderState(U32 i)
{
	BlendState *pState = m_pRenderStateMgr->GetRenderState(i);

	if (m_RenderFlags.setBlendState)
	{
		m_pDisplay->SetBlendState(pState->srcBlend, pState->destBlend, pState->blendOp, pState->srcBlendAlpha, pState->destBlendAlpha, pState->blendOpAlpha);
		m_pDisplay->SetAlphaTestState(pState->alphaTestEnable, pState->alphaRef);

		if (m_RenderFlags.setAlphaToMask)
			m_pDisplay->SetAlphaToMaskEnable(pState->alphaToMaskEnable);
		else
			m_pDisplay->SetAlphaToMaskEnable(Display::E_Bool_False);
	}
	if (m_RenderFlags.setCullMode)
		m_pDisplay->SetCullMode(pState->cullMode);
}

// [Blur2PC][x64] The sampler word is a Core::PointerToSlot32 slot, so these work at any word size.

/// set texture and sampler states
void Context::SetTexture(int iHandle, nglFXParam_Sampler *param)
{
	// [Blur1PCDX11] Legacy sampler registers; the native pass binds its own.
	if (m_bNativePass) return;
	m_pDisplay->SetTexture(iHandle, FetchTexture(Core::Slot32ToPointer(param->Texture)));
	SetSamplerState(iHandle, param->iSamplerState);
}

/// set vertex texture and sampler states
void Context::SetVertexTexture(int iHandle, nglFXParam_Sampler *param)
{
	if (m_bNativePass) return; // [Blur1PCDX11] see SetTexture
	m_pDisplay->SetVertexTexture(iHandle, FetchTexture(GetDefaultVertexTextureFor(Core::Slot32ToPointer(param->Texture))));	// [Blur2PC]
	SetVertexSamplerState(iHandle, param->iSamplerState);	
}

/// set static sources
void Context::SetStaticSources()
{
	m_Sources[NGLFX_BIND_PARAMETER] = NULL;

	if (m_iSharedParamDataSize > 0)
		m_Sources[NGLFX_BIND_SHARED] = &m_SharedParameterData[0];
	else
		m_Sources[NGLFX_BIND_SHARED] = NULL;
}

/// set bind sources to the given effect
void Context::SetSources(Effect *pEffect)
{
	Spu::MemBaseMarker marker;

#if PLATFORM_PPU_SPU
	Btl::Vector<U32> *pParamData = &pEffect->ParameterData;
	Spu::FetchAlloc(pParamData);
	int c = tl_align(pParamData->Size(), 4);
	BIZ_ASSERT(m_LocalParameterData.Size() >= c);
	Spu::FetchAsync(&m_LocalParameterData[0], &pParamData->At(0), c * sizeof(F32), 3);
	m_Sources[NGLFX_BIND_PARAMETER] = &m_LocalParameterData[0];
#else
	m_Sources[NGLFX_BIND_PARAMETER] = &pEffect->ParameterData[0];
#endif
}

struct SetVertexConstant
{
	static inline void SetConstant(Display::C_Context *pContext, int idx, const F32* data, U32 nelements)
	{
		pContext->SetVertexShaderConstantF(idx, data, nelements);
	}
};

struct SetPixelConstant
{
	static inline void SetConstant(Display::C_Context *pContext, int idx, const F32* data, U32 nelements)
	{
#if !PLATFORM_PPU_SPU
		pContext->SetPixelShaderConstantF(idx, data, nelements);
#endif
	}
};

void Context::ApplyBinding(SharedParameter *pBinding)
{
	ImmediateParameterData *pImm = &m_Imm[FX_SYSTEM_MAX_LOCAL_PARAMETERS + pBinding->Idx];

	E_SPGResource type = (E_SPGResource)pBinding->Type;
	int handle = pBinding->Handle;
	Effect *pEffect = m_pSharedEffect;
	Parameter *pParam = &pEffect->Parameters[pBinding->Idx];
	BIZ_ASSERT(pParam->bShared);

	int iShaderType = 1^(type&1);
	if (type >= E_SPGResource_Texture)
	{
		nglFXParam_Sampler *param = (nglFXParam_Sampler*)((size_t)m_Sources[NGLFX_BIND_SHARED] + pBinding->Offset);

		if (param->Texture == NULL)
			return;

#if !CFG_SPEED && !PLATFORM_SPU
		if (param->Texture == NULL && gbFirstAssert)
		{
			LOGDEBUG( g_Log, "Context::ApplyBinding settings NULL texture to %s on %s\n",
				pParam->Name.Chars(), pEffect->Name.Chars());
			if (gbFirstAssert)
			{
				gbFirstAssert = false;
				BIZ_ASSERT_FAIL(0);	// Check you've set all of the texture parameters you exposed on your model
			}
		}
#endif
		// [Blur1PCDX11] Shared groups can change after BeginPass. FX9 handles
		// are not the registers of the recompiled FX11 pass; update by parameter name.
		if (m_pCurrentEffect && m_pCurrentPass)
		{
			void *pParamTexture = Core::Slot32ToPointer(param->Texture);	// [Blur2PC][x64] native binding takes a pointer
			NativeFX11SetImmediate(m_pCurrentEffect, m_pCurrentPass, m_iCurrentTechnique,
				pParam->Name.Chars(), &pParamTexture, 0, true, param->iSamplerState);
		}

		pImm->iHandle[iShaderType] = handle;
	}
	else
	{
		F32* src = (F32*)((size_t)m_Sources[NGLFX_BIND_SHARED] + pBinding->Offset);
		if (m_pCurrentEffect && m_pCurrentPass)
			NativeFX11SetImmediate(m_pCurrentEffect, m_pCurrentPass, m_iCurrentTechnique,
				pParam->Name.Chars(), src, pBinding->Size * 16U, false);

		pImm->iHandle[iShaderType] = handle;
	}

	FxStatistics::Instance().ApplyBinding(pParam->Name.Chars());
}

/// templated function to apply bindings to either a vertex or pixel shader
template<typename T, int dirty>
void Context::FXApplyBindings(Effect* effect, Btl::Vector<FunctionBinding> *pBindings, int iShaderType, int iOffset=0)
{
	if (!pBindings->Size())
		return;

	FunctionBinding* binding = &pBindings->At(0);
	FunctionBinding* binding_end = binding + pBindings->Size();
	binding += iOffset;
	while (binding < binding_end)
	{
		Parameter *p = NULL;
		ImmediateParameterData *pImm;
		if (!binding->Shared)
		{
			pImm = &m_Imm[binding->ParameterIdx];
		}
		else
		{
#if !PLATFORM_PPU_SPU
			BIZ_ASSERT(0); // here when applying a shared pixel shader parameter on the actual shader (PS3 only)
#endif
			pImm = &m_Imm[FX_SYSTEM_MAX_LOCAL_PARAMETERS + binding->ParameterIdx];
		}

		Effect *pEffect = binding->Shared?m_pSharedEffect:effect;
#if !CFG_SPEED && !PLATFORM_SPU
		Parameter *pParam = &pEffect->Parameters[binding->ParameterIdx];
#endif
		if (binding->Type == NGLFX_BIND_SAMPLER)
		{
			int handle = binding->Handle;
			nglFXParam_Sampler *param = (nglFXParam_Sampler*)((size_t)m_Sources[binding->Source] + binding->Offset);

#if !CFG_SPEED && !PLATFORM_SPU
			if (param->Texture == NULL && gbFirstAssert)
			{
				LOGDEBUG( g_Log, "Context::FXApplyBindings settings NULL texture to %s on %s\n",
					pParam->Name.Chars(), effect->Name.Chars());
				if (gbFirstAssert)
				{
					gbFirstAssert = false;
					BIZ_ASSERT(0);	// Check you've set all of the texture parameters you exposed on your model
				}
			}
#endif

#if ENABLE_DEBUGVARIABLES
			if ((_g_bEnableFXDebugVariables && pParam->bOverride) || !binding->Exported)
#else
			if (!binding->Exported)
#endif
			{
				if (iShaderType == 1)
				{
					// Set texture.
					SetTexture(handle, param);
				}
				else
				{
					// Set vertex texture
					SetVertexTexture(handle, param);
				}
			}
	
			pImm->iHandle[iShaderType] = handle;
		}
		else
		{
			F32* src = (F32*)((size_t)m_Sources[binding->Source] + binding->Offset);

#if PLATFORM_PPU_SPU
			if (iShaderType == 1)
			{
				pImm->iHandle[iShaderType] = binding->Offset/sizeof(F32);
			}
			else
#endif
			{
#if ENABLE_DEBUGVARIABLES
				if ((_g_bEnableFXDebugVariables && pParam->bOverride) || !binding->Exported)
#else
				if (!binding->Exported)
#endif
					T::SetConstant(m_pDisplay, binding->Handle, src, binding->Count);
				pImm->iHandle[iShaderType] = binding->Handle;
			}

#if !CFG_SPEED && !PLATFORM_SPU
			FxStatistics::Instance().ApplyBinding(pParam->Name.Chars());
#endif
		}
		binding++;
	}
}

// given a pointer to Btl::Vector, download the vector and it's data. the memory is allocated from m_Allocator
template<typename T>
void Context::SpuFetch(Btl::Vector<T> *pBindings)
{
#if PLATFORM_SPU
	int c = tl_align(pBindings->Size()*sizeof(T), 16);
	U8 *p = m_Allocator[0].Allocate<U8>(c);
	assert(p);
	Spu::FetchAsync(p, pBindings->First(), c, 3);
	S32 offset = p - (U8*)pBindings->First();
	pBindings->OffsetLocation(offset);
#endif
}

/// prefetch all pass related data onto the SPU
void Context::Prefetch(Pass *pPass)
{
	m_pPixelBindings = &pPass->PixelShader.Bindings;
#if PLATFORM_PPU_SPU
	m_pPixelPatchBindings = &pPass->PixelShader.PatchBindings;
#endif
	SpuFetch(m_pPixelBindings);
#if PLATFORM_PPU_SPU
	SpuFetch(m_pPixelPatchBindings);
#endif
	m_pVertexBindings = &pPass->VertexShader.Bindings;
	SpuFetch(m_pVertexBindings);

	Display2::C_PixelShader *pP = (Display2::C_PixelShader*)(PixelShader*)pPass->PixelShader.pShader;
	Display2::C_VertexShader *pV = (Display2::C_VertexShader*)(VertexShader*)pPass->VertexShader.pShader;
#if PLATFORM_SPU
	Spu::FetchAsync(&m_PixelShader, pP, sizeof(*pP), 3);
	m_pCurrentPixelShader = &m_PixelShader;
	Spu::FetchAsync(&m_VertexShader, pV, sizeof(*pV), 3);
	m_pCurrentVertexShader = &m_VertexShader;
#else
	m_pCurrentPixelShader = pP;
	m_pCurrentVertexShader = pV;
#endif
}

/// set parameters for the given vertex shader
void Context::BindVertexFunction(Effect* effect, Pass* /*pass*/, Function* function)
{
	m_pDisplay->SetVertexShader(m_pCurrentVertexShader);
	FXApplyBindings<SetVertexConstant,3>(effect, m_pVertexBindings, 0);
}

/// set parameters for the given pixel shader
void Context::BindPixelFunction(Effect* effect, Pass* pass, Function* function)
{
	// [Blur1PCDX11] The native pass sets the pixel shader at the next draw.
	if (!m_bNativePass)
#if !PLATFORM_PPU_SPU
	m_pDisplay->SetPixelShader(m_pCurrentPixelShader);
#endif
#if PLATFORM_SPU
	m_pPixelShaderUCode = m_Allocator[0].Allocate<U8>(tl_align(m_PixelShader.GetSize(),16));
	assert(m_pPixelShaderUCode);
	Spu::FetchAsync(m_pPixelShaderUCode, m_PixelShader.GetAddress(), tl_align(m_PixelShader.GetSize(),16), 3);
#endif
#if PLATFORM_PPU_SPU
	m_bPatchPixelShaderOnCommit = true;
	m_bChangedPixelShader = true;
#endif
	FXApplyBindings<SetPixelConstant,3>(effect, m_pPixelBindings, 1);
}

/// clear previously assigned iHandle[2] on parameters that aren't used anymore
/// on PS3 the value in the handle for pixel shader constants is meaningless, but something other than -1 means the value is used
void Context::ClearParameterBindings(const Btl::Vector<FunctionBinding> *pBindings, int iShaderType)
{
	for (int i=0; i<pBindings->Size(); i++)
	{
		const FunctionBinding *pBinding = &pBindings->At(i);
		ImmediateParameterData *pImm = &m_Imm[pBinding->ParameterIdx];
		if (!pBinding->Shared)
		{
			pImm = &m_Imm[pBinding->ParameterIdx];
		}
		else
		{
#if !PLATFORM_PPU_SPU
			BIZ_ASSERT(0); // here when applying a shared pixel shader parameter on the actual shader (PS3 only)
#else
			BIZ_ASSERT(iShaderType == 1);
#endif
			pImm = &m_Imm[FX_SYSTEM_MAX_LOCAL_PARAMETERS + pBinding->ParameterIdx];
		}
		pImm->iHandle[iShaderType] = -1;
	}
}

inline U32 Contains(const SharedBindingBits &a, const SharedBindingBits &b)
{
	U32 iResult = 0;
	U32 n = a.SizeInBytes()/4;
	U32 *pA = (U32*)a.Data();
	U32 *pB = (U32*)b.Data();
	for (U32 i=0; i<n; i++, pA++, pB++)
		iResult |= ((*pA & *pB) ^ *pB);

	return iResult;
}

inline bool operator &(const SharedBindingBits &a, const SharedBindingBits &b)
{
	U32 iResult = 0;
	U32 n = a.SizeInBytes()/4;
	U32 *pA = (U32*)a.Data();
	U32 *pB = (U32*)b.Data();
	for (U32 i=0; i<n; i++, pA++, pB++)
		iResult |= (*pA & *pB);

	return iResult!=0;
}

inline void Merge(SharedBindingBits &a, const SharedBindingBits &b)
{
	U32 n = a.SizeInBytes()/4;
	U32 *pA = (U32*)a.Data();
	U32 *pB = (U32*)b.Data();
	for (U32 i=0; i<n; i++, pA++, pB++)
		*pA |= *pB;
}

inline void Mask(SharedBindingBits &a, const SharedBindingBits &b)
{
	U32 n = a.SizeInBytes()/4;
	U32 *pA = (U32*)a.Data();
	U32 *pB = (U32*)b.Data();
	for (U32 i=0; i<n; i++, pA++, pB++)
		*pA &= ~*pB;
}

// utility function for BindSharedParameters
void Context::ApplyBindings(SharedParameter *pBinding, U32 bits)
{
	for (int i=0; i<4; i++)
	{
		if (bits & 0xff)
		{
			if (bits&1)
				ApplyBinding(pBinding+0);
			if (bits&2)
				ApplyBinding(pBinding+1);
			if (bits&4)
				ApplyBinding(pBinding+2);
			if (bits&8)
				ApplyBinding(pBinding+3);
			if (bits&16)
				ApplyBinding(pBinding+4);
			if (bits&32)
				ApplyBinding(pBinding+5);
			if (bits&64)
				ApplyBinding(pBinding+6);
			if (bits&128)
				ApplyBinding(pBinding+7);
		}
		bits>>=8;
		pBinding+=8;
	}
}

/// utility function for BindSharedParameters
void Context::ClearBindings(SharedParameter *pBinding, U32 bits)
{
	for (int i=0; i<4; i++)
	{
		if (bits & 0xff)
		{
			if (bits&1)
				m_Imm[FX_SYSTEM_MAX_LOCAL_PARAMETERS + pBinding[0].Idx].iHandle[1^(pBinding[0].Type&1)] = -1;
			if (bits&2)
				m_Imm[FX_SYSTEM_MAX_LOCAL_PARAMETERS + pBinding[1].Idx].iHandle[1^(pBinding[1].Type&1)] = -1;
			if (bits&4)
				m_Imm[FX_SYSTEM_MAX_LOCAL_PARAMETERS + pBinding[2].Idx].iHandle[1^(pBinding[2].Type&1)] = -1;
			if (bits&8)
				m_Imm[FX_SYSTEM_MAX_LOCAL_PARAMETERS + pBinding[3].Idx].iHandle[1^(pBinding[3].Type&1)] = -1;
			if (bits&16)
				m_Imm[FX_SYSTEM_MAX_LOCAL_PARAMETERS + pBinding[4].Idx].iHandle[1^(pBinding[4].Type&1)] = -1;
			if (bits&32)
				m_Imm[FX_SYSTEM_MAX_LOCAL_PARAMETERS + pBinding[5].Idx].iHandle[1^(pBinding[5].Type&1)] = -1;
			if (bits&64)
				m_Imm[FX_SYSTEM_MAX_LOCAL_PARAMETERS + pBinding[6].Idx].iHandle[1^(pBinding[6].Type&1)] = -1;
			if (bits&128)
				m_Imm[FX_SYSTEM_MAX_LOCAL_PARAMETERS + pBinding[7].Idx].iHandle[1^(pBinding[7].Type&1)] = -1;
		}
		bits>>=8;
		pBinding+=8;
	}
}

/// when switching between effects, this function updates shared shader constants
void Context::BindSharedParameters(const SharedBindingBits *pUsageVec, const SharedBindingBits *pMaskVec)
{
#ifndef CFG_SPEED
	if (g_bUploadAllSharedBindings)
	{
		int n = m_pSharedEffect->GetNumSharedBindings();
		for (int i=0; i<n; i++)
		{
			if ((*pUsageVec)[i])
			{
				ApplyBinding(m_pSharedEffect->GetSharedBinding(i));
			}
		}
		Merge(m_CurrentBindings,*pUsageVec);
	}
	else
#endif
	{
		U32 *pCurrent = (U32*)m_CurrentBindings.Data();
		U32 *pUsage = (U32*)pUsageVec->Data();
		U32 *pMask = (U32*)pMaskVec->Data();
		U32 currentA = pCurrent[0];
		U32 currentB = pCurrent[1];
		U32 currentC = pCurrent[2];
		U32 currentD = pCurrent[3];

		SharedParameter *pBinding = m_pSharedEffect->GetSharedBinding(0);
		U32 partA = (currentA & pUsage[0]) ^ pUsage[0];
		U32 partB = (currentB & pUsage[1]) ^ pUsage[1];
		U32 partC = (currentC & pUsage[2]) ^ pUsage[2];
		U32 partD = (currentD & pUsage[3]) ^ pUsage[3];

		U32 maskA = currentA & pMask[0];
		U32 maskB = currentB & pMask[1];
		U32 maskC = currentC & pMask[2];
		U32 maskD = currentD & pMask[3];

		if (maskA)
		{
			ClearBindings(pBinding, maskA);
			currentA &= ~maskA;
		}
		if (partA)
		{
			ApplyBindings(pBinding, partA);
			currentA |= partA;
		}
		if (maskB)
		{
			ClearBindings(pBinding+32, maskB);
			currentB &= ~maskB;
		}
		if (partB)
		{
			ApplyBindings(pBinding+32, partB);
			currentB |= partB;
		}
		if (maskC)
		{
			ClearBindings(pBinding+64, maskC);
			currentC &= ~maskC;
		}
		if (partC)
		{
			ApplyBindings(pBinding+64, partC);
			currentC |= partC;
		}
		if (maskD)
		{
			ClearBindings(pBinding+96, maskD);
			currentD &= ~maskD;
		}
		if (partD)
		{
			ApplyBindings(pBinding+96, partD);
			currentD |= partD;
		}

		pCurrent[0] = currentA;
		pCurrent[1] = currentB;
		pCurrent[2] = currentC;
		pCurrent[3] = currentD;
	}

	// when this function is called from the renderingdata, it can potentially clear bindings which are in use by the currently set effect
	// this force the effect to be re-applied.

	// ideally we eventually want to not un-set the bindings that are in use by the currently set effect
	// this is a temporary fix for Blur
}

/// get an EA pointer to a pass, given an EA pointer to an Effect and a technique and pass index
Pass *Context::PassFromTechnique(Effect *pEffect, int iTechnique)
{
	Spu::MemBaseMarker marker;

	Btl::Vector<Technique> *pTVector = &pEffect->Techniques;
	Spu::FetchAlloc(pTVector);
	Technique *pTechnique = &pTVector->At(iTechnique);
	Btl::Vector<Pass> *pPVector = &pTechnique->Passes;
	Spu::FetchAlloc(pPVector);
	return &pPVector->At(0);
}

#define ENABLE_FX_PASS_PROFILING 0

/// begin FX pass
void Context::BeginPass(Effect* pEffect, U32 iTechnique)
{
	Spu::MemBaseMarker marker;

#if ENABLE_FX_PASS_PROFILING
	PERF_EVENT("SceneGraph.FX.BeginPass");
#endif

	Pass *pPass;
	{
#if ENABLE_FX_PASS_PROFILING
	PERF_EVENT("SceneGraph.FX.PassFromTechnique");
#endif
	pPass = PassFromTechnique(pEffect, iTechnique);
	}

	if (m_pCurrentEffect == pEffect && m_pCurrentPass == pPass && m_iCurrentTechnique != -1)
		return;

	if (m_pCurrentEffect)
	{
		ClearParameterBindings(m_pPixelBindings, 1);
		ClearParameterBindings(m_pVertexBindings, 0);
	}

	m_pCurrentPass = pPass;
	m_bNativePass = NativeFX11HasPass(pEffect, pPass, iTechnique);

#if PLATFORM_SPU
	m_Allocator[0].Reset();
	Pass *_pPass = m_Allocator[0].Allocate<Pass>(1);
	assert(_pPass);
	{
#if ENABLE_FX_PASS_PROFILING
		PERF_EVENT("SceneGraph.FX.FetchPass");
#endif
		Spu::Fetch(_pPass, pPass, sizeof(*pPass));
	}
	pPass = _pPass;
#endif
	Prefetch(pPass);
	m_pDisplay->SetSamplerUsageFlags(&pPass->m_SamplerUsageFlags);
#if PLATFORM_PPU_SPU
	m_pDisplay->SetVtxAttrUsageFlags(pPass->m_VtxAttrUsageFlags);
#endif

	FxStatistics::Instance().ChangeShader();

	{
#if ENABLE_FX_PASS_PROFILING
	PERF_EVENT("SceneGraph.FX.SetSources");
#endif
	SetSources(pEffect);
	}

	{
#if ENABLE_FX_PASS_PROFILING
	PERF_EVENT("SceneGraph.FX.BindShared");
#endif
	BindSharedParameters(&pPass->SharedBindingUsage, &pPass->SharedBindingMask);
	}

#if PLATFORM_SPU
	{
#if ENABLE_FX_PASS_PROFILING
	PERF_EVENT("SceneGraph.FX.Wait");
#endif
	Spu::Wait(3);
	}
#endif

	{
#if ENABLE_FX_PASS_PROFILING
	PERF_EVENT("SceneGraph.FX.Bind");
#endif
	if (m_RenderFlags.setPixelShader)
		BindPixelFunction(pEffect, pPass, &pPass->PixelShader);
	BindVertexFunction(pEffect, pPass, &pPass->VertexShader);
	}

	m_pCurrentEffect = pEffect;
	m_iCurrentTechnique = iTechnique;
	// [Blur1PCDX11] FXB/render stages own state; FX5 binds native shaders/resources.
	NativeFX11Apply(pEffect, pPass, iTechnique, &m_Sources[0]);
}

/// cleanup after BeginPass
void Context::EndPass()
{
	Spu::Wait(3);
}

} // namespace FX
