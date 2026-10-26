#include "stdafx.h"
#include "Graphics/Lighting/SpotLights.h"

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>
#include <Deferred/Shaders/Constants.h>
#include <Display/Buffer/VertexDeclarations.h>
#include <Render/State/RenderState.h>

EXTERN_SHADER(vsStencilPrepass);
EXTERN_SHADER(psStencilPrepass);
EXTERN_SHADER(vsSpotLight);
EXTERN_SHADER(psSpotLight);
EXTERN_SHADER(psSpotLightProjected);
EXTERN_SHADER(psSpotLightVolume);

Debug::Float	g_debugPrepassLightCameraBias		("Graphics.Game.LightPrepass.LightCameraBias",		0.16f,	Debug::Range(0.0f, 1.0f));
Debug::Bool		g_debugPrepassDepthBounds			("Graphics.Game.LightPrepass.DepthBounds",			true);	

C_SpotLights::C_SpotLights() : Display::I_Renderer("Light"), m_pNormalDepthTexture(0)
{
}

C_SpotLights::~C_SpotLights()
{
	DestroyRenderer();
}

bool C_SpotLights::OnCreate()
{
	RESOURCE_MEMORY_ID("SpotLights");

	if (!Display::CreateVertexShaderFromFunc("Shaders/vsSpotlight", 0, &m_SpotlightVertexShader, REF_SHADER(vsSpotLight)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsStencilPrepass", 0, &m_StencilVertexShader, REF_SHADER(vsStencilPrepass)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSpotlight", 0, &m_SpotlightPixelShader[E_SpotlightShader_Default], REF_SHADER(psSpotLight)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSpotlightProjected", 0, &m_SpotlightPixelShader[E_SpotlightShader_Projected], REF_SHADER(psSpotLightProjected)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psSpotlightVolume", 0, &m_SpotlightPixelShader[E_SpotlightShader_Volume], REF_SHADER(psSpotLightVolume)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psStencilPrepass", 0, &m_StencilPixelShader, REF_SHADER(psStencilPrepass)))
		return false;

	for (S32 i = 0; i < E_SpotlightShader_Count; ++i)
	{
		m_SpotlightLightParameter[i]		= m_SpotlightPixelShader[i].GetParameterArray(CP_RENDER_SETTINGS, "g_mLight", 4);
		m_SpotlightPositionParameter[i]		= m_SpotlightPixelShader[i].GetParameter(CP_RENDER_SETTINGS_4, "g_vPosition");
		m_SpotlightDirectionParameter[i]	= m_SpotlightPixelShader[i].GetParameter(CP_RENDER_SETTINGS_5, "g_vDirection");
		m_SpotlightColourParameter[i]		= m_SpotlightPixelShader[i].GetParameter(CP_RENDER_SETTINGS_6, "g_vColour");
		m_SpotlightFalloffParameter[i]		= m_SpotlightPixelShader[i].GetParameter(CP_RENDER_SETTINGS_7, "g_vFalloff");
	}

	return true;
}

void C_SpotLights::OnDestroy()
{
	Display::Device()->Destroy(&m_SpotlightVertexShader);
	Display::Device()->Destroy(&m_StencilVertexShader);
	Display::Device()->Destroy(&m_SpotlightPixelShader[E_SpotlightShader_Default]);
	Display::Device()->Destroy(&m_SpotlightPixelShader[E_SpotlightShader_Projected]);
	Display::Device()->Destroy(&m_SpotlightPixelShader[E_SpotlightShader_Volume]);
	Display::Device()->Destroy(&m_StencilPixelShader);
}

void C_SpotLights::RenderSpotlight(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfView, const Display::C_Float3& vColour) const
{
	PERF_EVENT("Render.Deferred.Spotlight");

	RenderStencilCone(mLight, fRadius, fFieldOfView, fFieldOfView);
	BindStencilTest();

	bool bCameraOutside = !IsCameraInside(mLight, fRadius, fFieldOfView, fFieldOfView);
	if (bCameraOutside)
	{
		BindFrontFaces(E_SpotlightShader_Default, mLight, fRadius, fFieldOfView, fFieldOfView, vColour);
	}
	else
	{
		BindBackFaces(E_SpotlightShader_Default, mLight, fRadius, fFieldOfView, fFieldOfView, vColour);
	}

	RenderCone(fRadius, fFieldOfView, fFieldOfView);

	Unbind();

	UnbindStencilTest();
}

void C_SpotLights::RenderSpotlightSimple(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfView, const Display::C_Float3& vColour) const
{
	PERF_EVENT("Render.Deferred.SpotlightSimple");

	bool bCameraOutside = !IsCameraInside(mLight, fRadius, fFieldOfView, fFieldOfView);
	if (bCameraOutside)
	{
		BindFrontFaces(E_SpotlightShader_Default, mLight, fRadius, fFieldOfView, fFieldOfView, vColour);
	}
	else
	{
		BindBackFaces(E_SpotlightShader_Default, mLight, fRadius, fFieldOfView, fFieldOfView, vColour);
	}

	RenderCone(fRadius, fFieldOfView, fFieldOfView);

	Unbind();
}

void C_SpotLights::RenderSpotlightProjected(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour, const Display::C_Texture2D* pTexture2D) const
{
	PERF_EVENT("Render.Deferred.SpotlightProjected");

	RenderStencilCone(mLight, fRadius, fFieldOfViewX, fFieldOfViewY);
	BindStencilTest();

	bool bCameraOutside = !IsCameraInside(mLight, fRadius, fFieldOfViewX, fFieldOfViewY);
	if (bCameraOutside)
	{
		BindFrontFaces(E_SpotlightShader_Projected, mLight, fRadius, fFieldOfViewX, fFieldOfViewY, vColour);
	}
	else
	{
		BindBackFaces(E_SpotlightShader_Projected, mLight, fRadius, fFieldOfViewX, fFieldOfViewY, vColour);
	}

	Display::Device()->SetTexture(1, pTexture2D);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Border);
	Display::Device()->SetTextureBorderColour(1, 0x00000000);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_False);

	RenderCone(fRadius, fFieldOfViewX, fFieldOfViewY);

	Display::Device()->SetTexture(1, 0);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_Default);

	Unbind();

	UnbindStencilTest();
}

