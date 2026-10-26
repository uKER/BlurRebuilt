#include "stdafx.h"
#include "Dynamic/Light/Manager.h"

#include "Dynamic/Node.h"
#include "Dynamic/Renderer.h"
#include "Dynamic/TypeIndex.h"
#include "Dynamic/Light/EffectInterface.h"

#include "Dynamic/Shaders/Constants.h"
#include "Render/State/RenderState.h"

#include "System/Debug/I_DebugRender.h"

EXTERN_SHADER(vsLightDefault2d);
EXTERN_SHADER(vsLightDefault3d);
EXTERN_SHADER(psLightDefault);
EXTERN_SHADER(psLightDefaultHDR);
EXTERN_SHADER(psLightDefaultHDRU32);

namespace DynamicEffect
{
namespace Light
{

Debug::Bool g_debugDoVisibilityQuery("Graphics.Dynamic.Light.DoVisibilityQuery", true);
Debug::Bool g_debugDisplayQuery("Graphics.Dynamic.Light.DisplayQuery", false);
Debug::Bool g_debugDoVisibilityOnScreen("Graphics.Dynamic.Light.DoVisibilityOnScreen", true);
Debug::Bool g_debugFreezeOcclusionQueries("Graphics.Dynamic.Light.FreezeOcclusionQueries", false);

Debug::Float g_debugIntensityScale("Graphics.Dynamic.Light.IntensityScale", 1.0f, Debug::Range(0.0f, 8.0f));
Debug::Float g_debugGlobalOffsetX("Graphics.Dynamic.Light.GlobalOffsetX", 0.0f, Debug::Range(-4.0f, +4.0f));
Debug::Float g_debugGlobalOffsetY("Graphics.Dynamic.Light.GlobalOffsetY", 0.0f, Debug::Range(-4.0f, +4.0f));

C_Manager::C_Manager(C_Renderer* pRenderer) : I_DynamicEffectManager(pRenderer, E_ManagerTypeIndex_Light)
{
}

C_Manager::~C_Manager()
{
	FreeLightDefinitions();
	m_LightList.Free();
}

bool C_Manager::Create()
{
	if (!Display::CreateVertexShaderFromFunc("vsLightDefault2d", 0, &m_VertexShader[E_VertexShader_Default2d], REF_SHADER(vsLightDefault2d)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("vsLightDefault3d", 0, &m_VertexShader[E_VertexShader_Default3d], REF_SHADER(vsLightDefault3d)))
		return false;

	if(Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		// choose which set of shaders we need to use - we only have depth fog for shader tonemap at the moment
		if(eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("psLightDefaultHDRU32", 0, &m_PixelShader[E_PixelShader_Default], REF_SHADER(psLightDefaultHDRU32)))
				return false;
		}
		else if(eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psLightDefaultHDRU32", 0, &m_PixelShader[E_PixelShader_Default], REF_SHADER(psLightDefaultHDRU32)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psLightDefault", 0, &m_PixelShader[E_PixelShader_Default], REF_SHADER(psLightDefault)))
			return false;
	}

	for (U32 shaderNo = 0; shaderNo < E_PixelShader_Count; ++shaderNo)
	{
		m_LuminanceParam[shaderNo] = m_PixelShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");
	}

	if (!m_OcclusionQueries.Create(32 * 8))
		return false;

