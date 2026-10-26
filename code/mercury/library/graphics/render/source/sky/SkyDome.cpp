#include "stdafx.h"
#include "Render/Sky/SkyDome.h"
#include "Render/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include <System/Debug/Variables.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

EXTERN_SHADER(vsSkyDome);
EXTERN_SHADER(psSkyDome);
EXTERN_SHADER(psSkyDomeHDR);
EXTERN_SHADER(psSkyDomeHDRU32);

namespace Render
{
namespace Sky
{

::Debug::Bool	g_debugSkyDomeEnabled		("Graphics.Render.SkyDome.Enabled",			true);
::Debug::Bool	g_debugSkyDomeWireframe		("Graphics.Render.SkyDome.Wireframe",		false);
::Debug::Int	g_debugSkyDomeRingCount		("Graphics.Render.SkyDome.RingCount",		25,		::Debug::Range(1, 50));
::Debug::Int	g_debugSkyDomeSegmentCount	("Graphics.Render.SkyDome.SegmentCount",	25,		::Debug::Range(1, 50));
::Debug::Float  g_debugSkyDomeHeight        ("Graphics.Render.SkyDome.Height",			1.0f,	::Debug::Range(-1.0f, 1.0f ));
::Debug::Float  g_debugSkyDomeOffset        ("Graphics.Render.SkyDome.Offset",			0.0f,	::Debug::Range(-1.0f, 1.0f ));
::Debug::Float  g_debugSkyDomeFogTop		("Graphics.Render.SkyDome.FogTop",			1.0f,	::Debug::Range(0.0f, 1.0f));
::Debug::Float  g_debugSkyDomeFogAlpha		("Graphics.Render.SkyDome.FogAlpha",		1.0f,	::Debug::Range(0.0f, 1.0f));
::Debug::Float  g_debugSkyDomeFogPower		("Graphics.Render.SkyDome.FogPower",		20.0f,	::Debug::Range(0.0f, 100.0f));
::Debug::Float  g_debugSkyDomeRotation		("Graphics.Render.SkyDome.Rotation",		0.0f,	::Debug::Range(-1.0f, 1.0f ));
::Debug::Float	g_debugSkyDomeIntensity		("Graphics.Render.SkyDome.Intensity",		1.f,	::Debug::Range(0.f, 10.f));
::Debug::Float	g_debugSkyDomeBlackHeight	("Graphics.Render.SkyDome.BlackHeight",		3.0f,	::Debug::Range(0.f, 10.f));
::Debug::Float	g_debugSkyDomeBlackSize		("Graphics.Render.SkyDome.BlackSize",		1.f,	::Debug::Range(0.f, 10.f));

// C_SkyDomeVertex

struct C_SkyDomeVertex
{
	F32 x, y, z;
	F32 u, v;
};

// C_SkyDome

C_SkyDome::C_SkyDome() : Display::I_Renderer("SkyDome"),
	m_Rotation(0.f),
	m_Intensity(-1.f),
	m_VertexCount(0),
	m_IndexCount(0),
	m_TextureFileName("Textures/Sunny.dds")
{
	m_FogColour = Maths::C_RGBA_F32(0.5f, 0.5f, 0.5f, 0.5f);

	Display::C_VertexFormat vertexFormat;
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0);
	m_pVertexDeclaration = Display::Device()->CreateVertexDeclaration(vertexFormat);