void C_SpotLights::RenderSpotlightProjectedSimple(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour, const Display::C_Texture2D* pTexture2D) const
{
	PERF_EVENT("Render.Deferred.SpotlightProjectedSimple");

	bool bCameraOutside = !IsCameraInside(mLight, fRadius, fFieldOfViewX, fFieldOfViewY);
	if (bCameraOutside)
	{
		BindFrontFaces(E_SpotlightShader_Projected, mLight, fRadius, fFieldOfViewX, fFieldOfViewY, vColour);
	}
	else
	{
		BindBackFaces(E_SpotlightShader_Projected, mLight, fRadius, fFieldOfViewX, fFieldOfViewY, vColour);
	}

	Display::Device()->SetTexture(1, pTexture2D);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Border);
	Display::Device()->SetTextureBorderColour(1, 0x00000000);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_False);

	RenderCone(fRadius, fFieldOfViewX, fFieldOfViewY);

	Display::Device()->SetTexture(1, 0);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_Default);

	Unbind();
}

void C_SpotLights::RenderSpotlightVolume(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour, const Display::C_Texture3D* pTexture3D) const
{
	PERF_EVENT("Render.Deferred.SpotlightVolume");

	const F32 fWidth = 2.0f * fRadius * tanf(fFieldOfViewX);
	const F32 fHeight = 2.0f * fRadius * tanf(fFieldOfViewY);
	const F32 fLength = fRadius;

	RenderStencilBox(mLight, fWidth, fHeight, fLength);
	BindStencilTest();

	bool bCameraOutside = !IsCameraInside(mLight, fRadius, fFieldOfViewX, fFieldOfViewY);
	if (bCameraOutside)
	{
		BindFrontFaces(E_SpotlightShader_Volume, mLight, fRadius, fFieldOfViewX, fFieldOfViewY, vColour);
	}
	else
	{
		BindBackFaces(E_SpotlightShader_Volume, mLight, fRadius, fFieldOfViewX, fFieldOfViewY, vColour);
	}

	Display::Device()->SetTexture(1, pTexture3D);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Border);
	Display::Device()->SetTextureBorderColour(1, 0x00000000);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_False);

	RenderBox(fWidth, fHeight, fLength);

	Display::Device()->SetTexture(1, 0);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_Default);

	Unbind();

	UnbindStencilTest();
}

