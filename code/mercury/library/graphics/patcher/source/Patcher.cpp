#include "stdafx.h"
#include "Patcher/Patcher.h"
#include <System/Scheduler.h>

extern Scheduler::jqModule PatcherModule;

namespace Patcher
{

Debug::Logger& g_Logger = Debug::GetLogger("Graphics.Patcher");

C_GPUPatcher::C_GPUPatcher()
{
}

C_GPUPatcher::~C_GPUPatcher()
{
}

void C_GPUPatcher::SetParameter(const Display::C_PixelShaderParameter& psParameter, const F32* pData)
{
	Display::Device()->SetPixelShaderParameterF(psParameter, pData);
}

void C_GPUPatcher::SetParameterArray(const Display::C_PixelShaderParameterArray& psParameterArray, const F32* pData)
{
	Display::Device()->SetPixelShaderParameterArrayF(psParameterArray, pData);
}

void C_GPUPatcher::SetPixelShader(const Display::C_PixelShader* pPixelShader)
{
	Display::Device()->SetPixelShader(pPixelShader);
}

C_PPUPatcher::C_PPUPatcher()
{
	m_LinearBuffer0.Create(10 * 1024 * 1024, GCM_ALIGNMENT_PIXELSHADER, Display::E_AllocHeap_Video);
	m_LinearBuffer1.Create(10 * 1024 * 1024, GCM_ALIGNMENT_PIXELSHADER, Display::E_AllocHeap_Video);
	
	m_pLinearBuffer = &m_LinearBuffer0;

	m_MaximumBufferSize = 0;
}

C_PPUPatcher::~C_PPUPatcher()
{
	m_LinearBuffer1.Destroy();
	m_LinearBuffer0.Destroy();

	LOGDEBUG(g_Logger, "Maximum PPU buffer size = %d\n", m_MaximumBufferSize);
}

void C_PPUPatcher::SetParameter(const Display::C_PixelShaderParameter& psParameter, const F32* pData)
{
	if (m_ParameterCount == MAX_PARAMETERS)
		return;
	for (U32 instNo = 0; instNo < psParameter->m_Offsets.Size(); ++instNo)
	{
		const U32 paramNo = m_ParameterCount++;
		const F32 x = cellGcmSwap16Float32(pData[0]);
		const F32 y = cellGcmSwap16Float32(pData[1]);
		const F32 z = cellGcmSwap16Float32(pData[2]);
		const F32 w = cellGcmSwap16Float32(pData[3]);
		m_ParameterOffsets[paramNo]	= psParameter->m_Offsets[instNo];
		m_ParameterData[paramNo]	= MathsStorage::Float4(x, y, z, w);
		if (m_ParameterCount == MAX_PARAMETERS)
			break;
	}
}

void C_PPUPatcher::SetParameterArray(const Display::C_PixelShaderParameterArray& psParameterArray, const F32* pData)
{
	for (U32 paramNo = 0; paramNo < psParameterArray.Size(); ++paramNo)
	{
		SetParameter(psParameterArray[paramNo], &pData[paramNo*4]);
	}
}

void C_PPUPatcher::SetPixelShader(const Display::C_PixelShader* pPixelShader)
{
	if (!pPixelShader)
	{
		Display::Device()->SetPixelShader(0);
		return;
	}

	m_nPatches++;
	assert(m_ParameterCount <= MAX_PARAMETERS);

	U8* pMemory = (U8*)m_pLinearBuffer->Allocate(pPixelShader->GetSize(), GCM_ALIGNMENT_PIXELSHADER);
	if (!pMemory)
	{
		assert(0);
		Display::Device()->SetPixelShader(pPixelShader);
		return;
	}

	memcpy(pMemory, pPixelShader->GetAddress(), pPixelShader->GetSize());

	for (U32 paramNo = 0; paramNo < m_ParameterCount; ++paramNo)
	{
		U8* pData		= (U8*)&m_ParameterData[paramNo];
		U8* pAddress	= pMemory + m_ParameterOffsets[paramNo];
		memcpy(pAddress, pData, 16);
	}
					
	Display::Device()->SetPixelShader(pPixelShader, pMemory);

	m_ParameterCount = 0;
}

void C_PPUPatcher::Reset()
{
	m_MaximumBufferSize = Maths::Max(m_MaximumBufferSize, m_pLinearBuffer->Used());

	LOGDEBUG(g_Logger, "Number of patches done = %d\n", m_nPatches);
	LOGDEBUG(g_Logger, "Current PPU buffer size = %d\n", m_pLinearBuffer->Used());
	LOGDEBUG(g_Logger, "Maximum PPU buffer size = %d\n", m_MaximumBufferSize);

	m_nPatches = 0;

	if (m_pLinearBuffer == &m_LinearBuffer0)
		m_pLinearBuffer = &m_LinearBuffer1;
	else
		m_pLinearBuffer = &m_LinearBuffer0;
	m_pLinearBuffer->Free();
}

C_SPUPatcher::C_SPUPatcher()
{
	m_LinearBuffer0.Create(2 * 1024 * 1024, GCM_ALIGNMENT_PIXELSHADER, Display::E_AllocHeap_Video);
	m_LinearBuffer1.Create(2 * 1024 * 1024, GCM_ALIGNMENT_PIXELSHADER, Display::E_AllocHeap_Video);

	m_pLinearBuffer = &m_LinearBuffer0;

	m_hJob = Scheduler::CreateGroup();
}

C_SPUPatcher::~C_SPUPatcher()
{
	Scheduler::DestroyGroup(m_hJob);

	m_LinearBuffer1.Destroy();
	m_LinearBuffer0.Destroy();
}

void C_SPUPatcher::SetParameter(const Display::C_PixelShaderParameter& psParameter, const F32* pData)
{
	for (U32 instNo = 0; instNo < psParameter->m_Offsets.Size(); ++instNo)
	{
		const U32 paramNo = m_ParameterCount++;
		m_ParameterOffsets[paramNo]	= psParameter->m_Offsets[instNo];
		m_ParameterData[paramNo]	= MathsStorage::Float4(pData[0], pData[1], pData[2], pData[3]);
	}
}

void C_SPUPatcher::SetParameterArray(const Display::C_PixelShaderParameterArray& psParameterArray, const F32* pData)
{
	for (U32 paramNo = 0; paramNo < psParameterArray.Size(); ++paramNo)
	{
		SetParameter(psParameterArray[paramNo], &pData[paramNo*4]);
	}
}

void C_SPUPatcher::SetPixelShader(const Display::C_PixelShader* pPixelShader)
{
	U8* pMemory = (U8*)m_pLinearBuffer->Allocate(pPixelShader->GetSize(), GCM_ALIGNMENT_PIXELSHADER);
	if (!pMemory)
		return;

	Display::Device()->SetPixelShader(pPixelShader, pMemory);
	U32* blocker = Display::Device()->InsertBlocker(1);

	// launch the task, not finished yet

	m_ParameterCount = 0;
}

void C_SPUPatcher::Reset()
{
	if (m_pLinearBuffer == &m_LinearBuffer0)
		m_pLinearBuffer = &m_LinearBuffer1;
	else
		m_pLinearBuffer = &m_LinearBuffer0;
	m_pLinearBuffer->Free();
}

}; // namespace Patcher
