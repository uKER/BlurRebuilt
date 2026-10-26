#include "stdafx.h"
#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/Visitor.h"
#include "SceneGraph/Light/Light.h"
#include "SceneGraph/View.h"
#include "SceneGraph/VisibleDetect.h"

#include "System/Debug/I_DebugRender.h"

namespace SceneGraph
{

Debug::Float	g_debugLightBeginFade		("Graphics.SceneGraph.Light.BeginFade",			0.9f,	Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugLightNearFade		("Graphics.SceneGraph.Light.NearFade",			4.0f,	Debug::Range(0.0f, 32.0f));
Debug::Float	g_debugLightNearCull		("Graphics.SceneGraph.Light.NearCull",			0.0f,	Debug::Range(-32.0f, +8.0f));
Debug::Bool		g_debugLightNearCullOverride("Graphics.SceneGraph.Light.NearCullOverride",	false);

C_Light::C_Light() : m_Salt(0), m_IsManaged(true), m_RenderFlags(0), m_ShadowFlags(0), m_CullDistance(1000.0f), m_NearCull(-1000.0f), m_pData(0), m_Flags(0), m_pVisDetect(NULL), m_VisibilityFlags(0xFFFFFFFF)
{
}

C_Light::~C_Light()
{
}

void C_Light::AllocatePerViewIntensities(S32 numPerViewIntensities)
{
	m_PerViewIntensities.Resize(numPerViewIntensities);
	for (S32 i = 0; i < numPerViewIntensities; i++) m_PerViewIntensities[i] = 1.f;
}

void C_Light::SetPerViewIntensity(S32 viewType, F32 intensity)
{
	BIZ_ASSERT(viewType < m_PerViewIntensities.Size());
	m_PerViewIntensities[viewType] = intensity;
}

void C_Light::SetVisibility( const U32 iViewBitfield ) const
{
	if (!m_pVisDetect)
		return;

	if (MT::AtomicOr(&m_pVisDetect->flag, iViewBitfield | E_VisDetectFlag_AddedToNewList | E_VisDetectFlag_DefaultVisible) & (E_VisDetectFlag_AddedToNewList | E_VisDetectFlag_AddedToVisibleList) )
		return;

	if (m_pSceneGraph)
		m_pSceneGraph->AddNewlyVisible(m_pVisDetect);
}

C_PointLight::C_PointLight()
{
}

C_PointLight::~C_PointLight()
{
}

void C_PointLight::Gather(const C_View& view) const
{
	if ((m_VisibilityFlags & view.GetTypeFlag()) == 0)
		return;

	Maths::HVec3	vView		= view.GetViewSettings3D().GetCameraPosition();
	Maths::HMat44	mProjection	= view.GetViewSettings3D().GetCameraToDevice();

	Maths::HVec4 vPositionOne(Maths::Load<Maths::HVec4>(&m_BoundingSphere));
	Maths::HVec3 vColour(Maths::Load<Maths::HVec3>(&m_Colour));
	Maths::HF32 vRadius(vPositionOne.W());
	Maths::HF32 vCullDistance(m_CullDistance);
	Maths::HF32 vFarClip(view.GetViewData().farClip);
	vPositionOne.W(Maths::Constants<Maths::HF32>::One());
	Maths::HVec4 vForward(Maths::Load<Maths::HVec4>(&view.GetViewData().forwardZscaled));

	const F32	fade		= g_debugLightBeginFade;
	const F32	depthScale	= view.GetViewData().depthScale;
	
	if(Maths::LengthSquared(vColour) > Maths::Constants<Maths::HF32>::Epsilon() && view.GetViewFrustum().CullSphere(vPositionOne, vRadius, Maths::Min(vCullDistance, vFarClip)))
	{
		Maths::HF32 vDot = vPositionOne | vForward;
		Maths::HVec4 vCoord(vRadius, 0.0f, vDot, 1.0f);
		vCoord = vCoord * mProjection;
		Maths::HF32 vScreenSize = Maths::Abs(vCoord.X() / vCoord.W());

		const F32 distance	= vDot;
		if(distance < m_NearCull)
			return;
		
		F32 drawDistace		= Maths::Min(m_CullDistance, view.GetViewData().farClip);
		F32 nearIntensity	= (distance - m_NearCull) / m_Radius;
			nearIntensity	= Maths::Clamp01(nearIntensity);
		F32 farIntensity	= 1.0f - (distance / drawDistace - fade) / (1.0f - fade);
			farIntensity	= Maths::Clamp01(farIntensity);
		
		F32 depth			= distance * depthScale;

		F32 viewIntensity = 1.f;
		if (m_PerViewIntensities.Size() > 0)
		{
			viewIntensity = m_PerViewIntensities[view.GetType()];
		}

		C_VisiblePointLight visibleLight;
		visibleLight.m_pObject = this;
		visibleLight.m_Intensity = nearIntensity * farIntensity * viewIntensity;
		visibleLight.m_ScreenSize = vScreenSize;
		visibleLight.m_SortKey = (U32)depth;
		
		//Set vis detect
		SetVisibility(view.GetTypeFlag());

		view.AddVisiblePointLight(visibleLight);
	}
}

void C_PointLight::Accept(I_Visitor& visitor) const
{
	visitor.Visit(*this);
}

void C_PointLight::OnGather(const C_View& view) const
{
	Gather(view);
}

void C_PointLight::DebugRender(const C_View& view) const
{
	Maths::HVec4 sphereW(Maths::Load<Maths::HVec4>(&m_BoundingSphere));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(Maths::HVec3(sphereW), sphereW.W(), Maths::RGBA(0xFF0000FF));
}

void C_PointLight::SetDimensions(const Maths::HVec3 position, const F32 radius)
{
	Maths::Store(position, &m_Position);
	m_Radius = radius;
	
	Maths::Store(position, (MathsStorage::Float3*)&m_BoundingSphere);
	m_BoundingSphere.w = radius;
}

C_SpotLight::C_SpotLight()
{
}

C_SpotLight::~C_SpotLight()
{
}

void C_SpotLight::Gather(const C_View& view) const
{
	if ((m_VisibilityFlags & view.GetTypeFlag()) == 0)
		return;

	Maths::HVec3	vView		= view.GetViewSettings3D().GetCameraPosition();
	Maths::HMat44	mProjection	= view.GetViewSettings3D().GetCameraToDevice();
	Maths::HVec4	vOrigin		= Maths::Load<Maths::HVec4>((MathsStorage::Float4*)&m_Matrix.rw);

	Maths::HVec4 vPositionOne(Maths::Load<Maths::HVec4>(&m_BoundingSphere));
	Maths::HVec3 vColour(Maths::Load<Maths::HVec3>(&m_Colour));
	Maths::HF32 vRadius(vPositionOne.W());
	vPositionOne.W(Maths::Constants<Maths::HF32>::One());
	Maths::HF32 vCullDistance(m_CullDistance);
	Maths::HF32 vFarClip(view.GetViewData().farClip);
	Maths::HVec4 vForward(Maths::Load<Maths::HVec4>(&view.GetViewData().forwardZscaled));

	const F32	fade		= g_debugLightBeginFade;
	const F32	nearFade	= g_debugLightNearFade;
	const F32	depthScale	= view.GetViewData().depthScale;
	
	if(Maths::LengthSquared(vColour) > Maths::Constants<Maths::HF32>::Epsilon() && view.GetViewFrustum().CullSphere(vPositionOne, vRadius, Maths::Min(vCullDistance, vFarClip)))
	{
		Maths::HF32 vDot = vPositionOne | vForward;
		Maths::HVec4 vCoord(vRadius, 0.0f, vDot, 1.0f);
		vCoord = vCoord * mProjection;
		Maths::HF32 vScreenSize = Maths::Abs(vCoord.X() / vCoord.W());

		const F32 inFront	= vOrigin | vForward;
		const F32 distance	= vDot;

		F32 nearCull = m_NearCull;
#ifndef CFG_SPEED
		if(g_debugLightNearCullOverride)
			nearCull = g_debugLightNearCull;
#endif

		if(inFront < nearCull)
			return;

		F32 drawDistace		= Maths::Min(m_CullDistance, view.GetViewData().farClip);
		F32 nearIntensity	= (inFront - nearCull) / nearFade;
			nearIntensity	= Maths::Clamp01(nearIntensity);
		F32 farIntensity	= 1.0f - (distance / drawDistace - fade) / (1.0f - fade);
			farIntensity	= Maths::Clamp01(farIntensity);
		
		F32 viewIntensity = 1.f;
		if (m_PerViewIntensities.Size() > 0)
		{
			viewIntensity = m_PerViewIntensities[view.GetType()];
		}

		F32 depth			= distance * depthScale;

		C_VisibleSpotLight visibleLight;
		visibleLight.m_pObject = this;
		visibleLight.m_Intensity = nearIntensity * farIntensity * viewIntensity;
		visibleLight.m_ScreenSize = vScreenSize;
		visibleLight.m_SortKey = (U32)depth;

		//Set vis detect
		SetVisibility(view.GetTypeFlag());

		view.AddVisibleSpotLight(visibleLight);
	}
}

void C_SpotLight::Accept(I_Visitor& visitor) const
{
	visitor.Visit(*this);
}

void C_SpotLight::OnGather(const C_View& view) const
{
	Gather(view);
}

void C_SpotLight::DebugRender(const C_View& view) const
{
	const Maths::HVec4 sphereW(Maths::Load<Maths::HVec4>(&m_BoundingSphere));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(Maths::HVec3(sphereW), sphereW.W(), Maths::RGBA(0xFF00FF00));
}

Maths::C_Sphere GetMinimalEnclosingSphere(const Maths::HVec3& vO, const Maths::HVec3& vA, const Maths::HVec3& vB, const Maths::HVec3& vC)
{
	const Maths::HVec3 va = vA - vO;
	const Maths::HVec3 vb = vB - vO;
	const Maths::HVec3 vc = vC - vO;

	const Maths::HVec3 vaCrossb = va ^ vb;
	const Maths::HVec3 vaCrossc = va ^ vc;

	const Maths::HF32 vDenominator0 = Maths::Constants<Maths::HF32>::Two() * vaCrossb.Length2();
	const Maths::HF32 vDenominator1 = Maths::Constants<Maths::HF32>::Two() * vaCrossc.Length2();

	const Maths::HVec3 vo0 = ((vaCrossb ^ va) * vb.Length2() + (vb ^ vaCrossb) * va.Length2()) * vDenominator0.Rcp();
	const Maths::HVec3 vo1 = ((vaCrossc ^ va) * vc.Length2() + (vc ^ vaCrossc) * va.Length2()) * vDenominator1.Rcp();

	const Maths::HF32 vRadius0 = vo0.Length();
	const Maths::HF32 vRadius1 = vo1.Length();
	const Maths::HVec3 vCentre0 = vo0 + vO;
	const Maths::HVec3 vCentre1 = vo1 + vO;

	const Maths::HVec3 vD = vCentre1 - vCentre0;
	const Maths::HF32 vd = Maths::Length(vD);

	const Maths::HF32 vRadius = 0.5f * (vd + vRadius0 + vRadius1);
	const Maths::HVec3 vCentre = vCentre0 + 0.5f * (vd + vRadius0 - vRadius1) * vD * vd.Rcp();

	return Maths::C_Sphere(vCentre, vRadius);
}

void C_SpotLight::SetDimensions(const Maths::HMat43& worldMatrix, const F32 radius, const F32 horizontalFoV, const F32 verticalFoV)
{
	Maths::Store(worldMatrix.AW(), &m_Position);
	Maths::Store(Maths::HMat44(worldMatrix), &m_Matrix);
	m_Radius = radius;
	m_HorizontalFoV = horizontalFoV;
	m_VerticalFoV = verticalFoV;

	Maths::HVec3	vPosition	= Maths::Load<Maths::HVec3>(&m_Position);
	Maths::HMat43	mLight		= (Maths::HMat43) Maths::Load<Maths::HMat44>(&m_Matrix);

	Maths::HF32		fRadius		= Maths::Load<Maths::HF32>(&m_Radius);
	Maths::HF32		fAngleX		= Maths::Load<Maths::HF32>(&m_HorizontalFoV);
	Maths::HF32		fAngleY		= Maths::Load<Maths::HF32>(&m_VerticalFoV);
	Maths::HF32		fSizeX		= fRadius * Maths::Tan(fAngleX);
	Maths::HF32		fSizeY		= fRadius * Maths::Tan(fAngleY);
	Maths::HF32		fZero		= Maths::Constants<Maths::HF32>::Zero();

	Maths::HVec3	v0			= Maths::HVec3(fZero, fZero, fZero) * mLight;
	Maths::HVec3	v1			= Maths::HVec3(-fSizeX, -fSizeY, fRadius) * mLight;
	Maths::HVec3	v2			= Maths::HVec3(-fSizeX, +fSizeY, fRadius) * mLight;
	Maths::HVec3	v3			= Maths::HVec3(+fSizeX, +fSizeY, fRadius) * mLight;

	Maths::C_Sphere	vSphere		= GetMinimalEnclosingSphere(v0, v1, v2, v3);

	Maths::Store(vSphere.m_V, &m_BoundingSphere);
}

C_DropShadow::C_DropShadow() : m_Salt(0), m_IsManaged(true), m_CullDistance(1000.0f), m_CullScreenSize(0.01f)
{
}

C_DropShadow::~C_DropShadow()
{
}

void C_DropShadow::Gather(const C_View& view) const
{
	Maths::HVec3	vView		= view.GetViewSettings3D().GetCameraPosition();
	Maths::HMat44	mProjection	= view.GetViewSettings3D().GetCameraToDevice();

	Maths::HVec4 vPositionOne(Maths::Load<Maths::HVec4>(&m_BoundingSphere));
	Maths::HF32 vRadius(vPositionOne.W());
	vPositionOne.W(Maths::Constants<Maths::HF32>::One());
	Maths::HF32 vCullDistance(m_CullDistance);
	Maths::HF32 vFarClip(view.GetViewData().farClip);
	Maths::HVec4 vForward(Maths::Load<Maths::HVec4>(&view.GetViewData().forwardZscaled));

	const F32	fade		= g_debugLightBeginFade;
	const F32	depthScale	= view.GetViewData().depthScale;
	
	if(view.GetViewFrustum().CullSphere(vPositionOne, vRadius, Maths::Min(vCullDistance, vFarClip)))
	{
		Maths::HF32 vDot = vPositionOne | vForward;
		Maths::HVec4 vCoord(vRadius, 0.0f, vDot, 1.0f);
		vCoord = vCoord * mProjection;
		Maths::HF32 vScreenSize = Maths::Abs(vCoord.X() / vCoord.W());

		if (vScreenSize > m_CullScreenSize)
		{
			F32 drawDistance = Maths::Min(m_CullDistance, view.GetViewData().farClip);
			F32 distance	= vDot;
			F32 depth		= distance * depthScale;
			F32 intensityDistant = 1.0f - (distance / drawDistance - fade) / (1.0f - fade);
			F32 intensitySize = 1.f - (m_CullScreenSize/F32(vScreenSize) - fade) / (1.0f - fade);;

			F32 intensity	= Maths::Clamp01(Maths::Min(intensityDistant, intensitySize));

			C_VisibleDropShadow visibleLight;
			visibleLight.m_pObject = this;
			visibleLight.m_Intensity = intensity;
			visibleLight.m_ScreenSize = vScreenSize;
			visibleLight.m_SortKey = (U32)depth;

			view.AddVisibleDropShadow(visibleLight);
		}
	}
}

void C_DropShadow::Accept(I_Visitor& visitor) const
{
	visitor.Visit(*this);
}

void C_DropShadow::OnGather(const C_View& view) const
{
	Gather(view);
}

void C_DropShadow::SetDimensions(const Maths::HVec3 position, const F32 radius)
{
	Maths::Store(position, &m_Position);
	m_Radius = radius;

	Maths::Store(position, (MathsStorage::Float3*)&m_BoundingSphere);
	m_BoundingSphere.w = radius;
}


} // namespace SceneGraph
