#include "stdafx.h"
#include "SceneGraph/Visitor.h"
#include "SceneGraph/Light/Manager.h"
#include "SceneGraph/View.h"
#include "SceneGraph/RenderNode.h"

namespace SceneGraph
{

extern Debug::Logger &g_Log;

C_LightManager::C_LightManager(C_SceneGraph* pSceneGraph) : m_NumPointLights(0), m_MaxPointLights(256), m_NumSpotLights(0), m_MaxSpotLights(128), m_NumDropShadows(0), m_MaxDropShadows(1024),
	m_disablePointLightCountWarning(false),
	m_disableSpotLightCountWarning(false),
	m_disableDropShadowCountWarning(false),
	m_pSceneGraph(pSceneGraph)
{
}

C_LightManager::~C_LightManager()
{
	BIZ_ASSERT_MSG(m_PointLightPool.Allocated() == 0, "Allocated(%d)", m_PointLightPool.Allocated());
	BIZ_ASSERT_MSG(m_SpotLightPool.Allocated() == 0, "Allocated(%d)", m_SpotLightPool.Allocated());
	BIZ_ASSERT_MSG(m_DropShadowPool.Allocated() == 0, "Allocated(%d)", m_DropShadowPool.Allocated());
}

void C_LightManager::ReservePointLights(const S32 maxPointLights)
{
	m_MaxPointLights = maxPointLights;
}

C_PointLightHandle C_LightManager::AddPointLight(bool isManaged)
{
	if(m_NumPointLights < m_MaxPointLights)
	{
		C_PointLight* pLight = m_PointLightPool.Allocate();
		++m_NumPointLights;
		
		pLight->m_IsManaged = isManaged;
		pLight->m_pSceneGraph = m_pSceneGraph;
		if (isManaged)
			m_ActivePointLights.PushBack(*pLight);
		
		return C_PointLightHandle(pLight);
	}
	else
	{
		if (!m_disablePointLightCountWarning)
		{
			LOGWARN(g_Log, "SceneGraph::AddPointLight: Maximum point lights reached (m_MaxPointLights = %d)", m_MaxPointLights);
			m_disablePointLightCountWarning = true;
		}
		return C_PointLightHandle();
	}
}

void C_LightManager::RemovePointLight(C_PointLightHandle handle)
{
	if (C_PointLight* pLight = handle.Object())
	{
		m_disablePointLightCountWarning = false;
		--m_NumPointLights;
		++pLight->m_Salt;
		if (pLight->m_IsManaged)
			m_ActivePointLights.Erase(PointLights::Iterator(pLight));
		m_PointLightPool.Free(pLight);
	}
}

void C_LightManager::ReserveSpotLights(const S32 maxSpotLights)
{
	m_MaxSpotLights = maxSpotLights;
}

C_SpotLightHandle C_LightManager::AddSpotLight(bool isManaged)
{
	if(m_NumSpotLights < m_MaxSpotLights)
	{
		C_SpotLight* pLight = m_SpotLightPool.Allocate();
		++m_NumSpotLights;

		pLight->m_IsManaged = isManaged;
		pLight->m_pSceneGraph = m_pSceneGraph;
		if (isManaged)
			m_ActiveSpotLights.PushBack(*pLight);
		
		return C_SpotLightHandle(pLight);
	}
	else
	{
		if (!m_disableSpotLightCountWarning)
		{
			LOGWARN(g_Log, "SceneGraph::AddSpotLight: Maximum spot lights reached (m_MaxSpotLights = %d)", m_MaxSpotLights);
			m_disableSpotLightCountWarning = true;
		}
		return C_SpotLightHandle();
	}
}

void C_LightManager::RemoveSpotLight(C_SpotLightHandle handle)
{
	if (C_SpotLight* pLight = handle.Object())
	{
		m_disableSpotLightCountWarning = false;
		--m_NumSpotLights;
		++pLight->m_Salt;
		if (pLight->m_IsManaged)
			m_ActiveSpotLights.Erase(SpotLights::Iterator(pLight));
		m_SpotLightPool.Free(pLight);
	}
}

void C_LightManager::ReserveDropShadows(const S32 maxDropShadows)
{
	m_MaxDropShadows = maxDropShadows;
}

C_DropShadowHandle C_LightManager::AddDropShadow(bool isManaged)
{
	if(m_NumDropShadows < m_MaxDropShadows)
	{
		C_DropShadow* pShadow = m_DropShadowPool.Allocate();
		++m_NumDropShadows;

		pShadow->m_IsManaged = isManaged;
		pShadow->m_pSceneGraph = m_pSceneGraph;
		if (isManaged)
			m_ActiveDropShadows.PushBack(*pShadow);

		return C_DropShadowHandle(pShadow);
	}
	else
	{
		if (!m_disableDropShadowCountWarning)
		{
			LOGWARN(g_Log, "SceneGraph::AddDropShadows: Maximum drop shadows reached (m_MaxDropShadows = %d)", m_MaxDropShadows);
			m_disableDropShadowCountWarning = true;
		}
		return C_DropShadowHandle();
	}
}

void C_LightManager::RemoveDropShadow(C_DropShadowHandle handle)
{
	if (C_DropShadow* pShadow = handle.Object())
	{
		m_disableDropShadowCountWarning = false;
		--m_NumDropShadows;
		++pShadow->m_Salt;
		if (pShadow->m_IsManaged)
			m_ActiveDropShadows.Erase(DropShadows::Iterator(pShadow));
		m_DropShadowPool.Free(pShadow);
	}
}

Maths::C_RGBA_F32 C_LightManager::EvaluateLighting(const Maths::HVec3& vPoint) const
{
	Maths::HVec3 vLighting(Maths::Constants<Maths::HF32>::Zero());

	// point lights
	for (PointLights::ConstIterator it = m_ActivePointLights.Begin(); it != m_ActivePointLights.End(); ++it)
	{
		const C_PointLight& light = (*it);
		
		const Maths::HVec4	vBounds		= Maths::Load<Maths::HVec4>(&light.m_BoundingSphere);
		const Maths::HVec3	vPosition	= Maths::HVec3(vBounds);
		const Maths::HF32	fRadius		= vBounds.W();

		const Maths::HVec3	vVector		= vPoint - vPosition;
		const Maths::HF32	fDistanceSq	= vVector.Length2();
		const Maths::HF32	fRadiusSq	= fRadius * fRadius;

		const Maths::Scalar_Bool bOutside = (fRadiusSq < fDistanceSq);
		if (bOutside)
			continue;

		const Maths::HVec3	vColour		= Maths::Load<Maths::HVec3>(&light.m_Colour);

		Maths::HF32	fFalloff	= fDistanceSq * fRadiusSq.Rcp();
					fFalloff	= Maths::Constants<Maths::HF32>::One() - fFalloff;
					fFalloff	= Maths::Clamp(fFalloff, Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::One());

		vLighting += vColour * fFalloff;
	}

	// spot lights
	for (SpotLights::ConstIterator it = m_ActiveSpotLights.Begin(); it != m_ActiveSpotLights.End(); ++it)
	{
		const C_SpotLight& light = (*it);
		
		const Maths::HVec4	vBounds		= Maths::Load<Maths::HVec4>(&light.m_BoundingSphere);
		const Maths::HVec3	vPosition	= Maths::HVec3(vBounds);
		const Maths::HF32	fRadius		= vBounds.W();

		const Maths::HVec3	vVector		= vPoint - vPosition;
		const Maths::HF32	fDistanceSq	= vVector.Length2();
		const Maths::HF32	fRadiusSq	= fRadius * fRadius;

		const Maths::Scalar_Bool bOutside = (fRadiusSq < fDistanceSq);
		if (bOutside)
			continue;

		const Maths::HVec3	vColour		= Maths::Load<Maths::HVec3>(&light.m_Colour);
		const Maths::HVec3	vDirection	= Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&light.m_Matrix.rz);
		const Maths::HF32	fDDotL		= Maths::Clamp(vVector | vDirection, Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::One());
		const Maths::HF32	fCosAlpha	= cosf(light.m_HorizontalFoV);

		Maths::HF32	fScale		= Maths::Constants<Maths::HF32>::One() - fCosAlpha;
					fScale		= fScale.Rcp();
					fScale		= (fDDotL - fCosAlpha) * fScale;

		Maths::HF32	fFalloff	= fDistanceSq * fRadiusSq.Rcp();
					fFalloff	= Maths::Constants<Maths::HF32>::One() - fFalloff;
					fFalloff	= Maths::Clamp(fFalloff, Maths::Constants<Maths::HF32>::Zero(), Maths::Constants<Maths::HF32>::One());

		vLighting += vColour * fFalloff * fScale;
	}