void C_SpotLights::RenderSpotlightVolumeSimple(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour, const Display::C_Texture3D* pTexture3D) const
{
	PERF_EVENT("Render.Deferred.SpotlightVolumeSimple");

	bool bCameraOutside = !IsCameraInside(mLight, fRadius, fFieldOfViewX, fFieldOfViewY);
	if (bCameraOutside)
	{
		BindFrontFaces(E_SpotlightShader_Volume, mLight, fRadius, fFieldOfViewX, fFieldOfViewY, vColour);
	}
	else
	{
		BindBackFaces(E_SpotlightShader_Volume, mLight, fRadius, fFieldOfViewX, fFieldOfViewY, vColour);
	}

	Display::Device()->SetTexture(1, pTexture3D);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Border);
	Display::Device()->SetTextureBorderColour(1, 0x00000000);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_False);

	const F32 fWidth = 2.0f * fRadius * tanf(fFieldOfViewX);
	const F32 fHeight = 2.0f * fRadius * tanf(fFieldOfViewY);
	const F32 fLength = fRadius;

	RenderBox(fWidth, fHeight, fLength);

	Display::Device()->SetTexture(1, 0);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(1, Display::E_Bool_Default);

	Unbind();
}

void C_SpotLights::BindFrontFaces(const E_SpotlightShader eShader, const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour) const
{
	Bind(eShader, mLight, fRadius, fFieldOfViewX, fFieldOfViewY, vColour, false);	
}

void C_SpotLights::BindBackFaces(const E_SpotlightShader eShader, const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour) const
{
	Bind(eShader, mLight, fRadius, fFieldOfViewX, fFieldOfViewY, vColour, true);	
}

void C_SpotLights::Bind(const E_SpotlightShader eShader, const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY, const Display::C_Float3& vColour, const bool bBackFaces) const
{
	Display::Device()->SetBlendMode(Display::E_BlendMode_Additive);
	Display::Device()->SetCullMode(bBackFaces ? Display::E_CullMode_Front : Display::E_CullMode_Back);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	if (Display::Device()->GetDepthFlip())
		Display::Device()->SetDepthFunc(bBackFaces ? Display::E_CmpFunc_Less : Display::E_CmpFunc_Greater);
	else
		Display::Device()->SetDepthFunc(bBackFaces ? Display::E_CmpFunc_Greater : Display::E_CmpFunc_Less);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

	Display::Device()->SetTexture(0, m_pNormalDepthTexture);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_False);

	Display::Device()->SetVertexShader(&m_SpotlightVertexShader);
	Display::Device()->SetPixelShader(&m_SpotlightPixelShader[eShader]);

	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&mLight, 4);

	Maths::Mat44 mLightWorld = Maths::Load<Maths::Mat44>((MathsStorage::MatrixRow44*)&mLight);
	Maths::Mat44 mLightProj;
	if (eShader == E_SpotlightShader_Volume)
		mLightProj = CreateBoxProjectionMatrix(fFieldOfViewX, fFieldOfViewY, fRadius);
	else
		mLightProj = CreateConeProjectionMatrix(fFieldOfViewX, fFieldOfViewY, fRadius);

	Display::C_Float4 vLightPosition, vLightDirection;
	Maths::Store(Maths::MakeVec4F32(Maths::RowW(mLightWorld)), &vLightPosition);
	Maths::Store(Maths::MakeVec4F32(Maths::RowZ(mLightWorld)), &vLightDirection);

	Display::C_Matrix mLightFinal;
	Maths::Store(Maths::Inverse(mLightWorld) * mLightProj, (MathsStorage::MatrixRow44*)&mLightFinal);

	Display::Device()->SetPixelShaderParameterArrayF(m_SpotlightLightParameter[eShader], (F32*)&mLightFinal);

	const Display::C_Float4 vLightFalloff(1.0f / (fRadius * fRadius), cosf(fFieldOfViewX), 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_SpotlightFalloffParameter[eShader], (F32*)&vLightFalloff);

	const Display::C_Float4 vLightColour(vColour.x, vColour.y, vColour.z, 1.0f);
	Display::Device()->SetPixelShaderParameterF(m_SpotlightColourParameter[eShader], (F32*)&vLightColour);

	Display::Device()->SetPixelShaderParameterF(m_SpotlightPositionParameter[eShader], (F32*)&vLightPosition);
	Display::Device()->SetPixelShaderParameterF(m_SpotlightDirectionParameter[eShader], (F32*)&vLightDirection);

	const U32 iWidthInPixels = Display::Device()->GetViewport().m_W;
	const U32 iHeightInPixels = Display::Device()->GetViewport().m_H;
	MathsStorage::Float4 vOffset = MathsStorage::Float4(0.5f / iWidthInPixels, 0.5f / iHeightInPixels, 0.0f, 0.0f);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS, (F32*)&vOffset, 1);

	const F32 fFarClip = m_FarClip;
	const F32 fInvProjX = fFarClip / Display::Device()->GetViewSettings3D().GetProjMatrix()._11;
	const F32 fInvProjY = fFarClip / Display::Device()->GetViewSettings3D().GetProjMatrix()._22;
	const F32 fInvShiftX = -fInvProjX * Display::Device()->GetViewSettings3D().GetProjMatrix()._31;
	const F32 fInvShiftY = -fInvProjY * Display::Device()->GetViewSettings3D().GetProjMatrix()._32;
	const Display::C_Float4 vXYScale(+2.0f * fInvProjX, -2.0f * fInvProjY, -fInvProjX + fInvShiftX, +fInvProjY + fInvShiftY);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_1, (F32*)&vXYScale, 1);

	const Display::C_Float4 vFarClip(fFarClip, fFarClip, fFarClip, fFarClip);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_2, (F32*)&vFarClip, 1);

	Display::Device()->SetPixelShaderParameters(&m_SpotlightPixelShader[eShader]);

	BindDepthBounds(mLight, fRadius, fFieldOfViewX, fFieldOfViewY);
}