	assert(g_pSkyDome == NULL);
	g_pSkyDome = this;
}

C_SkyDome::~C_SkyDome()
{
	DestroyRenderer();

	assert(g_pSkyDome == this);
	g_pSkyDome = NULL;
}

bool C_SkyDome::OnCreate()
{
	RESOURCE_MEMORY_ID("SkyDome");

	m_VertexCount = (g_debugSkyDomeRingCount + 1) * (g_debugSkyDomeSegmentCount + 1);
	m_IndexCount = (g_debugSkyDomeRingCount * (g_debugSkyDomeSegmentCount + 1)) * 2;
	
	if(!Display::Device()->CreateVertexBuffer(m_VertexCount * sizeof(C_SkyDomeVertex), 0, &m_VertexBuffer))
		return false;
	if(!Display::Device()->CreateIndexBuffer(m_IndexCount * sizeof(U16), 0, Display::E_Format_Index16, &m_IndexBuffer))
		return false;
	#if defined(DEVICE_PS3GCM)
	if(!Display::CreateVertexShaderFromFunc("Shaders/vsSkyDome", 0, &m_VertexShader, REF_SHADER(vsSkyDome)))
		return false;
	if(!Display::CreatePixelShaderFromFunc("Shaders/psSkyDome", 0, &m_PixelShader[E_SkyDomeShader_Default], REF_SHADER(psSkyDome)))
		return false;
	if(!Display::CreatePixelShaderFromFunc("Shaders/psSkyDomeHDR", 0, &m_PixelShader[E_SkyDomeShader_HDR], REF_SHADER(psSkyDomeHDR)))
		return false;
	if(!Display::CreatePixelShaderFromFunc("Shaders/psSkyDomeHDRU32", 0, &m_PixelShader[E_SkyDomeShader_HDRU32], REF_SHADER(psSkyDomeHDRU32)))
		return false;
	#endif
	m_vsSkySettings		= m_VertexShader.GetParameter(CV_RENDER_SETTINGS,			"g_SkySettings");
	m_vsSkySettings2	= m_VertexShader.GetParameter(CV_RENDER_SETTINGS_1,			"g_SkySettings2");
	m_vsFogSettings		= m_VertexShader.GetParameter(CV_RENDER_SETTINGS_3,			"g_FogSettings");
	m_vsWorldMatrix		= m_VertexShader.GetParameter(CV_DISPLAY_WORLD_MATRIX,		"g_WorldMatrix");
	m_vsViewProjMatrix	= m_VertexShader.GetParameter(CV_DISPLAY_VIEWPROJ_MATRIX,	"g_ViewProjMatrix");

	for(U32 i=0; i<E_SkyDomeShader_Count; ++i)
	{
		m_psSkySettings[i]	= m_PixelShader[i].GetParameter(CP_RENDER_SETTINGS,					"g_SkySettings");
		m_psFogColour[i]	= m_PixelShader[i].GetParameter(CP_RENDER_SETTINGS_1,				"g_FogColour");
		m_psLuminance[i]	= m_PixelShader[i].GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE,	"g_vLuminance");
	}

	Build();
		
	if(!Display::CreateTexture2DFromFile(m_TextureFileName, &m_Texture))
		Display::CreateTexture2DFromFile(Util::C_String("DontShip/") + m_TextureFileName, &m_Texture);
	


	return true;
}

void C_SkyDome::OnDestroy()
{
	Display::Device()->Destroy(&m_VertexBuffer);
	Display::Device()->Destroy(&m_IndexBuffer);
	Display::Device()->Destroy(&m_Texture);
	Display::Device()->Destroy(&m_VertexShader);

	for(U32 i=0; i<E_SkyDomeShader_Count; ++i)
	{
		Display::Device()->Destroy(&m_PixelShader[i]);
	}
}

bool C_SkyDome::CreateTexture()
{
	RESOURCE_MEMORY_ID("SkyDome");

	return Display::CreateTexture2DFromFile(m_TextureFileName, &m_Texture);
}

void C_SkyDome::DestroyTexture()
{
	Display::Device()->Destroy(&m_Texture);
}

