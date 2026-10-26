#include "stdafx.h"
#include "Render/Deferred/DropShadows.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Core/Maths/HVec/New/IncludeMinimal.h"
#include "Core/Maths/HVec/New/IncludeMinimal.inl"
#include "Display/Buffer/VertexDeclarations.h"
#include "System/debug/I_DebugRender.h"

EXTERN_SHADER(vsDropShadows);
EXTERN_SHADER(psDropShadows1);
EXTERN_SHADER(psDropShadows2);
EXTERN_SHADER(psDropShadows3);
EXTERN_SHADER(psDropShadows4);
EXTERN_SHADER(psDropShadows5);
EXTERN_SHADER(psDropShadows6);
EXTERN_SHADER(psDropShadows7);
EXTERN_SHADER(psDropShadows8);
EXTERN_SHADER(psDropShadows1HDR);
EXTERN_SHADER(psDropShadows2HDR);
EXTERN_SHADER(psDropShadows3HDR);
EXTERN_SHADER(psDropShadows4HDR);
EXTERN_SHADER(psDropShadows5HDR);
EXTERN_SHADER(psDropShadows6HDR);
EXTERN_SHADER(psDropShadows7HDR);
EXTERN_SHADER(psDropShadows8HDR);

Debug::Bool gDebugShadowDropShadows("Graphics.Render.Deferred.ShowDropShadows", false);
Debug::Bool gDebugShadowDropShadowTiles("Graphics.Render.Deferred.ShowDropShadowTiles", false);

Debug::Float gDebugShadowDropShadowsMinSize("Graphics.Render.Deferred.DropShadowsMinSize", 0.00f);
Debug::Float gDebugShadowDropShadowsMaxDistant("Graphics.Render.Deferred.DropShadowsMaxDistant", 100.f);

namespace Render
{
namespace Deferred
{

const U32 c_MaximumShadows = 8;

C_DropShadows::C_DropShadows() : Display::I_Renderer("DeferredDropShadows")
{
	m_pDepthTexture = 0;
	m_pNormalTexture = 0;

	m_HorizontalTiles = 8;
	m_VerticalTiles = 4;
}

C_DropShadows::~C_DropShadows()
{
	DestroyRenderer();
}

bool C_DropShadows::OnCreate()
{
	RESOURCE_MEMORY_ID("DropShadows");

	if (!Display::CreateVertexShaderFromFunc("Shader/vsDropShadows", 0, &m_VertexShader, REF_SHADER(vsDropShadows)))
		return false;

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows1HDR", 0, &m_PixelShader[E_DropShadowShader_1], REF_SHADER(psDropShadows1HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows2HDR", 0, &m_PixelShader[E_DropShadowShader_2], REF_SHADER(psDropShadows2HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows3HDR", 0, &m_PixelShader[E_DropShadowShader_3], REF_SHADER(psDropShadows3HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows4HDR", 0, &m_PixelShader[E_DropShadowShader_4], REF_SHADER(psDropShadows4HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows5HDR", 0, &m_PixelShader[E_DropShadowShader_5], REF_SHADER(psDropShadows5HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows6HDR", 0, &m_PixelShader[E_DropShadowShader_6], REF_SHADER(psDropShadows6HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows7HDR", 0, &m_PixelShader[E_DropShadowShader_7], REF_SHADER(psDropShadows7HDR)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows8HDR", 0, &m_PixelShader[E_DropShadowShader_8], REF_SHADER(psDropShadows8HDR)))
			return false;
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows1", 0, &m_PixelShader[E_DropShadowShader_1], REF_SHADER(psDropShadows1)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows2", 0, &m_PixelShader[E_DropShadowShader_2], REF_SHADER(psDropShadows2)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows3", 0, &m_PixelShader[E_DropShadowShader_3], REF_SHADER(psDropShadows3)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows4", 0, &m_PixelShader[E_DropShadowShader_4], REF_SHADER(psDropShadows4)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows5", 0, &m_PixelShader[E_DropShadowShader_5], REF_SHADER(psDropShadows5)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows6", 0, &m_PixelShader[E_DropShadowShader_6], REF_SHADER(psDropShadows6)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows7", 0, &m_PixelShader[E_DropShadowShader_7], REF_SHADER(psDropShadows7)))
			return false;
		if (!Display::CreatePixelShaderFromFunc("Shader/psDropShadows8", 0, &m_PixelShader[E_DropShadowShader_8], REF_SHADER(psDropShadows8)))
			return false;
	}

	for (U32 shaderNo = 0; shaderNo < E_DropShadowShader_Count; ++shaderNo)
	{
		m_PositionParameter[shaderNo]	= m_PixelShader[shaderNo].GetParameterArray(CP_RENDER_SETTINGS, "g_vPosition", 8);
		m_IntensityParameter[shaderNo]	= m_PixelShader[shaderNo].GetParameterArray(CP_RENDER_SETTINGS_8, "g_vIntensity", 8);
		m_DepthParameter[shaderNo]		= m_PixelShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_20, "g_vDepth");
		m_LuminanceParameter[shaderNo]	= m_PixelShader[shaderNo].GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE,	"g_vLuminance");
		m_ViewUpVectorParameter[shaderNo]	= m_PixelShader[shaderNo].GetParameter(CP_RENDER_SETTINGS_21,	"g_vViewUpVector");
	}