void C_SpotLights::Unbind() const
{
	UnbindDepthBounds();

	Display::Device()->SetTexture(0, 0);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
	Display::Device()->SetTextureGamma(0, Display::E_Bool_Default);

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetDepthEnable(Display::E_Bool_Default);
	Display::Device()->SetDepthFunc(Display::E_CmpFunc_Default);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_Default);
}

void C_SpotLights::BindStencilTest() const
{

	Display::C_StencilState stencilState;
	stencilState.m_StencilDepthFail	= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass	= Display::E_StencilOp_Keep;
	stencilState.m_StencilEnable	= Display::E_Bool_True;
	stencilState.m_StencilFail		= Display::E_StencilOp_Keep;
	stencilState.m_StencilFunc		= Display::E_CmpFunc_Equal;
	stencilState.m_StencilMask		= 0xFF;
	stencilState.m_StencilRef		= 0xFF;
	stencilState.m_StencilWriteMask	= 0x00;
	Display::Device()->SetStencilState(stencilState);
}

void C_SpotLights::UnbindStencilTest() const
{

	Display::C_StencilState stencilState;
	stencilState.m_StencilDepthFail	= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass	= Display::E_StencilOp_Keep;
	stencilState.m_StencilEnable	= Display::E_Bool_False;
	stencilState.m_StencilFail		= Display::E_StencilOp_Keep;
	stencilState.m_StencilFunc		= Display::E_CmpFunc_Always;
	stencilState.m_StencilMask		= 0xFF;
	stencilState.m_StencilRef		= 0xFF;
	stencilState.m_StencilWriteMask	= 0xFF;
	Display::Device()->SetStencilState(stencilState);
}

Maths::Mat44 C_SpotLights::CreateBoxProjectionMatrix(const F32 fFieldOfViewX, const F32 fFieldOfViewY, const F32 fFarClip) const
{
	const F32	fWidth	= fFarClip * tanf(fFieldOfViewX);
	const F32	fHeight	= fFarClip * tanf(fFieldOfViewY);
	const F32	fLength	= fFarClip;

	const Maths::Mat44	mProj	= Maths::MakeMat44(	0.5f / fWidth, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f / fHeight, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f / fLength, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f );
	return mProj;
}

