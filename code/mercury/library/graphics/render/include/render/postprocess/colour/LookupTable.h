#ifndef RENDER_POSTPROCESS_COLOUR_LUT_H
#define RENDER_POSTPROCESS_COLOUR_LUT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Colour/Task.h>
#include <System/Scheduler.h>

namespace PostProcess
{

struct ColourSettings
{
	ColourSettings() : fBrightness(0.0f), fContrast(1.0f), fGamma(1.0f), fSepia(0.0f), vDesaturate(0.0f, 0.0f, 0.0f), vAdd(0.0f, 0.0f, 0.0f), vModulate(1.0f, 1.0f, 1.0f) {};

	F32						fBrightness;
	F32						fContrast;
	F32						fGamma;
	F32						fSepia;
	MathsStorage::Float3	vDesaturate;
	MathsStorage::Float3	vAdd;
	MathsStorage::Float3	vModulate;
};

class C_LookupTable : public I_PostProcess
{
public:

	C_LookupTable( C_Manager& manager );
	~C_LookupTable();

	bool Create();
	void Destroy();
	bool CreateVolatile() { return true; }
	void DestroyVolatile() {}
	void Update() {};

	void Display() const;

	const Display::C_Texture3D*	GetLookupTable() const { return &m_LookupTable[m_TableIndex]; }

	void SetJobQueuePolicyHandle(const Scheduler::PolicyHandle& policyHandle) { m_PolicyHandle = policyHandle; }

	void SetBrightness(const F32 fBrightness)								{ m_Settings.fBrightness = fBrightness; }
	void SetContrast(const F32 fContrast)									{ m_Settings.fContrast = fContrast; }
	void SetSepia(const F32 fSepia)											{ m_Settings.fSepia = fSepia; }
	void SetGamma(const F32 fGamma)											{ m_Settings.fGamma = fGamma; }
	void SetDesaturate(const F32 fRed, const F32 fGreen, const F32 fBlue)	{ m_Settings.vDesaturate = MathsStorage::Float3(fRed, fGreen, fBlue); }
	void SetAdd(const F32 fRed, const F32 fGreen, const F32 fBlue)			{ m_Settings.vAdd = MathsStorage::Float3(fRed, fGreen, fBlue); }
	void SetModulate(const F32 fRed, const F32 fGreen, const F32 fBlue)		{ m_Settings.vModulate = MathsStorage::Float3(fRed, fGreen, fBlue); }

public:

	void BeginUpdate();
	void EndUpdate();

private:

	Display::C_PixelShader			m_PixelShader;
	Display::C_PixelShaderParameter	m_PixelShaderParameter;

	Display::C_Texture3D			m_LookupTable[3];
	U32								m_TableIndex;
	U32								m_Size;

	ColourSettings					m_Settings;
	ColourTaskData					m_TaskData;

	Scheduler::GroupHandle			m_GroupHandle;
	Scheduler::PolicyHandle			m_PolicyHandle;

};

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_COLOUR_LUT_H