	if (!Display::Device()->CreateVertexBuffer(sizeof(Display::Vertex_XY) * m_VerticalTiles * m_HorizontalTiles * 4, 0, &m_VertexBuffer))
		return false;

	Display::Vertex_XY* pVertex = (Display::Vertex_XY*)m_VertexBuffer.Lock();
	if (pVertex)
	{
		const F32 fDX = 2.0f / (F32) m_HorizontalTiles;
		const F32 fDY = 2.0f / (F32) m_VerticalTiles;
		for (U32 y = 0; y < m_VerticalTiles; ++y)
		{
			for (U32 x = 0; x < m_HorizontalTiles; ++x)
			{
				const F32 fX = x * fDX - 1.0f;
				const F32 fY = y * fDY - 1.0f;

				pVertex->x = fX;
				pVertex->y = fY;
				pVertex++;

				pVertex->x = fX;
				pVertex->y = fY + fDY;
				pVertex++;

				pVertex->x = fX + fDX;
				pVertex->y = fY;
				pVertex++;

				pVertex->x = fX + fDX;
				pVertex->y = fY + fDY;
				pVertex++;
			}
		}

		m_VertexBuffer.Unlock();
	}

	return true;
}

bool C_DropShadows::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("DropShadows");

	MEMORY_TRACKER_PUSH("graphics.renderers.dropshadows");

	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();
	
	const U32 iWidth = backBuffer.GetWidth();
	const U32 iHeight = backBuffer.GetHeight();
	const Display::E_Format eFormat = backBuffer.GetColourFormat();

	if (!Display::Device()->CreateRenderTexture2D(iWidth, iHeight, 1, 0, eFormat, Display::E_Multisample_None, 0, &m_RenderTexture))
	{
		MEMORY_TRACKER_POP("graphics.renderers.dropshadows");
		return false;
	}
	
	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetRenderTexture2DName(&m_RenderTexture, "_DropShadowsRT");
	#endif

	MEMORY_TRACKER_POP("graphics.renderers.dropshadows");

	return true;
}

void C_DropShadows::OnDestroy()
{
	for (U32 shaderNo = 0; shaderNo < E_DropShadowShader_Count; ++shaderNo)
	{
		Display::Device()->Destroy(&m_PixelShader[shaderNo]);
	}
	Display::Device()->Destroy(&m_VertexShader);

	Display::Device()->Destroy(&m_VertexBuffer);
}

void C_DropShadows::OnDestroyVolatile()
{
	Display::Device()->Destroy(&m_RenderTexture);
}