Maths::Mat44 C_SpotLights::CreateConeProjectionMatrix(const F32 fFieldOfViewX, const F32 fFieldOfViewY, const F32 fFarClip) const
{
	const F32	fWidth	= tanf(fFieldOfViewX);
	const F32	fHeight	= tanf(fFieldOfViewY);

	const Maths::Mat44	mProj	= Maths::MakeMat44(	0.5f / fWidth, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f / fHeight, 0.0f, 0.0f,
		0.5f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f );
	return mProj;
}

bool C_SpotLights::IsCameraInside(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY) const
{
	const Maths::Mat44		mWorldView		= Maths::Load<Maths::Mat44>((MathsStorage::MatrixRow44*)&mLight);

	const F32				fWidth			= fRadius * tanf(fFieldOfViewX) + g_debugPrepassLightCameraBias;
	const F32				fHeight			= fRadius * tanf(fFieldOfViewY) + g_debugPrepassLightCameraBias;
	const F32				fLengthBegin	= -g_debugPrepassLightCameraBias;
	const F32				fLengthEnd		= fRadius + g_debugPrepassLightCameraBias;

	Maths::Vec4F32	vMin	= Maths::MakeVec4F32(-fWidth, -fHeight, fLengthBegin, 1.0f);
	Maths::Vec4F32	vMax	= Maths::MakeVec4F32(+fWidth, +fHeight, fLengthEnd, 1.0f);

	Maths::Vec4F32	vCameraPosition	= Maths::MakeVec4F32(0.0f, 0.0f, 0.0f, 1.0f) * Maths::Inverse(mWorldView);

	const Maths::Vec4Bool	vSelect0		= vCameraPosition >= vMin;
	const Maths::Vec4Bool	vSelect1		= vCameraPosition <= vMax;
	const Maths::Vec4Bool	vSelect			= vSelect0 & vSelect1;
	const Maths::VBool		bSelect			= Maths::HorizontalAND3(vSelect);	

	return bSelect;
}

void C_SpotLights::RenderCone(const F32 fLength, const F32 fFieldOfViewX, const F32 fFieldOfViewY) const
{
	const F32				fWidth	= fLength * tanf(fFieldOfViewX);
	const F32				fHeight	= fLength * tanf(fFieldOfViewY);
	const Maths::Vec4F32	vBegin	= Maths::MakeVec4F32(0.0f, 0.0f, 0.0f, 0.0f);
	const Maths::Vec4F32	v0		= Maths::MakeVec4F32(-fWidth, -fHeight, fLength, 0.0f);
	const Maths::Vec4F32	v1		= Maths::MakeVec4F32(-fWidth, +fHeight, fLength, 0.0f);
	const Maths::Vec4F32	v2		= Maths::MakeVec4F32(+fWidth, +fHeight, fLength, 0.0f);
	const Maths::Vec4F32	v3		= Maths::MakeVec4F32(+fWidth, -fHeight, fLength, 0.0f);

	Display::Vertex_XYZ* pVertex = (Display::Vertex_XYZ*)Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleList, 18, sizeof(Display::Vertex_XYZ), Display::Vertex_XYZ::m_pDeclaration);
	if (pVertex)
	{
		// 0
		Maths::Store(vBegin, (MathsStorage::Float3*)pVertex); pVertex++;
		Maths::Store(v0, (MathsStorage::Float3*)pVertex); pVertex++;
		Maths::Store(v1, (MathsStorage::Float3*)pVertex); pVertex++;

		// 1
		Maths::Store(vBegin, (MathsStorage::Float3*)pVertex); pVertex++;
		Maths::Store(v1, (MathsStorage::Float3*)pVertex); pVertex++;
		Maths::Store(v2, (MathsStorage::Float3*)pVertex); pVertex++;

		// 2
		Maths::Store(vBegin, (MathsStorage::Float3*)pVertex); pVertex++;
		Maths::Store(v2, (MathsStorage::Float3*)pVertex); pVertex++;
		Maths::Store(v3, (MathsStorage::Float3*)pVertex); pVertex++;

		// 3
		Maths::Store(vBegin, (MathsStorage::Float3*)pVertex); pVertex++;
		Maths::Store(v3, (MathsStorage::Float3*)pVertex); pVertex++;
		Maths::Store(v0, (MathsStorage::Float3*)pVertex); pVertex++;

		// 4
		Maths::Store(v0, (MathsStorage::Float3*)pVertex); pVertex++;
		Maths::Store(v3, (MathsStorage::Float3*)pVertex); pVertex++;
		Maths::Store(v2, (MathsStorage::Float3*)pVertex); pVertex++;

		// 5
		Maths::Store(v0, (MathsStorage::Float3*)pVertex); pVertex++;
		Maths::Store(v2, (MathsStorage::Float3*)pVertex); pVertex++;
		Maths::Store(v1, (MathsStorage::Float3*)pVertex); pVertex++;

		Display::Device()->EndVertices();
	}
}

