#ifndef __FX_CONTEXT_H__
#define __FX_CONTEXT_H__

#ifndef FX_ALLOW_INCLUDE
#error "Illegal include of FX library internals.h"
#endif

#include "fx/Shaders.h"
#include "fx/ImmediateParameter.h"
#include "fx/SamplerStates.h"
#include <Display/Device/DeviceContext.h>
#include "fx/Fx.h"
#include "fx/effect.h"
#include "fx/System.h"
#include "btl/allocators/stackallocator.h"

namespace Display
{
class C_Context;
}

namespace FX
{

struct Parameter;
class Effect;
class SharedEffect;
class BlendStateManager;
struct Pass;
struct FunctionBinding;
struct nglFXParam_Sampler;
struct SamplerState;
struct Function;
struct SharedParameter;

#define MAX_PIXEL_SAMPLERS 16
#define MAX_VERTEX_SAMPLERS 4

DECLARE_CONTEXT(Context, Context);

struct ImmediateParameterData
{
	S32 iHandle[2];
};

struct ALIGN16 Align16
{
	int iDummy1, iDummy2, iDummy3, iDummy4;
};

/// thread-safe rendering context
class Context : public Display::I_Context
{
	friend class I_Effect;
public:
	/// constructor
	Context();

	/// destructor
	~Context();

	/// one-time initialisation
	void Initialise();

	/// allocate a new instance of this class
	I_Context *New()
	{ 
#if !PLATFORM_SPU
		return new Context;
#endif
	}

	/// clone the state of another context into this context
	void CloneState(const I_Context *pContext);

	/// invalidate the cached render state
	void InvalidateState();

	/// sets the render flags
	void SetRenderFlags(const RenderFlags renderFlags) { m_RenderFlags = renderFlags; }

	/// gets render flags
	RenderFlags	GetRenderFlags() const { return m_RenderFlags; }

	/// set a parameter value during Begin-End pass
	void SetParameterImmediate(ImmediateParameter param, const void *pData);

	/// set a texture parameter value during Begin-End pass
	void SetTextureParameterImmediate(ImmediateParameter param, const void *pTexture, U32 iRenderState);

	/// set a parameter value during Begin-End pass with a specified size
	void SetParameterImmediateSize(ImmediateParameter param, const void *pData, S32 cSize);

	/// set a blend state block (index supplied by g_RenderStateMgr)
	void SetRenderState(U32 i);

	/// begin FX pass
	void BeginPass(Effect* Effect, U32 iTechnique);

	/// cleanup after BeginPass
	void EndPass();

	/// find a matching technique index on an effect for a given technique handle
	int TechniqueFromTechniqueHandle(Effect *pEffect, TechniqueHandle iTechnique);

#if !PLATFORM_PPU_SPU
	/// commit changed parameters during Begin-End pass
	void Commit() {}
#else
	/// commit changed parameters during Begin-End pass
	void Commit();
#endif

	/// returns the current value of the given shared parameter in this context
	void *GetSharedParameterValue(ParameterHandle &h);

#if !PLATFORM_SPU
	/// update Display sampler states using the cached sampler states on the context
	void CopyFXSamplerStatesToDisplay();
#endif
	/// update shared shader constants when either switching between effects or between renderlist nodes
	void BindSharedParameters(const SharedBindingBits *pUsage, const SharedBindingBits *pMask);

private:
	/// set bind sources to the given effect
	void SetSources(Effect *pEffect);

	/// set static sources
	void SetStaticSources();

	/// set texture and sampler states
	void SetSamplerState(S32 iHandle, U32 iSamplerState);

	/// set vertex texture and sampler states
	void SetVertexSamplerState(S32 iHandle, U32 iSamplerState);

	/// set texture and sampler states
	void SetTexture(S32 iHandle, nglFXParam_Sampler *param);

	/// set vertex texture and sampler states
	void SetVertexTexture(S32 iHandle, nglFXParam_Sampler *param);

	/// fetch texture
#if PLATFORM_SPU
	const void *FetchTexture(const void *pTexture);
#else
	const void *FetchTexture(const void *pTexture) { return pTexture; }
#endif

	/// prefetch all pass related data onto the SPU
	void Prefetch(Pass *pPass);

