#include "stdafx.h"
#include "Render/Shadow/ShadowMap.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

namespace Render
{
namespace Shadow
{

INLINE DWORD FtoDW(FLOAT f) { return *((DWORD*)&f); }

::Debug::Bool g_debugUseDepthStencilTexture				("Graphics.Render.ShadowMap.UseDepthStencilTexture",		true);

::Debug::Float g_debugShadowMapNearClip					("Graphics.Render.ShadowMap.NearClip",					1.f,		::Debug::Range(0.0f, 10.0f));
::Debug::Float g_debugShadowMapFarClip					("Graphics.Render.ShadowMap.FarClip",					1.2f,		::Debug::Range(0.0f, 10.0f));
::Debug::Float g_debugShadowMapViewSize					("Graphics.Render.ShadowMap.ViewSize",					1.f,		::Debug::Range(0.0f, 10.0f));

::Debug::Bool  g_debugShadowMapTrapezoidForceOn			("Graphics.Render.ShadowMap.Trapezoid.ForceOn",			false);
::Debug::Bool  g_debugShadowMapTrapezoidForceOff			("Graphics.Render.ShadowMap.Trapezoid.ForceOff",			false);
::Debug::Float g_debugShadowMapTrapezoidDelta				("Graphics.Render.ShadowMap.Trapezoid.Delta",			0.52f,		::Debug::Range(0.0f, 1.0f));
::Debug::Float g_debugShadowMapTrapezoidXi				("Graphics.Render.ShadowMap.Trapezoid.Xi",				-0.60f,		::Debug::Range(-1.0f, 1.0f));
::Debug::Float g_debugShadowMapTrapezoidLightNearOffset	("Graphics.Render.ShadowMap.Trapezoid.LightNearOffset",	-10.0f,		::Debug::Range(-10.0f, 10.0f));
::Debug::Float g_debugShadowMapTrapezoidLightFarOffset	("Graphics.Render.ShadowMap.Trapezoid.LightFarOffset",	0.0f,		::Debug::Range(0.0f, 200.0f));
::Debug::Float g_debugShadowMapTrapezoidNearClip			("Graphics.Render.ShadowMap.Trapezoid.NearClip",			0.10f,		::Debug::Range(0.0f, 1.0f));
::Debug::Float g_debugShadowMapTrapezoidFarClipNearest	("Graphics.Render.ShadowMap.Trapezoid.FarClipNearest",	12.0f,		::Debug::Range(0.0f, 100.0f));
::Debug::Float g_debugShadowMapTrapezoidFarClipFurthest	("Graphics.Render.ShadowMap.Trapezoid.FarClipFurthest",	40.0f,		::Debug::Range(0.0f, 100.0f));
::Debug::Float g_debugShadowMapTrapezoidFarClipPower		("Graphics.Render.ShadowMap.Trapezoid.FarClipPower",		4.0f,		::Debug::Range(0.0f, 64.0f));

// C_ShadowMapSetup

C_ShadowMapSetup::C_ShadowMapSetup() :
	m_NearClip(0.0f),
	m_FarClip(0.0f),
	m_ViewSize(0.0f),
	m_MatrixType(E_ShadowMapMatrixType_Standard)
{
}

C_ShadowMapSetup::~C_ShadowMapSetup()
{
}

// C_ShadowMap

C_ShadowMap::C_ShadowMap() : Display::I_Renderer("ShadowMap"),
	m_Bound(false),
	m_Width(0),
	m_Height(0),
	m_Flags(0),
	m_Levels(0),
	m_ColourFormat(Display::E_Format_Unknown),
	m_DepthStencilFormat(Display::E_Format_Unknown)
{
	m_ViewSetup.m_ViewSetup3D.m_PerspDist = 0.f;
	m_View.m_Type = Display::E_ViewType_3D;
}

C_ShadowMap::~C_ShadowMap()
{
	DestroyRenderer();
}

void C_ShadowMap::Initialise(const U32 width, const U32 height, const U32 flags)
{
	m_Width = m_Viewport.m_W = width;
	m_Height = m_Viewport.m_H = height;
	m_Flags = flags;
}

bool C_ShadowMap::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("ShadowMap");

	Display::C_SurfaceParams surfaceParams;
	