	return true;
}

bool C_Manager::CreateVolatile()
{
	Display::Device()->AcquireThreadOwnership();
	bool result = m_OcclusionQueries.CreateVolatile();
	Display::Device()->ReleaseThreadOwnership();

	return result;
}

bool C_Manager::Initialise()
{
	return true;
}

void C_Manager::Destroy()
{
	CleanLights();

	m_OcclusionQueries.Destroy();

	for (U32 shaderNo = 0; shaderNo < E_VertexShader_Count; ++shaderNo)
		Display::Device()->Destroy(&m_VertexShader[shaderNo]);
	for (U32 shaderNo = 0; shaderNo < E_PixelShader_Count; ++shaderNo)
		Display::Device()->Destroy(&m_PixelShader[shaderNo]);
}

void C_Manager::DestroyVolatile()
{
	m_OcclusionQueries.DestroyVolatile();
}

void C_Manager::Shutdown()
{
}

void C_Manager::Reset()
{
}

void C_Manager::SetShaderConstants() const
{
}

void C_Manager::SetVertexShader(const E_VertexShader eVertexShader)
{
	Display::Device()->SetVertexShader(&m_VertexShader[eVertexShader]);
}

void C_Manager::SetPixelShader(const E_PixelShader ePixelShader)
{
	MathsStorage::Float4 hdrParam = Render::C_RenderState::Instance().ShaderHDR() ? Render::C_RenderState::Instance().HDR()->GetShaderParameters() : Render::C_RenderState::Instance().GetGainShaderParameters();
	Display::Device()->SetPixelShaderParameterF(m_LuminanceParam[ePixelShader], (F32*)&hdrParam);

	Display::Device()->SetPixelShaderParameters(&m_PixelShader[ePixelShader]);
	Display::Device()->SetPixelShader(&m_PixelShader[ePixelShader]);
}

void C_Manager::CleanEffect(const I_DynamicEffect* pEffect)
{
	// iterate through lights and remove any that have instances that belong to this effect
	Ut::Vector<Light*> lightsToErase;
	for (Lights::Iterator it = m_Lights.Begin(); it != m_Lights.End(); ++it)
	{
		const InstanceDefinitions& instances = it->GetValue()->m_pDefinition->m_Instances;
		for (S32 instanceNo = 0; instanceNo < instances.Size(); ++instanceNo)
		{
			if (instances[instanceNo].m_iEffect == pEffect->GetIndex())
			{
				lightsToErase.PushBack(it->GetValue());
				continue;
			}
		}		
	}
	for (S32 lightNo = 0; lightNo < lightsToErase.Size(); ++lightNo)
	{
		m_Lights.Erase(lightsToErase[lightNo]);
	}
}

void C_Manager::CleanNode(const C_Node* pNode)
{
	// iterate through lights and remove any that are attached to this node
	Ut::Vector<Light*> lightsToErase;
	for (Lights::Iterator it = m_Lights.Begin(); it != m_Lights.End(); ++it)
	{
		Light* pLight = it->GetValue();
		if (pLight->m_pNode == pNode)
			lightsToErase.PushBack(pLight);
	}
	for (S32 lightNo = 0; lightNo < lightsToErase.Size(); ++lightNo)
	{
		m_Lights.Erase(lightsToErase[lightNo]);
	}
}

void C_Manager::Render(const U32 visibilityFlags)
{
	PERF_EVENT("DynamicEffect::LightManager::Render");

	// calculate occlusion for selected lights
	Render::Occlusion::Begin(g_debugDisplayQuery);
	GatherQueries(visibilityFlags);
	ProcessQueries();
	Render::Occlusion::End();

	// calculate per viewport data, such as orientation and screen position
	const Maths::HF32 vZero = Maths::Constants<Maths::HF32>::Zero();
	const Maths::HF32 vOne = Maths::Constants<Maths::HF32>::One();
	const Maths::HVec3& vViewPosition = Display::Device()->GetViewSettings3D().GetCameraPosition();
	for (Lights::Iterator it = m_Lights.Begin(); it != m_Lights.End(); ++it)
	{
		Light* pLight = it->GetValue();

		if ((pLight->m_VisibilityFlags & visibilityFlags) == 0)
			continue;

		Maths::HVec3 vPosition = Maths::Load<Maths::HVec3>(&pLight->m_vPosition);
		Maths::C_Quaternion qRotation = Maths::Load<Maths::C_Quaternion>(&pLight->m_vRotation);
		if (pLight->m_pNode)
		{
			const Maths::C_QuatTrans& transform = *pLight->m_pNode;
			vPosition = vPosition * transform;
			qRotation = qRotation * transform.Quaternion();
		}

		// viewing angle and distance
		Maths::HVec3 vViewVector = vViewPosition - vPosition;
		Maths::HF32 vViewDistance = vViewVector.Length();
		Maths::HF32 vRcpViewDistance = vViewVector.RcpLength();
		vViewVector *= vRcpViewDistance;

		Maths::C_Normal vLightDirection = Maths::C_Normal(vZero, -vOne, vZero) * qRotation;
		Maths::HF32 vViewFactor = (vViewVector | vLightDirection);

		vViewFactor.MaxEq(vZero);
		vViewFactor.MinEq(vOne);
		Maths::Store(vViewFactor, &pLight->m_fView);
		Maths::Store(vViewDistance, &pLight->m_fDistance);

		// screen position
		Maths::HVec4 vPositionOne = Maths::HVec4(vPosition);
		vPositionOne.W(Maths::Constants<Maths::HF32>::One());
		Maths::HMat44 mViewProj = Display::Device()->GetViewSettings3D().GetWorldToDevice();
		Maths::HVec4 vScreenPosition = vPositionOne * mViewProj;
		vScreenPosition *= vScreenPosition.W().Rcp();
		Maths::Store(vScreenPosition, &pLight->m_vScreenPosition);
	}
}

void C_Manager::GatherQueries(const U32 visibilityFlags)
{
	const U32 viewNo = Display::Device()->GetView()->m_UserIndex;

	const Display::C_ViewSettings3D viewSettings3D =  Display::Device()->GetView()->GetViewSettings3D();
	const Display::C_Matrix& viewMatrix = viewSettings3D.GetViewMatrix();

	Maths::HVec4 vViewDirection(viewMatrix._11, viewMatrix._21, viewMatrix._31, viewMatrix._41);
	Maths::HF32 vDepthScale( 4294967296.0f / viewSettings3D.GetFarClip() );

	m_QueryArray.Clear();
	for (Lights::Iterator it = m_Lights.Begin(); it != m_Lights.End(); ++it)
	{
		Light* pLight = it->GetValue();

		if ((pLight->m_VisibilityFlags & visibilityFlags) == 0)
		{
			//Clear occussion queries for this view
			Render::Occlusion::C_PercentageQuery*& pQuery = pLight->m_OcclusionQueries[viewNo];
			if (pQuery)
			{
				m_OcclusionQueries.Free(pQuery);
				pQuery = 0;
			}
			Render::Occlusion::C_PercentageQuery*& pQuery2 = pLight->m_OcclusionQueries[viewNo+4];
			if (pQuery2)
			{
				m_OcclusionQueries.Free(pQuery2);
				pQuery2 = 0;
			}
			continue;
		}

		if (g_debugFreezeOcclusionQueries)
			continue;
		
		// if this light does not use occlusion queries, then exit
		if (pLight->m_pDefinition->m_eOcclusion != E_Occlusion_Query)
			continue;

		// get position and size
		Maths::HVec3 vPosition = Maths::Load<Maths::HVec3>(&pLight->m_vPosition);
		Maths::HVec3 vSize = Maths::Load<Maths::HVec3>(&pLight->m_pDefinition->m_vSize);
		if (pLight->m_pNode)
		{
			const Maths::C_QuatTrans transform = *pLight->m_pNode;
			vPosition = vPosition * transform;
		}

		// get a sort key for depth sorting
		Maths::HVec4 vPositionOne(vPosition);
		vPositionOne.W(Maths::Constants<Maths::HF32>::One());
		Maths::HF32 vDepth = (vPositionOne | vViewDirection);
		vDepth = vDepth * vDepthScale;

		// build the instance
		LightInstance instance;
		instance.pLight = pLight;
		instance.sortKey = (U32)vDepth;
		m_QueryArray.PushBack(instance);
	}	

	m_QuerySorter.Sort(m_QueryArray);
}

void C_Manager::ProcessQueries()
{
	for (S32 queryNo = 0; queryNo < m_QueryArray.Size(); ++queryNo)
	{
		ReceiveVisibility(m_QueryArray[queryNo].pLight);
		RequestVisibility(m_QueryArray[queryNo].pLight);
	}
}

void C_Manager::RemoveQueries(Light* pLight)
{
	for (S32 queryNo = 0; queryNo < pLight->m_OcclusionQueries.Size(); ++queryNo)
	{	
		Render::Occlusion::C_PercentageQuery*& pQuery = pLight->m_OcclusionQueries[queryNo];
		if (pQuery)
		{
			m_OcclusionQueries.Free(pQuery);
			pQuery = 0;
		}
	}
}

void C_Manager::ReceiveVisibility(Light* pLight)
{
	const U32 viewNo = Display::Device()->GetView()->m_UserIndex;

	// occlusion query
	#ifndef CFG_GOLD
	if (!g_debugDoVisibilityQuery)
		pLight->m_fOcclusion = 1.0f;
	else
	#endif
	{
		Render::Occlusion::C_PercentageQuery*& pQuery = pLight->m_OcclusionQueries[viewNo];
		if (pQuery)
		{
			pLight->m_fOcclusion = pQuery->Visible();
			m_OcclusionQueries.Free(pQuery);
		}
		else
			pLight->m_fOcclusion = pLight->m_fDefaultOcclusion;
	}

	// proportion on screen
	#ifndef CFG_GOLD
	if (g_debugDoVisibilityOnScreen)
	#endif
	{
		Maths::HVec3	vPosition	= Maths::Load<Maths::HVec3>(&pLight->m_vPosition);
		Maths::HVec3	vSize		= Maths::Load<Maths::HVec3>(&pLight->m_pDefinition->m_vSize);
		if (pLight->m_pNode)
		{
			const Maths::C_QuatTrans transform = *pLight->m_pNode;
			vPosition = vPosition * transform;
		}
		Maths::HVec3	vViewPosition	= vPosition * Display::Device()->GetViewSettings3D().GetWorldToCamera();

		Maths::HF32		vPerspDist	= Display::Device()->GetViewSettings3D().GetPerspDist();
		Maths::C_Rect	rExtents	= Display::Device()->GetViewSettings3D().GetViewExtents();
		Maths::HVec2	vMinExtent	= rExtents.Min();
		Maths::HVec2	vMaxExtent	= rExtents.Max();
		rExtents.Min(vMinExtent * vViewPosition.Z() * vPerspDist.Rcp());
		rExtents.Max(vMaxExtent * vViewPosition.Z() * vPerspDist.Rcp());

		Maths::HF32		vRadius		= Maths::Min(vSize.X(), Maths::Min(vSize.Y(), vSize.Z()));
		Maths::HVec2	vExtent		= Maths::HVec2(vRadius, vRadius);
		Maths::HVec2	vLightMin	= Maths::HVec2(vViewPosition) - vExtent;
		Maths::HVec2	vLightMax	= Maths::HVec2(vViewPosition) + vExtent;
		vLightMin.MinEq(rExtents.Max());
		vLightMin.MaxEq(rExtents.Min());
		vLightMax.MinEq(rExtents.Max());
		vLightMax.MaxEq(rExtents.Min());
		Maths::HVec2	vLightSize	= vLightMax - vLightMin;

		Maths::HF32		vArea		= Maths::Constants<Maths::HF32>::Quarter() * Maths::Abs(vLightSize.X() * vLightSize.Y());
		Maths::HF32		vMaxArea	= vRadius * vRadius;
		Maths::HF32		vVisibility	= vArea * vMaxArea.Rcp();
		vVisibility.MaxEq(Maths::Constants<Maths::HF32>::Zero());
		vVisibility.MinEq(Maths::Constants<Maths::HF32>::One());
	
		pLight->m_fOcclusion = pLight->m_fOcclusion * vVisibility;
	}
}

Debug::Float g_debugLightFlareQueryOffset("Vehicle.LightFlareQueryOffset", -0.3f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugLightFlareSizeScalar("Vehicle.LightFlareSizeScalar", 1.0f, Debug::Range(0.0001f, 1000.0f), Debug::Step(0.01f));

void C_Manager::RequestVisibility(Light* pLight)
{
	#ifndef CFG_GOLD
	if (!g_debugDoVisibilityQuery)
		return;
	#endif

	const U32 viewNo = Display::Device()->GetView()->m_UserIndex;
	Render::Occlusion::C_PercentageQuery*& pQuery0 = pLight->m_OcclusionQueries[viewNo];
	Render::Occlusion::C_PercentageQuery*& pQuery1 = pLight->m_OcclusionQueries[viewNo+4];

	pQuery0 = pQuery1;
	pQuery1 = m_OcclusionQueries.Allocate();		
	if (pQuery1)
	{
		Maths::HVec3 vPosition = Maths::Load<Maths::HVec3>(&pLight->m_vPosition);
		Maths::HVec3 vSize = Maths::Load<Maths::HVec3>(&pLight->m_pDefinition->m_vSize);
		
		if (pLight->m_pNode)
		{
			const Maths::C_QuatTrans transform = *pLight->m_pNode;
			vPosition	= vPosition * transform;
		}

		//Camera Vector
		Maths::HVec3 cameraVector = (vPosition - Display::Device()->GetView()->GetViewSettings3D().GetCameraPosition());
		cameraVector.Normalise();
		Maths::HVec3 vOffset = vPosition + cameraVector * (float)g_debugLightFlareQueryOffset;

		Maths::HMat43 mWorld;
		mWorld.AW(vOffset);
		pQuery1->SendFirstPass(mWorld, vSize);
		pQuery1->SendSecondPass(mWorld, vSize);
		mWorld.AW(vPosition);
	}
}

LightHandle C_Manager::CreateLight(const LightDefinition* pDefinition, const Maths::C_QuatTrans& transform, const C_Node *pNode, const Maths::RGBA& colour, const F32 fIntensity, const F32 fDefaultOcclusion)
{
	if (!pDefinition)
		return LightHandle();

	Light* pLight = NewLight();
	if (pLight)
	{
		pLight->m_pDefinition = pDefinition;
		pLight->m_pNode = pNode;

		Maths::Store(transform.Translation(), &pLight->m_vPosition);
		Maths::Store(transform.Quaternion(), &pLight->m_vRotation);
		pLight->m_iColour = colour;
		pLight->m_fIntensity = fIntensity;
		pLight->m_fOcclusion = 1.0f;
		pLight->m_fDefaultOcclusion = fDefaultOcclusion;

		for (S32 queryNo = 0; queryNo < pLight->m_OcclusionQueries.Size(); ++queryNo)
			pLight->m_OcclusionQueries[queryNo] = 0;

		CreateInstances(pLight);

		return LightHandle(pLight);
	}
	else
		return LightHandle();
}

void C_Manager::DestroyLight(Light* pLight)
{
	if (!pLight)
		return;

	DestroyInstances(pLight);

	RemoveQueries(pLight);
	RemoveLight(pLight);
}

void C_Manager::ReserveLights(const U32 maxLights)
{
	for (U32 lightNo = 0; lightNo < maxLights; ++lightNo)
	{
		m_LightList.Cache(new Light());
	}
}

void C_Manager::CleanLights()
{
	Ut::Vector<Light*> toErase;
	for (Lights::Iterator it = m_Lights.Begin(); it != m_Lights.End(); ++it)
		toErase.PushBack(it->GetValue());
	for (Ut::Vector<Light*>::Iterator it = toErase.Begin(); it != toErase.End(); ++it)
		DestroyLight(*it);
}

void C_Manager::GetExtents(const LightDefinition* pDefinition, const Maths::C_QuatTrans& transform, MathsStorage::Float4A16* pvCentre, MathsStorage::Float4A16* pvSizeCull)
{	
	const InstanceDefinitions& instances = pDefinition->m_Instances;

	Maths::HVec3 vPosition	= transform.Translation();
	Maths::HVec3 vSize		= Maths::Load<Maths::HVec3>(&pDefinition->m_vSize);
	Maths::HVec3 vMin		= vPosition - vSize;
	Maths::HVec3 vMax		= vPosition + vSize;
	Maths::HF32 vCullDistance = Maths::Constants<Maths::HF32>::Zero();

	for (InstanceDefinitions::ConstIterator it = instances.Begin(); it != instances.End(); ++it)
	{
		I_EffectBase* pEffect = (I_EffectBase*) m_pRenderer->FindEffect(it->m_iEffect);
		if (pEffect)
			pEffect->UpdateExtents(&(*it), transform, vMin, vMax, vCullDistance);			
	}

	Maths::HVec4 vCentre = Maths::HVec4(vMax + vMin);
	Maths::HVec4 vSizeCull = Maths::HVec4(vMax - vMin);
	vCentre *= Maths::Constants<Maths::HF32>::Half();
	vSizeCull *= Maths::Constants<Maths::HF32>::Half();
	vCentre.W(Maths::Constants<Maths::HF32>::One());
	vSizeCull.W(vCullDistance);

	Maths::Store(vCentre, pvCentre);
	Maths::Store(vSizeCull, pvSizeCull);
}

void C_Manager::CreateInstances(Light* pLight)
{
	const InstanceDefinitions& instances = pLight->m_pDefinition->m_Instances;

	InstanceDefinitions::ConstIterator itBegin = instances.Begin();
	InstanceDefinitions::ConstIterator itEnd = instances.End();
	for (InstanceDefinitions::ConstIterator it = itBegin; it != itEnd; ++it)
	{
		const InstanceDefinition &instance = *it;

		I_EffectBase* pEffect = (I_EffectBase*)m_pRenderer->FindEffect(instance.m_iEffect);
		if (pEffect)
			pEffect->CreateInstance(&instance, pLight);
	}
}

void C_Manager::DestroyInstances(Light* pLight)
{
	const InstanceDefinitions& instances = pLight->m_pDefinition->m_Instances;

	InstanceDefinitions::ConstIterator itBegin = instances.Begin();
	InstanceDefinitions::ConstIterator itEnd = instances.End();
	for (InstanceDefinitions::ConstIterator it = itBegin; it != itEnd; ++it)
	{
		I_EffectBase* pEffect = (I_EffectBase*)m_pRenderer->FindEffect(it->m_iEffect);
		if (pEffect)
			pEffect->Clean(pLight);
	}
}

Light* C_Manager::NewLight()
{
	Light* pLight = m_LightList.Allocate();
	if (!pLight)
		LOGWARN(Render::g_Log, "DynamicEffect: LightCache empty!\n");
	else
		m_Lights.Insert(pLight);

	return pLight;
}

void C_Manager::RemoveLight(Light* pLight)
{
	assert(pLight);

	m_Lights.Erase(pLight);

	pLight->m_iSalt += 1;
	m_LightList.Free(pLight);
}

void C_Manager::FreeLights()
{
	Lights::Iterator itBegin = m_Lights.Begin();
	Lights::Iterator itEnd = m_Lights.End();
	for (Lights::Iterator it = itBegin; it != itEnd; ++it)
	{
		DestroyLight(it->GetValue());
	}
	m_Lights.Clear();
}

void C_Manager::AddLightDefinition(LightDefinition* pDefinition)
{
	m_Definitions.Insert(pDefinition);
}

void C_Manager::FreeLightDefinition(LightDefinition* pDefinition)
{
	Ut::Vector<Light*> toErase;
	for (Lights::Iterator it = m_Lights.Begin(); it != m_Lights.End(); ++it)
	{
		if (it->GetValue()->m_pDefinition == pDefinition)
		{
			toErase.PushBack(it->GetValue());
		}
	}
	for (Ut::Vector<Light*>::Iterator it = toErase.Begin(); it != toErase.End(); ++it)
	{
		DestroyLight(*it);
	}

	m_Definitions.Erase(pDefinition);
	delete pDefinition;
}

void C_Manager::FreeLightDefinitions()
{
	for (LightDefinitions::Iterator it = m_Definitions.Begin(); it != m_Definitions.End(); ++it)
	{
		delete it->GetValue();
	}
	m_Definitions.Clear();
}

// 3D debug graphics
void C_Manager::DisplayDebugGraphics() const
{
	for (Lights::ConstIterator it = m_Lights.Begin(); it != m_Lights.End(); ++it)
	{
		const Light* pLight = it->GetValue();

		Maths::HVec3 vPosition = Maths::Load<Maths::HVec3>(&pLight->m_vPosition);
		Maths::HVec3 vSize = Maths::Load<Maths::HVec3>(&pLight->m_pDefinition->m_vSize);

		Display::BasicRender()->DrawColoured3DBox(vPosition, vSize, pLight->m_iColour);
	}	
}

// 2D overlays etc
MathsStorage::Int2 C_Manager::DisplayDebugInfo(MathsStorage::Int2 position) const
{
	position.y += 25;

	Util::C_String128 str;
	str.Printf("Active Lights: %d\0", m_Lights.Size());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(position.x, position.y), str);
	position.y += 25;

	return position;
}

}; // namespace Light
}; // namespace DynamicEffect
