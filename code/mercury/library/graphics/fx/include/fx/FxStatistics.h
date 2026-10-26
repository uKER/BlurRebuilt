#ifndef __FX_STATISTICS_H__
#define __FX_STATISTICS_H__

#if !CFG_SPEED && !PLATFORM_SPU
#define ENABLE_FX_STATISTICS 1
#endif

#if ENABLE_FX_STATISTICS
#include <core/threads/mutex.h>
#include <btl/containers/map.h>
#include <btl/utility/string.h>
#endif

namespace Debug
{
class Log;
}

namespace FX
{

/// class for tracking FX system statistics (for debugging purposes)
class FxStatistics
{
public:
#if PLATFORM_SPU
	static FxStatistics& Instance() {return *(FxStatistics*)NULL;}
#else
	static FxStatistics& Instance() {return s_instance;}
#endif

	void BeginFrame();					// called by renderingdata
	void EndFrame(float fnPrimitives);	// called by renderingdata
	void Dump();

#if ENABLE_FX_STATISTICS
	void ApplyBinding(const char *);
	void SetVSConstant(int iHandle);
	void SetPSConstant(int iHandle);
	void SetTexture();
	void ChangeSamplerState();
	void ChangeShader();
	void CopyParameter(const char *pszName);
#else
	void ApplyBinding(const char *) {}
	void SetVSConstant(int iHandle) {}
	void SetPSConstant(int iHandle) {}
	void SetTexture() {}
	void ChangeSamplerState() {}
	void ChangeShader() {}
	void CopyParameter(const char *pszName) {}
#endif

private:
#if ENABLE_FX_STATISTICS
	struct Counters
	{
		S32	nBindingsApplied;
		S32 nParametersCopied;
		S32 nConstantsSet;
		S32 nTexturesSet;
		S32 nSamplerStateChanges;
		S32 nShaderChanges;
	};

	struct Count
	{
		Count() : count(0) {}
		void operator ++(int) {count++;}
		S32 count;
	};

	typedef Btl::Map<U32, Count> DataMap;

	struct CountUnique
	{
		Count count;
		DataMap dataMap;
	};

	Counters m_frame;
	typedef Btl::Map<Btl::String, CountUnique> NameMap;
	NameMap m_bindings;
	NameMap m_copies; // copies made of shared parameters

	MT::Lock lock;
#endif

	FxStatistics();
	static FxStatistics s_instance;
};

}

#endif
