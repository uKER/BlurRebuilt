#ifndef DISPLAY_OCCLUSION_GCM_GCM_OCCLUSIONQUERY_INL
#define DISPLAY_OCCLUSION_GCM_GCM_OCCLUSIONQUERY_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{
namespace GCM
{

INLINE const bool C_OcclusionQuery::IsCreated() const
{
	return true;
}

INLINE const E_OcclusionQueryResult C_OcclusionQuery::Visible() const
{
	#ifdef DEVICE_DISABLE_OCCLUSION_QUERIES
	return E_OcclusionQueryResult_Visible;
	#else
	return cellGcmGetReport(CELL_GCM_ZPASS_PIXEL_CNT, m_CurrIndex) > 0 ? E_OcclusionQueryResult_Visible : E_OcclusionQueryResult_NotVisible;
	#endif
}

INLINE const E_OcclusionQueryResult C_OcclusionQuery::Visible(U32& visible) const
{
	#ifdef DEVICE_DISABLE_OCCLUSION_QUERIES
	visible = 1;
	return E_OcclusionQueryResult_Visible;
	#else
	visible = cellGcmGetReport(CELL_GCM_ZPASS_PIXEL_CNT, m_CurrIndex);
	return visible > 0 ? E_OcclusionQueryResult_Visible : E_OcclusionQueryResult_NotVisible;
	#endif
}

INLINE const E_OcclusionQueryResult C_OcclusionQuery::Visible(F32& visible) const
{
	visible = 1.f;
	#ifdef DEVICE_DISABLE_OCCLUSION_QUERIES
	return E_OcclusionQueryResult_Visible;
	#else
	U32 pixelsPassed = cellGcmGetReport(CELL_GCM_ZPASS_PIXEL_CNT, m_CurrIndex);
	return (pixelsPassed > 0) ? E_OcclusionQueryResult_Visible : E_OcclusionQueryResult_NotVisible;
	#endif
}

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_OCCLUSION_GCM_GCM_OCCLUSIONQUERY_INL
