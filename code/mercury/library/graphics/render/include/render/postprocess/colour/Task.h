#ifndef RENDER_POSTPROCESS_COLOUR_TASK_H
#define RENDER_POSTPROCESS_COLOUR_TASK_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Scheduler
{
	struct Task;
};

namespace PostProcess
{

struct ALIGN16 ColourTaskData
{
	U32*					m_pData;
	U32						m_iRowPitch;
	U32						m_iSlicePitch;
	U32						m_iSize;
	F32						m_fBrightness;
	F32						m_fContrast;
	F32						m_fGamma;
	F32						m_fSepia;
	MathsStorage::Float3	m_vAdd;
	MathsStorage::Float3	m_vModulate;
	MathsStorage::Float3	m_vDesaturate;
};

void ColourLookupTableBuild(ColourTaskData* pData);
void ColourLookupTableTask(Scheduler::Task* pTask);

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_COLOUR_TASK_H