void C_DropShadows::Render() const
{
	PERF_EVENT("RenderDropShadows");

	VisibleDropShadows visibleShadows;
	GatherVisibleShadows(visibleShadows);
	
	Display::Device()->SetBlendMode(Display::E_BlendMode_Multiply);
	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);

	Display::Device()->SetVertexShader(&m_VertexShader);

	Display::Device()->SetTexture(0, m_pDepthTexture);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

	Display::Device()->SetTexture(1, m_pNormalTexture);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_False);

	Display::Device()->SetVertexDeclaration(Display::Vertex_XY::m_pDeclaration);
	Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(Display::Vertex_XY));

	Maths::HMat44 mTexture;
	{
		Display::C_Matrix mTemp;
		Display::MatrixSetIdentity(&mTemp);
		mTemp._11 = +2.0f;
		mTemp._22 = -2.0f;
		mTemp._41 = -1.0f;
		mTemp._42 = +1.0f;
		mTexture = Maths::Load<Maths::HMat44>(&mTemp);
	}
	const Maths::HMat44 mDeviceToWorld = Display::Device()->GetViewSettings3D().GetWorldToDevice().Inverse();
	const Maths::HMat44 mDeviceToCamera = Display::Device()->GetViewSettings3D().GetCameraToDevice().Inverse();

	MathsStorage::Float4 vDepthParameter = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	vDepthParameter.y = vDepthParameter.y / Display::Device()->GetViewSettings3D().GetFarClip();

	Maths::HVec4 vFar0;
	Maths::HVec4 vFar1;
	Maths::HVec4 vFar2;
	Maths::HVec4 vFar3;
	if (Display::Device()->GetDepthFlip())
	{
		vFar0 = Maths::HVec4(-1.0f, -1.0f, 0.0f, 1.0f) * mDeviceToCamera;
		vFar1 = Maths::HVec4(-1.0f, +1.0f, 0.0f, 1.0f) * mDeviceToCamera;
		vFar2 = Maths::HVec4(+1.0f, +1.0f, 0.0f, 1.0f) * mDeviceToCamera;
		vFar3 = Maths::HVec4(+1.0f, -1.0f, 0.0f, 1.0f) * mDeviceToCamera;
	}
	else
	{
		vFar0 = Maths::HVec4(-1.0f, -1.0f, 1.0f, 1.0f) * mDeviceToCamera;
		vFar1 = Maths::HVec4(-1.0f, +1.0f, 1.0f, 1.0f) * mDeviceToCamera;
		vFar2 = Maths::HVec4(+1.0f, +1.0f, 1.0f, 1.0f) * mDeviceToCamera;
		vFar3 = Maths::HVec4(+1.0f, -1.0f, 1.0f, 1.0f) * mDeviceToCamera;
	}

	vFar0 = vFar0 * vFar0.W().Rcp();
	vFar1 = vFar1 * vFar1.W().Rcp();
	vFar2 = vFar2 * vFar2.W().Rcp();
	vFar3 = vFar3 * vFar3.W().Rcp();
	vFar0 = vFar0 * vDepthParameter.y;
	vFar1 = vFar1 * vDepthParameter.y;
	vFar2 = vFar2 * vDepthParameter.y;
	vFar3 = vFar3 * vDepthParameter.y;

	Display::C_Float4 vsSettings[4];
	Maths::Store(vFar0, &vsSettings[0]);
	Maths::Store(vFar1, &vsSettings[1]);
	Maths::Store(vFar2, &vsSettings[2]);
	Maths::Store(vFar3, &vsSettings[3]);

	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS, (F32*)vsSettings, 4);


	//MathsStorage::Float4 vDepthParameter = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	Display::C_Float4 vLuminance = Render::C_RenderState::Instance().ShaderHDR() ? Render::C_RenderState::Instance().HDR()->GetShaderParameters() : Render::C_RenderState::Instance().GetGainShaderParameters();

	//UpVector
	Display::C_Float4 vViewUpVector;
	//Maths::Store(Display::Device()->GetViewSettings3D().GetCameraToWorld().AY()||Maths::HVecW(1.f), &vViewUpVector);
	vViewUpVector = Display::C_Float4(Display::Device()->GetViewSettings3D().GetCameraToWorld().AX().Y(), Display::Device()->GetViewSettings3D().GetCameraToWorld().AY().Y(), Display::Device()->GetViewSettings3D().GetCameraToWorld().AZ().Y(), 1.f);