void C_SkyDome::Render() const
{
	if(!g_debugSkyDomeEnabled)
		return;

	PERF_EVENT("Effect.RenderSkyDome");

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	
	Display::Device()->SetVertexShader(&m_VertexShader);

	F32 rotation = m_Rotation + g_debugSkyDomeRotation;
	F32 intensity = m_Intensity == -1.f ? g_debugSkyDomeIntensity : m_Intensity;
	F32 blackHeight = g_debugSkyDomeBlackHeight;
	F32 blackSize = g_debugSkyDomeBlackSize;

	// turn off if no value set
	if(blackHeight == 0.0f)
	{
		blackHeight = 3.0f;
		blackSize = 1.0f;
	}

	Display::C_Float4 vsSkySettings(viewSettings3D.GetFarClip() * 0.9f, g_debugSkyDomeHeight, g_debugSkyDomeOffset, 0.f);
    Display::Device()->SetVertexShaderParameterF(m_vsSkySettings, (F32*)&vsSkySettings, 1);
	Display::C_Float4 vsSkySettings2(rotation, intensity, blackHeight, blackSize);
	Display::Device()->SetVertexShaderParameterF(m_vsSkySettings2, (F32*)&vsSkySettings2, 1);
	Display::C_Float4 vsFogSettings(g_debugSkyDomeFogAlpha, g_debugSkyDomeFogTop, g_debugSkyDomeFogPower, 0.0f);
	Display::Device()->SetVertexShaderParameterF(m_vsFogSettings, (F32*)&vsFogSettings, 1);


	Display::C_Matrix worldMatrix;
	Display::MatrixSetIdentity(&worldMatrix);
	Display::MatrixSetTranslation(&worldMatrix, viewSettings3D.GetCameraPosition().X(), viewSettings3D.GetCameraPosition().Y(), viewSettings3D.GetCameraPosition().Z());
	Display::Device()->SetVertexShaderParameterF(m_vsWorldMatrix, (F32*)&worldMatrix, 4);
	Display::Device()->SetVertexShaderParameterF(m_vsViewProjMatrix, (F32*)&viewSettings3D.GetViewProjMatrix(), 4);

	E_SkyDomeShader shader = E_SkyDomeShader_Default;
	if(Render::C_RenderState::Instance().ShaderHDR())
	{
		if(Render::C_RenderState::Instance().HDR()->GetType() == PostProcess::E_HighDynamicRangeType_Shader)
			shader = E_SkyDomeShader_HDR;
		else
			shader = E_SkyDomeShader_HDRU32;
	}
	else
	{
		shader = E_SkyDomeShader_Default;
	}

	Display::C_Float4 psSkySettings(rotation, intensity, blackHeight, blackSize);
    Display::Device()->SetPixelShaderParameterF(m_psSkySettings[shader], (F32*)&psSkySettings);
	Display::C_Float4 psFogColour(m_FogColour.R(), m_FogColour.G(), m_FogColour.B(), m_FogColour.A());
    Display::Device()->SetPixelShaderParameterF(m_psFogColour[shader], (F32*)&psFogColour);
	
	if(Render::C_RenderState::Instance().ShaderHDR())
	{
		Display::C_Float4 psLuminance = Render::C_RenderState::Instance().HDR()->GetShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_psLuminance[shader], (F32*)&psLuminance);
		
		Display::Device()->SetTexture(15, Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture());
		Display::Device()->SetTextureFilter(15, Display::E_TextureFilter_Point);
		Display::Device()->SetTextureAddress(15, Display::E_TextureAddress_Clamp);
	}
	else
	{
		Display::C_Float4 psGain = Render::C_RenderState::Instance().GetGainShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_psLuminance[shader], (F32*)&psGain);
	}

	Display::Device()->SetPixelShader(&m_PixelShader[shader]);

	if(m_Texture.IsCreated())
	{
		Display::Device()->SetTexture(0, &m_Texture);
	}

	Display::Device()->SetTextureFilterMagMinMip(0, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddressUVW(0, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Wrap);
	
	Display::Device()->SetVertexDeclaration(m_pVertexDeclaration);
	Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(C_SkyDomeVertex));
	Display::Device()->SetIndexBuffer(&m_IndexBuffer);

	#ifndef CFG_SPEED
	if(g_debugSkyDomeWireframe)
	{
		Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);
	}
	#endif
	
	Display::Device()->DrawIndexedPrimitive(Display::E_PrimitiveType_TriangleStrip, 0, 0, m_VertexCount, 0, m_IndexCount);

	#ifndef CFG_SPEED
	if(g_debugSkyDomeWireframe)
	{
		Display::Device()->SetFillMode(Display::E_FillMode_Solid);
	}
	#endif

	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
}

void C_SkyDome::Build()
{
	C_SkyDomeVertex* pVertex = (C_SkyDomeVertex*)m_VertexBuffer.Lock();
	assert(pVertex);

	U16* pIndex = (U16*)m_IndexBuffer.Lock();
	assert(pIndex);
	
	U16 vertexIndex = 0;
    F32 deltaRingAngle = Maths::Pi / g_debugSkyDomeRingCount;
	F32 deltaSegmentAngle = 2.f * Maths::Pi / g_debugSkyDomeSegmentCount;

	for(S32 ringIndex=0; ringIndex<g_debugSkyDomeRingCount+1; ringIndex++)
	{
		F32 r = sinf(ringIndex * deltaRingAngle);
		F32 y = cosf(ringIndex * deltaRingAngle);

		for(S32 segmentIndex=0; segmentIndex<g_debugSkyDomeSegmentCount+1; segmentIndex++)
		{
			F32 x = r * sinf(segmentIndex * deltaSegmentAngle);
			F32 z = r * cosf(segmentIndex * deltaSegmentAngle);

			pVertex->x = x;
			pVertex->y = y;
			pVertex->z = z;
			pVertex->u = (F32)segmentIndex / (F32)g_debugSkyDomeSegmentCount;
			pVertex->v = (F32)ringIndex * 2.f / (F32)g_debugSkyDomeRingCount;
			pVertex++;
			
			if(ringIndex != g_debugSkyDomeRingCount)
			{
				*pIndex = vertexIndex + (U16)(g_debugSkyDomeSegmentCount + 1);
				pIndex++;
				
				*pIndex = vertexIndex;
				pIndex++;

				vertexIndex++;
			}
		}
	}

	m_VertexBuffer.Unlock();
	m_IndexBuffer.Unlock();
}

C_SkyDome* g_pSkyDome = NULL;

void SkyDome(C_SkyDome* pSkyDome)
{
	g_pSkyDome = pSkyDome;
}

} // namespace Sky
} // namespace Render