	U32 depthStencilFlags = m_Flags | Display::E_Usage_DepthStencil;
	if(!Display::Device()->CreateDepthStencilSurface(m_Width, m_Height, depthStencilFlags, m_DepthStencilFormat, Display::E_Multisample_None, &surfaceParams, &m_DepthStencilSurface))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetSurfaceName(&m_DepthStencilSurface, "_ShadowMapDS");
	#endif

	#if defined(PLATFORM_PS3) && defined(USE_HARDWARESHADOWMAP)
	CellGcmTexture* pGcmTexture = m_RenderTexture.GetTexture().GetGPUTexture();
	pGcmTexture->format 		= CELL_GCM_TEXTURE_DEPTH24_D8 | CELL_GCM_TEXTURE_LN;
	pGcmTexture->mipmap 		= 1;
	pGcmTexture->dimension		= CELL_GCM_TEXTURE_DIMENSION_2;
	pGcmTexture->cubemap		= 0;
	pGcmTexture->remap			=	CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
									CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
									CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
									CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
									CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
									CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
									CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
									CELL_GCM_TEXTURE_REMAP_FROM_A;
	pGcmTexture->width			= m_Width;
	pGcmTexture->height			= m_Height;
	pGcmTexture->depth			= 1;
	pGcmTexture->location		= CELL_GCM_LOCATION_LOCAL;
	pGcmTexture->pitch			= m_DepthStencilSurface.GetPitch();
	pGcmTexture->offset			= m_DepthStencilSurface.GetOffset();
	#else
	U32 colourFlags = m_Flags | Display::E_Usage_Colour;
	if(!Display::Device()->CreateRenderTexture2D(m_Width, m_Height, m_Levels, colourFlags, m_ColourFormat, Display::E_Multisample_None, NULL, &m_RenderTexture))
		return false;
	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_RenderTexture, "_ShadowMapRT");
	#endif
	#endif

	return true;
}

void C_ShadowMap::OnDestroyVolatile()
{
	#if defined(PLATFORM_PS3) && defined(USE_HARDWARESHADOWMAP)
	Display::Device()->Destroy(&m_RenderTexture);
	#endif
	Display::Device()->Destroy(&m_DepthStencilSurface);
}

void C_ShadowMap::Bind()
{
	assert(!m_Bound);


	
	#ifdef PLATFORM_PS3
	cellGcmSetPolygonOffsetFillEnable(CELL_GCM_TRUE);
	cellGcmSetPolygonOffset(m_Setup.m_PolygonOffsetFactor, m_Setup.m_PolygonOffsetUnits);
	#endif

	#ifdef USE_HARDWARESHADOWMAP
	Display::Device()->SetColourDepthStencilSurface(0, &m_DepthStencilSurface);	
	#else
	Display::Device()->SetColourDepthStencilSurface(&m_RenderTexture.GetSurface(), &m_DepthStencilSurface);
	#endif	

	#ifdef USE_HARDWARESHADOWMAP
	Display::Device()->SetColourWriteEnable(0);
	#endif

	Display::Device()->SetView(&m_View);
	Display::Device()->SetViewport(m_Viewport);

	#ifdef USE_HARDWARESHADOWMAP
	Display::Device()->Clear(Display::E_Clear_Depth);
	#else
	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(255, 255, 0, 0));
	#endif

	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);

	if (m_Setup.m_MatrixType == E_ShadowMapMatrixType_Trapezoid)
	{
		Display::Device()->SetCullModeFlip();
	}

	m_Bound = true;
}

void C_ShadowMap::Unbind()
{
	assert(m_Bound);


	#ifdef PLATFORM_PS3
	cellGcmSetPolygonOffsetFillEnable(CELL_GCM_FALSE);
	#endif

	Display::Device()->SetView(NULL, false);

	Display::Device()->SetColourDepthStencilSurface();

	#ifdef USE_HARDWARESHADOWMAP
	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);
	#endif


	if (m_Setup.m_MatrixType == E_ShadowMapMatrixType_Trapezoid)
	{
		Display::Device()->SetCullModeFlip();
	}
	
	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	
	m_Bound = false;
}

void C_ShadowMap::SetupView(const C_ShadowMapSetup& setup)
{
	m_Setup = setup;
	
	#ifndef CFG_SPEED
	if(g_debugShadowMapTrapezoidForceOn)
		m_Setup.m_MatrixType = E_ShadowMapMatrixType_Trapezoid;
	else if(g_debugShadowMapTrapezoidForceOff)
		m_Setup.m_MatrixType = E_ShadowMapMatrixType_Standard;
	#endif

	if(m_Setup.m_MatrixType == E_ShadowMapMatrixType_Trapezoid)
	{
		SetupTrapezoidView(m_Setup);
	}
	else
	{
		SetupStandardView(m_Setup);
	}
}