	return Maths::C_RGBA_F32(vLighting.X(), vLighting.Y(), vLighting.Z(), Maths::Constants<Maths::HF32>::One());
}

void C_LightManager::Accept(I_Visitor& visitor) const
{
	visitor.Visit(*this);
}

void C_LightManager::OnGather(const C_View& view) const
{
	// TODO: move this to a job, its pretty slow
	if(view.RenderNodeEnabled(Flag<E_RenderNodeType_DynamicLight>()))
	{
		Gather(view);
	}
}

void C_LightManager::Gather(const C_View& view) const
{
	PERF_EVENT("LightManager.Gather");

	for(PointLights::ConstIterator pIterator = m_ActivePointLights.Begin(); pIterator != m_ActivePointLights.End(); ++pIterator)
	{
		const C_PointLight& light = *pIterator;
		light.Gather(view);
	}
	for(SpotLights::ConstIterator pIterator = m_ActiveSpotLights.Begin(); pIterator != m_ActiveSpotLights.End(); ++pIterator)
	{
		const C_SpotLight& light = *pIterator;
		light.Gather(view);
	}
	for(DropShadows::ConstIterator pIterator = m_ActiveDropShadows.Begin(); pIterator != m_ActiveDropShadows.End(); ++pIterator)
	{
		const C_DropShadow& shadow = *pIterator;
		shadow.Gather(view);
	}
}

#ifndef CFG_GOLD
void C_LightManager::LogDebug()
{
	LOGINFO(g_Log, "NumPointLights(%d/%d), NumSpotLights(%d/%d), NumDropShadows(%d/%d)", m_NumPointLights, m_MaxPointLights, m_NumSpotLights, m_MaxSpotLights, m_NumDropShadows, m_MaxDropShadows);
}
#endif

} // namespace SceneGraph