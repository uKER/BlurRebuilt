#include "stdafx.h"
#include "Display/Occlusion/GCM/GCMOcclusionQuery.h"
#include "Display/Device/GCM/GCM.h"
#include "Display/Device/Device.h"

namespace Display
{
namespace GCM
{

C_OcclusionQuery::C_OcclusionQuery() :
	m_CurrIndex(0)
{
}

C_OcclusionQuery::~C_OcclusionQuery()
{
}

bool C_OcclusionQuery::Create()
{
	return true;
}

void C_OcclusionQuery::Destroy()
{
}

void C_OcclusionQuery::Begin()
{
	#ifndef DEVICE_DISABLE_OCCLUSION_QUERIES
#ifdef USE_UNSAFE_GCM
	cell::Gcm::cellGcmSetZpassPixelCountEnable(CELL_GCM_TRUE);
	cell::Gcm::cellGcmSetClearReport(CELL_GCM_ZPASS_PIXEL_CNT);
#else
	GCM_NAMESPACE::cellGcmSetZpassPixelCountEnable(gCellGcmCurrentContext, CELL_GCM_TRUE);
	GCM_NAMESPACE::cellGcmSetClearReport(gCellGcmCurrentContext, CELL_GCM_ZPASS_PIXEL_CNT);
#endif
	#endif
}

void C_OcclusionQuery::End()
{
	#ifndef DEVICE_DISABLE_OCCLUSION_QUERIES
	m_CurrIndex = GCMGetNextReportIndex1();
	GCMSetReport(CELL_GCM_ZPASS_PIXEL_CNT, m_CurrIndex);
	GCM_NAMESPACE::cellGcmSetZpassPixelCountEnable(gCellGcmCurrentContext, CELL_GCM_FALSE);
	#endif
}

}; // namespace GCM
}; // namespace Display