void C_ShadowMap::SetupStandardView(const C_ShadowMapSetup& setup)
{
	Maths::HVec3 az;
	az = setup.m_Direction;
	az.Normalise();

	Maths::HVec3 ax;
	ax = Maths::HVec3(0.f, 1.f, 0.f) ^ az;
	ax.Normalise();

	Maths::HVec3 ay;
	ay = az ^ ax;
	ay.Normalise();

	Maths::HMat33 rot;
	rot.AX(ax);
	rot.AY(ay);
	rot.AZ(az);

	Maths::HMat43 matCameraToWorld(rot, setup.m_Position);

	const F32 nearClip = setup.m_NearClip * g_debugShadowMapNearClip;
	const F32 farClip = setup.m_FarClip * g_debugShadowMapFarClip;
	const F32 viewSize = setup.m_ViewSize * g_debugShadowMapViewSize;

	m_ViewSetup.m_ViewSetup3D.m_NearClip = nearClip;
	m_ViewSetup.m_ViewSetup3D.m_FarClip = farClip;
	m_ViewSetup.m_ViewSetup3D.m_ViewExtents.SetBounds(-viewSize, -viewSize, viewSize, viewSize);

	m_ViewSetup.m_ViewSetup3D.m_CameraToWorld = matCameraToWorld;

	Display::Device()->SetupView(&m_View, m_ViewSetup, m_Width, m_Height);

	m_LightViewMatrix = m_View.m_ViewSettings3D.m_WorldToCamera;
	m_LightProjMatrix = m_View.m_ViewSettings3D.m_CameraToDevice;
}

