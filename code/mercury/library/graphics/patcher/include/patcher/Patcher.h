#ifndef PATCHER_H
#define PATCHER_H

#if _MSC_VER > 1000
#pragma once
#endif

#if !defined(PLATFORM_PC)

#include <Display/Memory/MemoryLinearBuffer.h>
#include <Patcher/Job.h>
#include <System/Scheduler/JobQueue.h>

#define ENABLE_PATCHER_STATS

namespace Patcher
{

class C_GPUPatcher
{
public:

	C_GPUPatcher();
	~C_GPUPatcher();

	void SetParameter(const Display::C_PixelShaderParameter& psParameter, const F32* pData);
	void SetParameterArray(const Display::C_PixelShaderParameterArray& psParameter, const F32* pData);
	void SetPixelShader(const Display::C_PixelShader* pPixelShader);

};

const MAX_PARAMETERS = 128;

class C_PPUPatcher
{
public:
	
	C_PPUPatcher();
	~C_PPUPatcher();

	void SetParameter(const Display::C_PixelShaderParameter& psParameter, const F32* pData);
	void SetParameterArray(const Display::C_PixelShaderParameterArray& psParameter, const F32* pData);
	void SetPixelShader(const Display::C_PixelShader* pPixelShader);

	void Reset();

private:

	MathsStorage::Float4		m_ParameterData[MAX_PARAMETERS];
	U32							m_ParameterOffsets[MAX_PARAMETERS];
	U32							m_ParameterCount;

	Display::C_LinearBuffer		m_LinearBuffer0;
	Display::C_LinearBuffer		m_LinearBuffer1;
	Display::C_LinearBuffer*	m_pLinearBuffer;

	U32							m_MaximumBufferSize;
	U32							m_nPatches;
};

class C_SPUPatcher
{
public:
	
	C_SPUPatcher();
	~C_SPUPatcher();

	void SetParameter(const Display::C_PixelShaderParameter& psParameter, const F32* pData);
	void SetParameterArray(const Display::C_PixelShaderParameterArray& psParameter, const F32* pData);
	void SetPixelShader(const Display::C_PixelShader* pPixelShader);

	void Reset();

private:

	MathsStorage::Float4		m_ParameterData[MAX_PARAMETERS];
	U32							m_ParameterOffsets[MAX_PARAMETERS];
	U32							m_ParameterCount;

	Display::C_LinearBuffer		m_LinearBuffer0;
	Display::C_LinearBuffer		m_LinearBuffer1;
	Display::C_LinearBuffer*	m_pLinearBuffer;

	Scheduler::GroupHandle		m_hJob;
	Scheduler::PolicyHandle		m_hPolicy;

private:

	enum { kMaximumJobs = 32 };

	C_SPUJob					m_JobInfo[kMaximumJobs];
	U32							m_JobIndex;

};

}; // namespace Patcher

#endif // PLATFORM_PC

#endif // PATCHER_H