	/// clear previously assigned iHandle[2] on parameters that aren't used anymore
	void ClearParameterBindings(const Btl::Vector<FunctionBinding> *pBindings, int iShaderType);

	/// set parameters for the given pixel shader
	void BindPixelFunction(Effect* effect, Pass* pass, Function* function);

	/// set parameters for the given vertex shader
	void BindVertexFunction(Effect* effect, Pass* /*pass*/, Function* function);

	/// apply a single shared binding
	void ApplyBinding(SharedParameter *pBinding);

	/// templated function to apply bindings to either a vertex or pixel shader
	template<typename T, int dirty>
	void FXApplyBindings(Effect* effect, Btl::Vector<FunctionBinding> *pBindings, S32 iShaderType, S32 iOffset);

	/// patch pixel shader parameters
	void PatchPixelShaderConstants(Effect* effect, Btl::Vector<FunctionBinding> *pBindings, S32 iShaderType);

	/// get an EA pointer to a pass, given an EA pointer to an Effect and a technique index
	Pass *PassFromTechnique(Effect *pEffect, S32 iTechnique);

	// given a pointer to Btl::Vector, download the vector and it's data. the memory is allocated from m_Allocator
	template<typename T>
	void SpuFetch(Btl::Vector<T> *pBindings);

	// utility function for ApplyBindings
	void ApplyBindings(SharedParameter *pBinding, U32 bits);

	/// utility function for BindSharedParameters
	void ClearBindings(SharedParameter *pBinding, U32 bits);
private:
	Display::C_Context			*m_pDisplay;
	Display2::C_PixelShader		*m_pCurrentPixelShader;
	Display2::C_VertexShader	*m_pCurrentVertexShader;
	bool						m_bPatchPixelShaderOnCommit;
	bool						m_bChangedPixelShader;
	Effect						*m_pCurrentEffect;
	Pass						*m_pCurrentPass;
	S32							m_nSharedParams;
	S32							m_iSharedParamDataSize;
	int							m_iCurrentTechnique;
	// [Blur1PCDX11] The current pass binds through its native FX5 pass, which
	// rebinds every resource and sampler its shaders use.
	bool						m_bNativePass;

	SharedEffect				*m_pSharedEffect;
	BlendStateManager			*m_pRenderStateMgr;
	SamplerStateManager			*m_pSamplerStateMgr;
	
	RenderFlags					m_RenderFlags;

	Btl::Vector<FunctionBinding>*m_pPixelBindings, *m_pVertexBindings;
#if PLATFORM_PPU_SPU
	Btl::Vector<PatchBinding>	*m_pPixelPatchBindings;
#endif

	Btl::Vector<void*, Btl::VectorParameters_Array<16> >
								m_Sources;	// bind sources
	Btl::Vector<ImmediateParameterData>
								m_Imm;		// cache the constant register numbers for each shared parameter for the current effect

	Btl::Vector<U32, Btl::VectorParameters_Array<16> >
								m_pCurrentPixelSamplerState;
	Btl::Vector<U32, Btl::VectorParameters_Array<MAX_VERTEX_SAMPLERS> >
								m_pCurrentVertexSamplerState;

	// bit vector representing the currently set shared bindings
	SharedBindingBits ALIGN16	m_CurrentBindings;

	Align16 m_Align;

	Btl::Vector<F32> ALIGN16	m_SharedParameterData;

#if PLATFORM_PPU_SPU
	int iPad;

	Btl::Vector<F32, Btl::VectorParameters_Array<1024> > ALIGN16
								m_LocalParameterData;
#endif
#if PLATFORM_SPU
	Btl::StackAllocatorSimple<> m_Allocator[2];

	Display2::C_PixelShader		m_PixelShader;
	Display2::C_VertexShader	m_VertexShader;

	U8*							m_pPixelShaderUCode;
	void*						m_pLastTextureEA;
	void*						m_pLastTextureLS;
#endif

#define PIXELSHADER_CACHE_SIZE 32

#if PLATFORM_PPU_SPU
	U32 m_Ucode[PIXELSHADER_CACHE_SIZE];
	U32 m_Hash[PIXELSHADER_CACHE_SIZE];
	U32 m_Addr[PIXELSHADER_CACHE_SIZE];
	bool PSCacheLookup(U8 *pUcode);
	void PSCacheClear();
#endif
};

extern Context *g_pContext;

}

#endif
