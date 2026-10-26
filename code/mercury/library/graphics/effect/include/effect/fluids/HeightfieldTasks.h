#ifndef EFFECT_FLUIDS_HEIGHTFIELD_TASKS_H
#define EFFECT_FLUIDS_HEIGHTFIELD_TASKS_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "System/Scheduler/JobQueue.h"

extern Scheduler::jqModule HeightfieldModifyModule;
extern Scheduler::jqModule HeightfieldSolveModule;

namespace Effect
{

struct ALIGN16 HeightfieldModifyData
{
	U8*	m_pNext;
	U8* m_pCurrent;
	U32	m_iSurfaceWidth;
	U32	m_iSurfaceHeight;
	U32	m_iSurfacePitch;
	U8*	m_pModify;
	U32	m_iModifyWidth;
	U32	m_iModifyHeight;
	U32	m_iModifyPitch;
	F32	m_fScale;
	MathsStorage::Float4 m_vTransformU;
	MathsStorage::Float4 m_vTransformV;
};

struct ALIGN16 HeightfieldSolveData
{
	U8*	m_pNext;
	U8*	m_pCurrent;
	U8*	m_pPrevious;
	U8*	m_pTop;
	U8*	m_pBottom;
	U8*	m_pLeft;
	U8*	m_pRight;
	U32	m_iWidth;
	U32	m_iHeight;
	U32	m_iPitch;
	F32	m_fScale;
	F32	m_fDamping;
};

void HeightfieldModify(Scheduler::Task* pTask);
void HeightfieldSolve(Scheduler::Task* pTask);

}; // namespace Effect

#endif // EFFECT_FLUIDS_HEIGHTFIELD_TASKS_H
