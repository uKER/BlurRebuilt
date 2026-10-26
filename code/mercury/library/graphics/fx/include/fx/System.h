#ifndef __FX_SYSTEM_H__
#define __FX_SYSTEM_H__

#ifndef FX_ALLOW_INCLUDE
#error "Illegal include of FX library internals.h"
#endif
#include <Display/context/SamplerUsageFlags.h>

#define FX_SYSTEM_MAX_LOCAL_PARAMETERS 128

namespace Patcher
{
	class C_PPUPatcher;
};

namespace FX
{

class System
{
	friend class SystemTarget;
	friend class Context;
public:
	/// Initialisation code
	void Initialise();

	/// Shutdown code
	void Shutdown();

	/// returns the NULL parameter which is returned for parameters that weren't found when calling GetParameter()
	Parameter *GetNullParameter();

	/// get pointer to default texture which is assigned instead of NULL textures
	gpuTexture *GetDefaultTexture();

#if PLATFORM_PPU_SPU
	/// get pointer to default texture which is assigned instead of NULL textures to vertex texture samplers on the PS3
	gpuTexture *GetDefaultFPTexture();
#endif
		/// set a parameter value
	void SetParameter(Parameter *pParam, const void *pData);

	/// Function that is to be called when a new effect is loaded
	void OnEffectLoad(Effect *Effect);

	/// Function that is to be called when an effect is deleted
	void OnEffectRelease(Effect *Effect);

	/// get maximum number of used parameters in any of the loaded effects
	int GetMaxParameters();

protected:
	/// for each function binding calculate the offset into the parameter data block
	void BindFunctionToParameters(Effect* effect, Function* function);

	/// delete all shared bindings off a particular pixel shader of vertex shader
	void DeleteSharedBindings(Effect *pEffect, Function *pFunc, bool bSamplersOnly);

	/// delete shared bindings off the effect, these will be set using the shared parameter system
	void BindFunctionToParameters(Effect *pEffect);

	/// for each function binding determine whether it is being set by the scene, or whether to use the effect defaults
	void DetermineBindingsExportFlag(Effect* effect, Function* function);

	/// set the flags for which samplers are used in the given bindings
	void DetermineSamplerUsageFlags(Display::C_SamplerUsageFlags *pFlags, const Btl::Vector<FunctionBinding> &bindings, int iShaderType);

	/// set the flags for which vertex input attributes are used
	void DetermineVertexInputAttributes(U32 *pFlags, Display2::C_VertexShader *pShader);

#ifndef CFG_SPEED
	/// dump shader bindings to debuglog on load
	void DumpBindings(Effect *pEffect, const Btl::Vector<FunctionBinding> &bindings);
	void DumpBindings(Effect *pEffect);
#endif

#if PLATFORM_PPU_SPU
	/// lookup pixel shader bindings
	void EvaluatePixelShaderBindings(Effect *pEffect, Function *pShader);
#endif

	int m_iMaxParameters;	// maximum number of parameters found on a single effect
};

extern Maths::HMat44 gFXIdentityMatrix;
extern System g_System;

} // namespace FX

#endif