#if defined(PLATFORM_PC)
	const Display::C_RenderTarget& renderTarget = Display::Device()->GetRenderTarget();
	const F32 fOffsetX = -1.0f / (F32) renderTarget.GetColourSurface()->GetWidth();
	const F32 fOffsetY = +1.0f / (F32) renderTarget.GetColourSurface()->GetHeight();
#else
	const F32 fOffsetX = 0.0f;
	const F32 fOffsetY = 0.0f;
#endif


	for (U32 y = 0; y < m_VerticalTiles; ++y)
	{
		for (U32 x = 0; x < m_HorizontalTiles; ++x)
		{
			const F32 fDX = 2.0f / (F32) m_HorizontalTiles;
			const F32 fDY = 2.0f / (F32) m_VerticalTiles;
			const F32 fX = x * fDX - 1.0f + fOffsetX;
			const F32 fY = y * fDY - 1.0f + fOffsetY;

			const Maths::C_Rect rectangle(Maths::HVec2(fX, fY), Maths::HVec2(fX + fDX, fY + fDY));

			U32 shadowCount = 0;
			Display::C_Float4 vShadowPositions[c_MaximumShadows];
			Display::C_Float4 vShadowIntensities[c_MaximumShadows];

			U32 renderCount = 0;

			U32 firstVisibleShadow = 0;
			bool endProcessing = false;
			do 
			{
				endProcessing = GatherVisibleParameters(visibleShadows, rectangle, firstVisibleShadow, shadowCount, vShadowPositions, vShadowIntensities);

				if (shadowCount > 0)
				{
					const U32 shader = shadowCount - 1;

					Display::Device()->SetPixelShaderParameterArrayF(m_PositionParameter[shader], (F32*)&vShadowPositions);
					Display::Device()->SetPixelShaderParameterArrayF(m_IntensityParameter[shader], (F32*)&vShadowIntensities);
					Display::Device()->SetPixelShaderParameterF(m_DepthParameter[shader], (F32*)&vDepthParameter);
					Display::Device()->SetPixelShaderParameterF(m_LuminanceParameter[shader], (F32*)&vLuminance);
					Display::Device()->SetPixelShaderParameterF(m_ViewUpVectorParameter[shader], (F32*)&vViewUpVector);
					Display::Device()->SetPixelShaderParameters(&m_PixelShader[shader]);

					Display::Device()->SetPixelShader(&m_PixelShader[shader]);

					Display::Device()->DrawPrimitive(Display::E_PrimitiveType_TriangleStrip, (y * m_HorizontalTiles + x) * 4, 4);
					renderCount++;
				}

			} while (!endProcessing);

			if (gDebugShadowDropShadowTiles)
			{
				//Tile pos
				F32 posX, posY;
				posX = 1024.f * ((F32(x) + 0.5f) / F32(m_HorizontalTiles));
				posY = 768.f * (1.f - ((F32(y) + 0.5f) / F32(m_VerticalTiles)));
				Util::C_String msg = Util::C_String::Format("%i",renderCount);
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(posX,posY), msg);	
			}
		}
	}

	Display::Device()->SetTexture(0, 0);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_Default);

	Display::Device()->SetTexture(1, 0);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_Default);

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthEnable(Display::E_Bool_Default);


	m_DropShadows.Clear();
}