void C_SpotLights::RenderBox(const F32 fWidth, const F32 fHeight, const F32 fLength) const
{
	Display::Vertex_XYZ vertex[8] = 
	{
		{ -0.5f * fWidth, -0.5f * fHeight, 0.0f },
		{ -0.5f * fWidth, -0.5f * fHeight, fLength },
		{ -0.5f * fWidth, +0.5f * fHeight, 0.0f },
		{ -0.5f * fWidth, +0.5f * fHeight, fLength },
		{ +0.5f * fWidth, -0.5f * fHeight, 0.0f },
		{ +0.5f * fWidth, -0.5f * fHeight, fLength },
		{ +0.5f * fWidth, +0.5f * fHeight, 0.0f },
		{ +0.5f * fWidth, +0.5f * fHeight, fLength },
	};

	Display::Vertex_XYZ* pVertex = (Display::Vertex_XYZ*)Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleList, 36, sizeof(Display::Vertex_XYZ), Display::Vertex_XYZ::m_pDeclaration);
	if (pVertex)
	{
		// top
		*(pVertex++) = vertex[2]; *(pVertex++) = vertex[3]; *(pVertex++) = vertex[7];
		*(pVertex++) = vertex[2]; *(pVertex++) = vertex[7]; *(pVertex++) = vertex[6];
		// bottom
		*(pVertex++) = vertex[0]; *(pVertex++) = vertex[5]; *(pVertex++) = vertex[1];
		*(pVertex++) = vertex[0]; *(pVertex++) = vertex[4]; *(pVertex++) = vertex[5];
		// left
		*(pVertex++) = vertex[1]; *(pVertex++) = vertex[3]; *(pVertex++) = vertex[2];
		*(pVertex++) = vertex[1]; *(pVertex++) = vertex[2]; *(pVertex++) = vertex[0];
		// right
		*(pVertex++) = vertex[4]; *(pVertex++) = vertex[6]; *(pVertex++) = vertex[5];
		*(pVertex++) = vertex[5]; *(pVertex++) = vertex[6]; *(pVertex++) = vertex[7];
		// front
		*(pVertex++) = vertex[2]; *(pVertex++) = vertex[6]; *(pVertex++) = vertex[0];
		*(pVertex++) = vertex[0]; *(pVertex++) = vertex[6]; *(pVertex++) = vertex[4];
		// back
		*(pVertex++) = vertex[1]; *(pVertex++) = vertex[7]; *(pVertex++) = vertex[3];
		*(pVertex++) = vertex[1]; *(pVertex++) = vertex[5]; *(pVertex++) = vertex[7];

		Display::Device()->EndVertices();
	}
}

