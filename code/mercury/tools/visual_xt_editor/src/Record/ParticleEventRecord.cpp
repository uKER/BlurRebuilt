
#include "stdafx.h"

#include "ParticleEventRecord.h"

#include "xt\xttool.h"
#include "../Frame.h"
#include "../Preview/PreviewWindow.h"

void C_ParticleEventRecord::Select()
{
	Maths::C_QuatTrans transform;
	transform.Translation(Maths::HVec3(0.0f, 1.0f, 0.0f));
	C_Frame::Instance().GetPreviewWindow()->SpawnEvent(m_pDomRecord->id, transform);

	C_GenericRecord::Select();
}