void C_DropShadows::GatherVisibleShadows(VisibleDropShadows& visibleShadows) const
{
	const Maths::VF32 vZero	= Maths::Const<Maths::VF32>::Zero();
	const Maths::VF32 vOne	= Maths::Const<Maths::VF32>::One();

	const Maths::Mat44 mView = Maths::Load<Maths::HMat44>(&Display::Device()->GetViewSettings3D().GetViewMatrix());
	const Maths::Mat44 mProj = Maths::Load<Maths::HMat44>(&Display::Device()->GetViewSettings3D().GetProjMatrix());
	
	const Maths::VF32 vUpCamera = vOne - Maths::Clamp(Display::Device()->GetViewSettings3D().GetCameraToWorld().AY().Y(), vZero, vOne);

	const Maths::VF32 vMinSize		= Maths::MakeVF32(gDebugShadowDropShadowsMinSize);
	const Maths::VF32 vMaxDistance	= Maths::MakeVF32(gDebugShadowDropShadowsMaxDistant);

	const Maths::C_Rect screenRectangle(Maths::HVec2(-0.5f, -0.5f), Maths::HVec2(0.5f, 0.5f));

	for (S32 shadowNo = 0; shadowNo < m_DropShadows.Size(); ++shadowNo)
	{

		const Maths::Vec4F32	vPositionRadius		= Maths::Load<Maths::Vec4F32>((MathsStorage::Float4*) &m_DropShadows[shadowNo].m_vPosition);

		const Maths::Vec4F32	vWorldPosition		= Maths::MakeVec4F32(Maths::X(vPositionRadius), Maths::Y(vPositionRadius), Maths::Z(vPositionRadius), vOne);
		const Maths::VF32		fRadius				= Maths::W(vPositionRadius);
		const Maths::Vec4F32	vViewExtents0		= Maths::MakeVec4F32(-fRadius, -fRadius, -fRadius, vZero);
		const Maths::Vec4F32	vViewExtents1		= Maths::MakeVec4F32(-fRadius, -fRadius, +fRadius , vZero);
		const Maths::Vec4F32	vViewExtents2		= Maths::MakeVec4F32(+fRadius, fRadius*vUpCamera, -fRadius, vZero);
		const Maths::Vec4F32	vViewExtents3		= Maths::MakeVec4F32(+fRadius, fRadius*vUpCamera, +fRadius , vZero);

		const Maths::Vec4F32	vViewPosition		= vWorldPosition * mView;
		const Maths::Vec4F32	vViewPosition0		= vViewPosition + vViewExtents0;
		const Maths::Vec4F32	vViewPosition1		= vViewPosition + vViewExtents1;
		const Maths::Vec4F32	vViewPosition2		= vViewPosition + vViewExtents2;
		const Maths::Vec4F32	vViewPosition3		= vViewPosition + vViewExtents3;

		const Maths::Vec4F32	vProjPosition		= vViewPosition * mProj;
		const Maths::Vec4F32	vProjPosition0		= vViewPosition0 * mProj;
		const Maths::Vec4F32	vProjPosition1		= vViewPosition1 * mProj;
		const Maths::Vec4F32	vProjPosition2		= vViewPosition2 * mProj;
		const Maths::Vec4F32	vProjPosition3		= vViewPosition3 * mProj;

		if (vZero < Maths::W(vProjPosition3) && Maths::W(vProjPosition3) < vMaxDistance)
		{
			Maths::Vec4F32	vScreenPosition0	= vProjPosition0 * Maths::Rcp(Maths::W(vProjPosition0));
			Maths::Vec4F32	vScreenPosition1	= vProjPosition1 * Maths::Rcp(Maths::W(vProjPosition1));
			Maths::Vec4F32	vScreenPosition2	= vProjPosition2 * Maths::Rcp(Maths::W(vProjPosition2));
			Maths::Vec4F32	vScreenPosition3	= vProjPosition3 * Maths::Rcp(Maths::W(vProjPosition3));

			vScreenPosition0 = Maths::Select(Maths::W(vProjPosition0) < vZero, Maths::Sign(vProjPosition0), vScreenPosition0);
			vScreenPosition1 = Maths::Select(Maths::W(vProjPosition1) < vZero, Maths::Sign(vProjPosition1), vScreenPosition1);
			vScreenPosition2 = Maths::Select(Maths::W(vProjPosition2) < vZero, Maths::Sign(vProjPosition2), vScreenPosition2);
			vScreenPosition3 = Maths::Select(Maths::W(vProjPosition3) < vZero, Maths::Sign(vProjPosition3), vScreenPosition3);

			const Maths::Vec4F32	vScreenMin			= Maths::Min(vScreenPosition0, Maths::Min(vScreenPosition1, Maths::Min(vScreenPosition2, vScreenPosition3)));
			const Maths::Vec4F32	vScreenMax			= Maths::Max(vScreenPosition0, Maths::Max(vScreenPosition1, Maths::Max(vScreenPosition2, vScreenPosition3)));
			const Maths::Vec4F32	vScreenSize			= vScreenMax - vScreenMin;

			if (vMinSize < Maths::X(vScreenSize) && vMinSize < Maths::Y(vScreenSize))
			{
				const Maths::HVec2 vMin(Maths::X(vScreenMin), Maths::Y(vScreenMin));
				const Maths::HVec2 vMax(Maths::X(vScreenMax), Maths::Y(vScreenMax));

				C_VisibleDropShadow visibleShadow;
				visibleShadow.m_rectangle	= Maths::C_Rect(vMin, vMax);
				visibleShadow.m_pShadow		= &m_DropShadows[shadowNo];
				
				if (gDebugShadowDropShadows)
				{
					Maths::HVec3 vPosition(m_DropShadows[shadowNo].m_vPosition.x, m_DropShadows[shadowNo].m_vPosition.y, m_DropShadows[shadowNo].m_vPosition.z);
					
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(vPosition, m_DropShadows[shadowNo].m_fRadius, Maths::RGBA(200,200,200,255));
					Util::C_String msg = Util::C_String::Format("R: %f/nI: %f",m_DropShadows[shadowNo].m_fRadius, m_DropShadows[shadowNo].m_fIntensity);
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(vPosition, 1, msg);	
				}

				visibleShadows.PushBack(visibleShadow);
			}
		}
	}
}

