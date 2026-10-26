#include "stdafx.h"
#include "Render/Postprocess/Colour/LookupTable.h"
#include "Render/Postprocess/Colour/Task.h"
#include "Render/Postprocess/Manager.h"
#include "Render/Shaders/Constants.h"

EXTERN_SHADER(psLookupTable);

extern Scheduler::jqModule ColourLookupTableModule;

namespace PostProcess
{

Debug::Bool g_debugLookupTable("Graphics.Render.PostProcess.LookupTable", true);

C_LookupTable::C_LookupTable( C_Manager& manager ) : I_PostProcess( manager )
{
	m_TableIndex = 0;
	m_Size = 16;
	m_GroupHandle = Scheduler::JQ_GROUP_NONE;
}

C_LookupTable::~C_LookupTable()
{
}

bool C_LookupTable::Create()
{
	if (!Display::CreatePixelShaderFromFunc("Shader/psLookupTable", 0, &m_PixelShader, REF_SHADER(psLookupTable)))
		return false;

	m_PixelShaderParameter = m_PixelShader.GetParameter(CP_RENDER_SETTINGS, "g_vSettings");

	SetShader(&m_PixelShader);

	for (U32 i = 0; i < 3; ++i)
	{
		if (!Display::Device()->CreateTexture3D(m_Size, m_Size, m_Size, 1, Display::E_Format_Linear, Display::E_Format_A8R8G8B8, &m_LookupTable[i]))
			return false;

		Display::C_LockedBox lockedBox = m_LookupTable[i].LockBox3D(0);
		if (lockedBox.pBits)
		{
			U32* pSlice = (U32*)lockedBox.pBits;
			for (U32 b = 0; b < m_Size; ++b)
			{
				U32* pRow = pSlice;
				for (U32 g = 0; g < m_Size; ++g)
				{
					for (U32 r = 0; r < m_Size; ++r)
					{
						const F32 fScale = 255.0f / (F32) (m_Size - 1);
						const U32 red = (U32)(r * fScale);
						const U32 green = (U32)(g * fScale);
						const U32 blue = (U32)(b * fScale);
						pRow[r] = Maths::RGBA(red, green, blue, 255);
					}
					pRow += lockedBox.rowPitch / 4;
				}
				pSlice += lockedBox.slicePitch / 4;
			}

			m_LookupTable[i].Unlock(0);
		}
	}

	m_GroupHandle = Scheduler::CreateGroup();

	return true;
}

void C_LookupTable::Destroy()
{
	Scheduler::DestroyGroup(m_GroupHandle);

	for (U32 i = 0; i < 3; ++i)
	{
		Display::Device()->Destroy(&m_LookupTable[i]);
	}
	Display::Device()->Destroy(&m_PixelShader);
}

void C_LookupTable::Display() const
{
	if (g_debugLookupTable && m_Manager.Enabled())
	{
		PERF_EVENT("LookupTable");

		// get the current contents of render target and set as a texture
		SetViewport();
		Display::Device()->SetTexture(0, m_Manager.GetChainTexture());
		Display::Device()->SetTexture(1, &m_LookupTable[m_TableIndex]);
		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
		
		// set the shader parameters
		const F32 fSize		= 1.0f / (F32) m_Size;
		const F32 fScale	= 1.0f - fSize;
		const F32 fBias		= 0.5f * fSize;
		const Display::C_Float4 psSettings(fScale, fBias, 0.0f, 0.0f);
		Display::Device()->SetPixelShaderParameterF(m_PixelShaderParameter, (F32*)&psSettings);

		// display the effect
		DisplayFinal();

		// reset states
		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	}
}

void C_LookupTable::BeginUpdate()
{
	m_TableIndex = (m_TableIndex + 1) % 3;

	Display::C_LockedBox lockedBox = m_LookupTable[m_TableIndex].LockBox3D(0);
	m_TaskData.m_pData			= (U32*)lockedBox.pBits;

	m_TaskData.m_iSize			= m_Size;
	m_TaskData.m_iRowPitch		= lockedBox.rowPitch;
	m_TaskData.m_iSlicePitch	= lockedBox.slicePitch;
	m_TaskData.m_fBrightness	= m_Settings.fBrightness;
	m_TaskData.m_fContrast		= m_Settings.fContrast;
	m_TaskData.m_fGamma			= m_Settings.fGamma;
	m_TaskData.m_fSepia			= m_Settings.fSepia;
	m_TaskData.m_vAdd			= m_Settings.vAdd;
	m_TaskData.m_vModulate		= m_Settings.vModulate;
	m_TaskData.m_vDesaturate	= m_Settings.vDesaturate;

	Scheduler::AddTask(ColourLookupTableModule, 0, &m_TaskData, m_GroupHandle, m_PolicyHandle, 0, 0);
}

void C_LookupTable::EndUpdate()
{
	while (Scheduler::jqGetQueuedBatchCount(m_GroupHandle) + Scheduler::jqGetExecutingBatchCount(m_GroupHandle) > 0);

	m_LookupTable[m_TableIndex].UnlockBox3D(0);
}

}; // namespace PostProcess