void C_SpotLights::RenderStencilCone(const Display::C_Matrix& mLight, const F32 fLength, const F32 fFieldOfViewX, const F32 fFieldOfViewY) const
{
	Display::Device()->Clear(Display::E_Clear_Stencil);

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetCullMode(Display::E_CullMode_Front);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	if (Display::Device()->GetDepthFlip())
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Less);
	else
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Greater);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);	


	Display::C_StencilState stencilState;
	stencilState.m_StencilDepthFail	= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass	= Display::E_StencilOp_Replace;
	stencilState.m_StencilEnable	= Display::E_Bool_True;
	stencilState.m_StencilFail		= Display::E_StencilOp_Keep;
	stencilState.m_StencilFunc		= Display::E_CmpFunc_Always;
	stencilState.m_StencilMask		= 0x00;
	stencilState.m_StencilRef		= 0xFF;
	stencilState.m_StencilWriteMask	= 0xFF;
	Display::Device()->SetStencilState(stencilState);

	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&mLight, 4);

	Display::Device()->SetVertexShader(&m_StencilVertexShader);
	Display::Device()->SetPixelShader(&m_StencilPixelShader);

	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_None);

	RenderCone(fLength, fFieldOfViewX, fFieldOfViewY);

	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);
}

void C_SpotLights::RenderStencilBox(const Display::C_Matrix& mLight, const F32 fWidth, const F32 fHeight, const F32 fLength) const
{
	Display::Device()->Clear(Display::E_Clear_Stencil);

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetCullMode(Display::E_CullMode_Front);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	if (Display::Device()->GetDepthFlip())
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Less);
	else
		Display::Device()->SetDepthFunc(Display::E_CmpFunc_Greater);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);	


	Display::C_StencilState stencilState;
	stencilState.m_StencilDepthFail	= Display::E_StencilOp_Keep;
	stencilState.m_StencilDepthPass	= Display::E_StencilOp_Replace;
	stencilState.m_StencilEnable	= Display::E_Bool_True;
	stencilState.m_StencilFail		= Display::E_StencilOp_Keep;
	stencilState.m_StencilFunc		= Display::E_CmpFunc_Always;
	stencilState.m_StencilMask		= 0x00;
	stencilState.m_StencilRef		= 0xFF;
	stencilState.m_StencilWriteMask	= 0xFF;
	Display::Device()->SetStencilState(stencilState);

	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&mLight, 4);

	Display::Device()->SetVertexShader(&m_StencilVertexShader);
	Display::Device()->SetPixelShader(&m_StencilPixelShader);

	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_None);

	RenderBox(fWidth, fHeight, fLength);

	Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_Default);
}