void C_ShadowMap::SetupTrapezoidView(const C_ShadowMapSetup& setup)
{
	Maths::HF32 vZero( Maths::Constants<Maths::HF32>::Zero() );
	Maths::HF32 vQuarter( Maths::Constants<Maths::HF32>::Quarter() );
	Maths::HF32 vHalf( Maths::Constants<Maths::HF32>::Half() );
	Maths::HF32 vOne( Maths::Constants<Maths::HF32>::One() );
	Maths::HF32 vTwo( Maths::Constants<Maths::HF32>::Two() );

	// get light basis
	Maths::HVec3 az;
	az = setup.m_Direction * Maths::HMat33(setup.m_EyeViewMatrix);
	az.Normalise();

	Maths::HVec3 ax;
	ax = Maths::HVec3(vZero, vZero, -vOne) ^ az;
	ax.Normalise();

	Maths::HVec3 ay;
	ay = az ^ ax;
	ay.Normalise();

	Maths::HMat33 matLightBasis;
	matLightBasis.AX(ax);
	matLightBasis.AY(ay);
	matLightBasis.AZ(az);
	matLightBasis.Transpose();

	// calculate clip planes
	F32 nearClip = g_debugShadowMapTrapezoidNearClip;
	F32 farClip = g_debugShadowMapTrapezoidFarClipFurthest;
	{
		F32 cosTheta = az.Z();
		F32 t = powf(cosTheta * 0.5f + 0.5f, g_debugShadowMapTrapezoidFarClipPower);

		farClip = g_debugShadowMapTrapezoidFarClipFurthest + t * (g_debugShadowMapTrapezoidFarClipNearest - g_debugShadowMapTrapezoidFarClipFurthest);
	}

	// get view frustum
	Maths::HVec3 vFrustumPoints[8];
	{
		const Maths::HF32 vNearClip = nearClip;
		const Maths::HF32 vFarClip = farClip;
	
		const Maths::C_Rect viewExtents3D = setup.m_ViewExtents;
		Maths::HVec2 vClipMin( viewExtents3D.Min() );
		Maths::HVec2 vClipMax( viewExtents3D.Max() );
		Maths::HVec2 vNearClipMin = vClipMin * vNearClip / setup.m_PerspDist;
		Maths::HVec2 vNearClipMax = vClipMax * vNearClip / setup.m_PerspDist;
		Maths::HVec2 vFarClipMin = vClipMin * vFarClip / setup.m_PerspDist;
		Maths::HVec2 vFarClipMax = vClipMax * vFarClip / setup.m_PerspDist;
	
		vFrustumPoints[0] = Maths::HVec3(vNearClipMin.X(), vNearClipMin.Y(), vNearClip);
		vFrustumPoints[1] = Maths::HVec3(vNearClipMin.X(), vNearClipMax.Y(), vNearClip);
		vFrustumPoints[2] = Maths::HVec3(vNearClipMax.X(), vNearClipMax.Y(), vNearClip);
		vFrustumPoints[3] = Maths::HVec3(vNearClipMax.X(), vNearClipMin.Y(), vNearClip);
		vFrustumPoints[4] = Maths::HVec3(vFarClipMin.X(), vFarClipMin.Y(), vFarClip);
		vFrustumPoints[5] = Maths::HVec3(vFarClipMin.X(), vFarClipMax.Y(), vFarClip);
		vFrustumPoints[6] = Maths::HVec3(vFarClipMax.X(), vFarClipMax.Y(), vFarClip);
		vFrustumPoints[7] = Maths::HVec3(vFarClipMax.X(), vFarClipMin.Y(), vFarClip);
	}	

	// rotate into light space
	Maths::HF32 vMinX(FLT_MAX), vMaxX(-FLT_MAX);
	Maths::HF32 vMinY(FLT_MAX), vMaxY(-FLT_MAX);
	Maths::HF32 vMinZ(FLT_MAX), vMaxZ(-FLT_MAX);
	for(U32 pointNo = 0; pointNo < 8; ++pointNo)
	{
		vFrustumPoints[pointNo] = vFrustumPoints[pointNo] * matLightBasis;

		vMinX = Maths::Min(vMinX, vFrustumPoints[pointNo].X());
		vMaxX = Maths::Max(vMaxX, vFrustumPoints[pointNo].X());
		vMinY = Maths::Min(vMinY, vFrustumPoints[pointNo].Y());
		vMaxY = Maths::Max(vMaxY, vFrustumPoints[pointNo].Y());
		vMinZ = Maths::Min(vMinZ, vFrustumPoints[pointNo].Z());
		vMaxZ = Maths::Max(vMaxZ, vFrustumPoints[pointNo].Z());
	}

	Maths::HVec3 vCameraPosition = setup.m_Position * Maths::HMat43(setup.m_EyeViewMatrix) * matLightBasis;
	Maths::HF32 vLightMinZ = Maths::Min(vMinZ, vCameraPosition.Z() + g_debugShadowMapTrapezoidLightNearOffset);
	Maths::HF32 vLightMaxZ = Maths::Max(vMaxZ, vCameraPosition.Z() + g_debugShadowMapTrapezoidLightFarOffset);

	Maths::HVec3 vLightTranslate(vZero, vZero, vOne - vLightMinZ);
	if(vMinZ < vZero)
	{
		Maths::HMat43 matLightTranslate(Maths::HMat33(), vLightTranslate);
		vLightMaxZ = -vMinZ + vMaxZ + vOne;
		vLightMinZ = vOne;
		for(U32 pointNo = 0; pointNo < 8; ++pointNo)
		{
			vFrustumPoints[pointNo] = vFrustumPoints[pointNo] * matLightTranslate;
			vMinZ = Maths::Min(vMinZ, vFrustumPoints[pointNo].Z());
			vMaxZ = Maths::Min(vMaxZ, vFrustumPoints[pointNo].Z());
		}
	}
	
	Maths::HMat43 matLightView(matLightBasis, vLightTranslate);
	
	// build light space ortho matrix
	Maths::HVec4 matLightProjX(vTwo * (vMaxX - vOne).Rcp(), vZero, vZero, vZero);
	Maths::HVec4 matLightProjY(vZero, vTwo * (vMaxY - vMinY).Rcp(), vZero, vZero);
	Maths::HVec4 matLightProjZ(vZero, vZero, vOne * (vLightMaxZ - vLightMinZ).Rcp(), vZero);
	Maths::HVec4 matLightProjW((vOne + vMaxX) * (vOne - vMaxX).Rcp(), (vMaxY + vMinY) * (vMinY - vMaxY).Rcp(), vLightMinZ * (vLightMinZ - vLightMaxZ).Rcp(), vOne);
	Maths::HMat44 matLightProj;
	matLightProj.AX(matLightProjX);
	matLightProj.AY(matLightProjY);
	matLightProj.AZ(matLightProjZ);
	matLightProj.AW(matLightProjW);

	for(U32 pointNo = 0; pointNo < 8; ++pointNo)
	{
		vFrustumPoints[pointNo] = (Maths::HVec3) Maths::HVec4(vFrustumPoints[pointNo]) * matLightProj;
	}

	Maths::HVec2 vFarCentre( vFrustumPoints[0] + vFrustumPoints[1] + vFrustumPoints[2] + vFrustumPoints[3] );
	Maths::HVec2 vNearCentre( vFrustumPoints[4] + vFrustumPoints[5] + vFrustumPoints[6] + vFrustumPoints[7] );
	vNearCentre *= vQuarter;
	vFarCentre *= vQuarter;
	
	Maths::HVec2 vCentreOrig = (vNearCentre + vFarCentre) * vHalf;

	Maths::HVec4 vTranslateCentreW(-vCentreOrig.X(), -vCentreOrig.Y(), vZero, vOne);
	Maths::HMat44 matTranslateCentre;
	matTranslateCentre.AW(vTranslateCentreW);

	Maths::HVec2 vLength = vFarCentre - vCentreOrig;
	Maths::HF32 vHalfCentreLength = vLength.Length();
	Maths::HF32 vHalfCentreRcpLength = vHalfCentreLength.Rcp();

	Maths::HVec2 vCosSinTheta = vLength * vHalfCentreRcpLength;

	Maths::HMat44 matRotCentre;
	Maths::HVec4 vRotCentreX(vCosSinTheta.X(), -vCosSinTheta.Y(), vZero, vZero);
	Maths::HVec4 vRotCentreY(vCosSinTheta.Y(), vCosSinTheta.X(), vZero, vZero);
	matRotCentre.AX(vRotCentreX);
	matRotCentre.AY(vRotCentreY);

	Maths::HMat44 matTrapezoidSpace = matTranslateCentre * matRotCentre;
	vMinX = FLT_MAX;
	vMinY = FLT_MAX;
	vMaxX = -FLT_MAX;
	vMaxY = -FLT_MAX;
	for(U32 pointNo = 0; pointNo < 8; ++pointNo)
	{
		vFrustumPoints[pointNo] = vFrustumPoints[pointNo] * Maths::HMat43(matTrapezoidSpace);
		vMinX = Maths::Min(vMinX, vFrustumPoints[pointNo].X());		
		vMinY = Maths::Min(vMinY, vFrustumPoints[pointNo].Y());
		vMaxX = Maths::Max(vMaxX, vFrustumPoints[pointNo].X());
		vMaxY = Maths::Max(vMaxY, vFrustumPoints[pointNo].Y());
	}

	Maths::HVec4 vXScale = Maths::HVec4( Maths::Max( vMaxX.Abs(), vMinX.Abs() ).Rcp(), vZero, vZero, vZero );
	Maths::HVec4 vYScale = Maths::HVec4( vZero, Maths::Max( vMaxY.Abs(), vMinY.Abs() ).Rcp(), vZero, vZero );

	Maths::HMat44 matScaleCentre;
	matScaleCentre.AX(vXScale);
	matScaleCentre.AY(vYScale);

	matTrapezoidSpace = matTrapezoidSpace * matScaleCentre;

	vMinX *= vXScale.X();
	vMaxX *= vXScale.X();
	vMinY *= vYScale.Y();
	vMaxY *= vYScale.Y();

	Maths::HF32 vLambda = vMaxX - vMinX;
	Maths::HF32 vDeltaProj = g_debugShadowMapTrapezoidDelta * vLambda;
	Maths::HF32 vXi = (F32) g_debugShadowMapTrapezoidXi;
	Maths::HF32 vEta = vLambda * vDeltaProj * (vOne + vXi) * (vLambda * (vOne - vXi) - vTwo * vDeltaProj).Rcp();

	Maths::HVec2 vProjectionQ(vMaxX + vEta, vZero);
	Maths::HF32 vMaxSlope = -FLT_MAX;
	Maths::HF32 vMinSlope = FLT_MAX;
	
	for(U32 pointNo = 0; pointNo < 8; ++pointNo)
	{
		Maths::HVec2 v( vFrustumPoints[pointNo].X() * vXScale.X(), vFrustumPoints[pointNo].Y() * vYScale.Y() );
		Maths::HF32 vXDist = v.X() - vProjectionQ.X();
		Maths::Scalar_Bool select0 = vXDist.Abs().LT(Maths::Constants<Maths::HF32>::Epsilon());
		Maths::Scalar_Bool select1 = v.Y().Abs().LT(Maths::Constants<Maths::HF32>::Epsilon());
		if( !(select0 || select1) )
		{
			vMaxSlope = Maths::Max(vMaxSlope, v.Y() * vXDist.Rcp());
			vMinSlope = Maths::Min(vMinSlope, v.Y() * vXDist.Rcp());
		}
	}

	Maths::HF32 vXNear = vEta;
	Maths::HF32 vXFar = vLambda + vEta;

	Maths::HVec4 vQTranslateX( -vOne, vZero, vZero, vZero );
	Maths::HVec4 vQTranslateW( vProjectionQ.X(), vZero, vZero, vOne );
	Maths::HMat44 matQTranslate;
	matQTranslate.AX(vQTranslateX);
	matQTranslate.AW(vQTranslateW);

	matTrapezoidSpace = matTrapezoidSpace * matQTranslate;

	Maths::HF32 vShearAmt = (vMinSlope.Abs() - vMaxSlope) * vHalf;
	vMaxSlope = vMaxSlope + vShearAmt;

	Maths::HVec4 vTrapezoidShearX(vOne, vShearAmt, vZero, vZero);
	Maths::HMat44 matTrapezoidShear;
	matTrapezoidShear.AX(vTrapezoidShearX);

	matTrapezoidSpace = matTrapezoidSpace * matTrapezoidShear;

	Maths::HF32 vZAspect = (vMaxZ - vMinZ) * (vMaxY - vMinY).Rcp();

	Maths::HVec4 vTrapezoidProjX( vXFar * (vXFar - vXNear).Rcp(), vZero, vZero, vOne );
	Maths::HVec4 vTrapezoidProjY( vZero, vMaxSlope.Rcp(), vZero, vZero );
	Maths::HVec4 vTrapezoidProjZ( vZero, vZero, (vZAspect * vMaxSlope).Rcp(), vZero );
	Maths::HVec4 vTrapezoidProjW( -vXNear * vXFar * (vXFar - vXNear).Rcp(), vZero, vZero, vZero );
	Maths::HMat44 matTrapezoidProj(vTrapezoidProjX, vTrapezoidProjY, vTrapezoidProjZ, vTrapezoidProjW);

	matTrapezoidSpace = matTrapezoidSpace * matTrapezoidProj;	

	Maths::HVec4 vBiasedScaleX(vTwo, vZero, vZero, vZero);
	Maths::HVec4 vBiasedScaleW(-vOne, vZero, vZero, vOne);
	Maths::HMat44 matBiasedScale;
	matBiasedScale.AX(vBiasedScaleX);
	matBiasedScale.AW(vBiasedScaleW);

	matTrapezoidSpace = matTrapezoidSpace * matBiasedScale;

	m_View.m_ViewSettings3D.m_CameraToDevice = matLightProj * matTrapezoidSpace;
	m_View.m_ViewSettings3D.m_WorldToCamera = Maths::HMat43(setup.m_EyeViewMatrix) * matLightView;
	m_View.m_ViewSettings3D.m_WorldToDevice = Maths::HMat44(m_View.m_ViewSettings3D.m_WorldToCamera) * m_View.m_ViewSettings3D.m_CameraToDevice;
	m_View.m_ViewSettings3D.m_CameraToWorld = m_View.m_ViewSettings3D.m_WorldToCamera.InverseRotTrans();

	Maths::Store(m_View.m_ViewSettings3D.m_CameraToDevice, &m_View.m_ViewSettings3D.m_ProjMatrix);
	Maths::Store(Maths::HMat44(m_View.m_ViewSettings3D.m_WorldToCamera), &m_View.m_ViewSettings3D.m_ViewMatrix);
	Maths::Store(m_View.m_ViewSettings3D.m_WorldToDevice, &m_View.m_ViewSettings3D.m_ViewProjMatrix);

	m_LightViewMatrix = m_View.m_ViewSettings3D.m_WorldToCamera;
	m_LightProjMatrix = matLightProj;
}

} // namespace Shadow
} // namespace Render