bool C_DropShadows::GatherVisibleParameters(const VisibleDropShadows& visibleShadows, const Maths::C_Rect& rectangle, U32& firstProcessedShadow, U32& shadowCount, Display::C_Float4* pPositionFalloffs, Display::C_Float4* pIntensities) const
{
	shadowCount = 0;
	for (S32 i = firstProcessedShadow; i < visibleShadows.Size(); ++i)
	{
		if (Maths::Intersect(rectangle, visibleShadows[i].m_rectangle))
		{
			const C_DropShadow* const pShadow	= visibleShadows[i].m_pShadow;

			//Convert position to viewspace
			Maths::HVec4 shadowPosition = Maths::Load<Maths::HVec3>(&pShadow->m_vPosition) || Maths::HVecW(1.f);
			Maths::HVec3 shadowViewPosition = Maths::HVec3(shadowPosition * Display::Device()->GetViewSettings3D().GetWorldToCamera());

			pPositionFalloffs[shadowCount]	= MathsStorage::Float4(shadowViewPosition.X(), shadowViewPosition.Y(), shadowViewPosition.Z(), 1.0f / (pShadow->m_fRadius * pShadow->m_fRadius));
			pIntensities[shadowCount]		= MathsStorage::Float4(pShadow->m_fIntensity, 0.0f, 0.0f, 0.0f);
			
			++shadowCount;
		}
		if (c_MaximumShadows <= shadowCount)
		{
			firstProcessedShadow = i + 1;
			return false;
		}
	}
	return true;
}

void C_DropShadows::AddShadow(const MathsStorage::Float3& vPosition, const F32 fRadius, const F32 fIntensity)
{
	m_DropShadows.PushBack();

	m_DropShadows.Back().m_vPosition = vPosition;
	m_DropShadows.Back().m_fRadius = fRadius;
	m_DropShadows.Back().m_fIntensity = fIntensity;
}

}; // namespace Deferred
}; // namespace Render