void C_SpotLights::BindDepthBounds(const Display::C_Matrix& mLight, const F32 fRadius, const F32 fFieldOfViewX, const F32 fFieldOfViewY) const
{
	const F32 fWidth = 2.0f * fRadius * tanf(fFieldOfViewX);
	const F32 fHeight = 2.0f * fRadius * tanf(fFieldOfViewY);
	const F32 fLength = fRadius;

	const Display::C_Matrix projMatrix = Display::Device()->GetViewSettings3D().GetProjMatrix();

	const Maths::Mat44 mWorldView = Maths::Load<Maths::Mat44>(&mLight);
	const Maths::Mat44 mProj = Maths::Load<Maths::Mat44>((MathsStorage::MatrixRow44*)&projMatrix);
	const Maths::Mat44 mWorldViewProj = mWorldView * mProj;

	Maths::Vec4F32 v0	= Maths::MakeVec4F32(-0.5f * fWidth, -0.5f * fHeight, 0.0f, 1.0f);
	Maths::Vec4F32 v1	= Maths::MakeVec4F32(-0.5f * fWidth, -0.5f * fHeight, fLength, 1.0f);
	Maths::Vec4F32 v2	= Maths::MakeVec4F32(-0.5f * fWidth, +0.5f * fHeight, 0.0f, 1.0f);
	Maths::Vec4F32 v3	= Maths::MakeVec4F32(-0.5f * fWidth, +0.5f * fHeight, fLength, 1.0f);
	Maths::Vec4F32 v4	= Maths::MakeVec4F32(+0.5f * fWidth, -0.5f * fHeight, 0.0f, 1.0f);
	Maths::Vec4F32 v5	= Maths::MakeVec4F32(+0.5f * fWidth, -0.5f * fHeight, fLength, 1.0f);
	Maths::Vec4F32 v6	= Maths::MakeVec4F32(+0.5f * fWidth, +0.5f * fHeight, 0.0f, 1.0f);
	Maths::Vec4F32 v7	= Maths::MakeVec4F32(+0.5f * fWidth, +0.5f * fHeight, fLength, 1.0f);

	v0 = v0 * mWorldViewProj;
	v1 = v1 * mWorldViewProj;
	v2 = v2 * mWorldViewProj;
	v3 = v3 * mWorldViewProj;
	v4 = v4 * mWorldViewProj;
	v5 = v5 * mWorldViewProj;
	v6 = v6 * mWorldViewProj;
	v7 = v7 * mWorldViewProj;

	Maths::VF32 z0		= Maths::Z(v0) * Maths::Rcp(Maths::W(v0));
	Maths::VF32 z1		= Maths::Z(v1) * Maths::Rcp(Maths::W(v1));
	Maths::VF32 z2		= Maths::Z(v2) * Maths::Rcp(Maths::W(v2));
	Maths::VF32 z3		= Maths::Z(v3) * Maths::Rcp(Maths::W(v3));
	Maths::VF32 z4		= Maths::Z(v4) * Maths::Rcp(Maths::W(v4));
	Maths::VF32 z5		= Maths::Z(v5) * Maths::Rcp(Maths::W(v5));
	Maths::VF32 z6		= Maths::Z(v6) * Maths::Rcp(Maths::W(v6));
	Maths::VF32 z7		= Maths::Z(v7) * Maths::Rcp(Maths::W(v7));

	Maths::VF32 vZero	= Maths::Const<Maths::VF32>::Zero();
	Maths::VF32 vOne	= Maths::Const<Maths::VF32>::One();
	if(Display::Device()->GetDepthFlip())
	{
		z0 = Maths::Select(vZero < z0, z0, vOne);
		z1 = Maths::Select(vZero < z1, z1, vOne);
		z2 = Maths::Select(vZero < z2, z2, vOne);
		z3 = Maths::Select(vZero < z3, z3, vOne);
		z4 = Maths::Select(vZero < z4, z4, vOne);
		z5 = Maths::Select(vZero < z5, z5, vOne);
		z6 = Maths::Select(vZero < z6, z6, vOne);
		z7 = Maths::Select(vZero < z7, z7, vOne);
	}
	else
	{
		z0 = Maths::Select(vZero < z0, z0, vZero);
		z1 = Maths::Select(vZero < z1, z1, vZero);
		z2 = Maths::Select(vZero < z2, z2, vZero);
		z3 = Maths::Select(vZero < z3, z3, vZero);
		z4 = Maths::Select(vZero < z4, z4, vZero);
		z5 = Maths::Select(vZero < z5, z5, vZero);
		z6 = Maths::Select(vZero < z6, z6, vZero);
		z7 = Maths::Select(vZero < z7, z7, vZero);
	}

	Maths::VF32 vMin = z0;
	vMin = Maths::Min(vMin, z1);
	vMin = Maths::Min(vMin, z2);
	vMin = Maths::Min(vMin, z3);
	vMin = Maths::Min(vMin, z4);
	vMin = Maths::Min(vMin, z5);
	vMin = Maths::Min(vMin, z6);
	vMin = Maths::Min(vMin, z7);
	Maths::VF32 vMax = z0;
	vMax = Maths::Max(vMax, z1);
	vMax = Maths::Max(vMax, z2);
	vMax = Maths::Max(vMax, z3);
	vMax = Maths::Max(vMax, z4);
	vMax = Maths::Max(vMax, z5);
	vMax = Maths::Max(vMax, z6);
	vMax = Maths::Max(vMax, z7);

	F32 fMin, fMax;
	Maths::Store(vMin, &fMin);
	Maths::Store(vMax, &fMax);

	fMin = Maths::Clamp01(fMin);
	fMax = Maths::Clamp01(fMax);

#if defined(PLATFORM_PS3)
	if (g_debugPrepassDepthBounds)
		cellGcmSetDepthBoundsTestEnable(CELL_GCM_TRUE);
	cellGcmSetDepthBounds(fMin, fMax);
#endif
}

void C_SpotLights::UnbindDepthBounds() const
{
#if defined(PLATFORM_PS3)
	cellGcmSetDepthBoundsTestEnable(CELL_GCM_FALSE);
#endif
}
