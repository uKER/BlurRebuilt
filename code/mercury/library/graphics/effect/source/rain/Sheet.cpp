#include "stdafx.h"

#include "Effect/Rain/Sheet.h"
#include "Effect/Rain/Drop.h"
#include "Effect/Shaders/Constants.h"

#include <Display/Buffer/VertexDeclarations.h>
#include <Render/View/ViewManager.h>

EXTERN_SHADER(vsSheet);
EXTERN_SHADER(psRainSheet);

using namespace Effect::Rain;

namespace Effect
{
namespace Rain
{

	Debug::Bool		g_debugSheetEnable("Graphics.Render.Effects.Rain.Sheet.Enable", true);
	Debug::Bool		g_debugSheetWireframe("Graphics.Render.Effects.Rain.Sheet.Wireframe", false);

	Debug::Int		g_debugSheetSphereRingCount("Graphics.Render.Effects.Rain.Sheet.SphereRingCount", 25, Debug::Range(1, 50));
	Debug::Int		g_debugSheetSphereSegmentCount("Graphics.Render.Effects.Rain.Sheet.SphereSegmentCount", 25, Debug::Range(1, 50));
	Debug::Float	g_debugSheetSphereHeight("Graphics.Render.Effects.Rain.Sheet.SphereHeight", 1.0f, Debug::Range(0.f, 5.f), Debug::Step(0.1f));

	Debug::Float	g_debugSheetColourScale("Graphics.Render.Effects.Rain.Sheet.ColourScale", 1.0f, Debug::Range(0.f, 5.f), Debug::Step(0.1f));
	Debug::Float	g_debugSheetAlphaPow("Graphics.Render.Effects.Rain.Sheet.AlphaPow", 1.0f, Debug::Range(0.f, 5.f), Debug::Step(0.1f));
	
	Debug::Float	g_debugSheetUVScale("Graphics.Render.Effects.Rain.Sheet.UVScale", 1.0f, Debug::Range(0.f, 10.f), Debug::Step(0.1f));
	Debug::Float	g_debugSheetSpeed0("Graphics.Render.Effects.Rain.Sheet.Speed0", 0.025f, Debug::Range(0.f, 1.f), Debug::Step(0.025f));
	Debug::Float	g_debugSheetSpeed1("Graphics.Render.Effects.Rain.Sheet.Speed1", 0.05f, Debug::Range(0.f, 1.f), Debug::Step(0.025f));
	Debug::Float	g_debugSheetSpeed2("Graphics.Render.Effects.Rain.Sheet.Speed2", 0.075f, Debug::Range(0.f, 1.f), Debug::Step(0.025f));
	Debug::Float	g_debugSheetSpeed3("Graphics.Render.Effects.Rain.Sheet.Speed3", 0.1f, Debug::Range(0.f, 1.f), Debug::Step(0.025f));
	Debug::Float	g_debugSheetSpeedX("Graphics.Render.Effects.Rain.Sheet.SpeedX", 0.25f, Debug::Range(0.f, 1.f), Debug::Step(0.025f));
	typedef Display::Vertex_XYZ_C_UV C_Vertex;
}
}

C_SheetRenderer::C_SheetRenderer() : Display::I_Renderer("RainSheet")
{
	for(U32 i = 0; i < 4; ++i)
	{
		m_fWind[i] = 0.0f;
		m_fHeight[i] = 0.0f;
	}

}

C_SheetRenderer::~C_SheetRenderer()
{
	DestroyRenderer();
}

void C_SheetRenderer::SetDefaults()
{
	
}
bool C_SheetRenderer::OnCreate()
{
	RESOURCE_MEMORY_ID("RainSheet");

	if (!Display::CreateTexture2DFromFile("Textures/RainSheet.dds", &m_Texture))
		return false;

	return true;
}

void C_SheetRenderer::OnDestroy()
{
	Display::Device()->Destroy(&m_VertexBuffer);
	Display::Device()->Destroy(&m_IndexBuffer);
	Display::Device()->Destroy(&m_Texture);
	Display::Device()->Destroy(&m_VertexShader);
	Display::Device()->Destroy(&m_PixelShader);
}

bool C_SheetRenderer::Initialise()
{
	m_iVertexCount = (g_debugSheetSphereRingCount + 1) * (g_debugSheetSphereSegmentCount + 1);
	m_iIndexCount = (g_debugSheetSphereRingCount * (g_debugSheetSphereSegmentCount + 1)) * 2;
	
	if (!Display::Device()->CreateVertexBuffer(m_iVertexCount * sizeof(C_Vertex), 0, &m_VertexBuffer))
		return false;
	if (!Display::Device()->CreateIndexBuffer(m_iIndexCount * sizeof(U16), 0, Display::E_Format_Index16, &m_IndexBuffer))
		return false;

	Build();

	return true;
}

void C_SheetRenderer::Update(const F32 fTime, const bool bPaused)
{
	if(!bPaused)
	{
		m_fHeight[0] -= fTime * g_debugSheetSpeed0;
		m_fHeight[1] -= fTime * g_debugSheetSpeed1;
		m_fHeight[2] -= fTime * g_debugSheetSpeed2;
		m_fHeight[3] -= fTime * g_debugSheetSpeed3;

		// TODO: fix this up to work properly
		const Display::C_Matrix viewMatrix = Render::C_ViewManager::Instance().GetView(0)->m_View.GetViewSettings3D().GetViewMatrix();
		Maths::HF32 vDot;
		if(C_DropRenderer::Instance().Initialised())
		{
			Maths::HVec3 vNoise = C_DropRenderer::Instance().GetNoise();
			Maths::HVec3 vRight(viewMatrix._11, viewMatrix._21, viewMatrix._31);
			vDot = vNoise | vRight;
		}

		m_fWind[0] += fTime * g_debugSheetSpeed0 * vDot * g_debugSheetSpeedX;
		m_fWind[1] += fTime * g_debugSheetSpeed1 * vDot * g_debugSheetSpeedX;
		m_fWind[2] += fTime * g_debugSheetSpeed2 * vDot * g_debugSheetSpeedX;
		m_fWind[3] += fTime * g_debugSheetSpeed3 * vDot * g_debugSheetSpeedX;
	}
}

void C_SheetRenderer::Build()
{
	C_Vertex* pVertex = (C_Vertex*)m_VertexBuffer.Lock();
	assert(pVertex);

	U16* pIndex = (U16*)m_IndexBuffer.Lock();
	assert(pIndex);
	
	U16 vertexIndex = 0;
    F32 deltaRingAngle = Maths::Pi / g_debugSheetSphereRingCount;
	F32 deltaSegmentAngle = 2.f * Maths::Pi / g_debugSheetSphereSegmentCount;

	for (S32 ringIndex = 0; ringIndex < g_debugSheetSphereRingCount + 1; ringIndex++)
	{
		F32 r = sinf(ringIndex * deltaRingAngle);
		F32 y = cosf(ringIndex * deltaRingAngle);

		for (S32 segmentIndex = 0; segmentIndex < g_debugSheetSphereSegmentCount + 1; segmentIndex++)
		{
			F32 x = r * sinf(segmentIndex * deltaSegmentAngle);
			F32 z = r * cosf(segmentIndex * deltaSegmentAngle);
			F32 a = (F32)ringIndex / (F32)g_debugSheetSphereRingCount;
			
			if(a >= 0.5f)
				a = (1.0f - a) * 0.5f;
			else
				a *= 0.5f;
			
			a *= 2.0f;
			//a = sqrtf(a);

			pVertex->x = x;
			pVertex->y = y * g_debugSheetSphereHeight;
			pVertex->z = z;
			pVertex->u = (F32)segmentIndex / (F32)g_debugSheetSphereSegmentCount;
			pVertex->v = (F32)ringIndex * 2.f / (F32)g_debugSheetSphereRingCount * g_debugSheetSphereHeight;
			pVertex->c = ((U8)(a * 255.0f) << 24) + (255 << 16) + (255 << 8) + 255;
			pVertex++;
			
			if (ringIndex != g_debugSheetSphereRingCount)
			{
				*pIndex = vertexIndex + (U16)(g_debugSheetSphereSegmentCount + 1);
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

void C_SheetRenderer::Display() const
{
	if(g_debugSheetEnable)
	{
		PERF_EVENT("Rain::Sheet::Display");
	
		const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();
		const Display::C_Matrix viewMatrix = viewSettings3D.GetViewMatrix();
		// current matrix
		{
			// load this into HMat for processing
			Maths::HMat43 mViewOrigin(Maths::Load<Maths::HMat44>(&viewSettings3D.GetViewMatrix()));
			Maths::HMat44 mProj(Maths::Load<Maths::HMat44>(&viewSettings3D.GetProjMatrix()));
			mViewOrigin.Translation(Maths::HVec3(0.f, 0.f, 0.f));

			// transpose and multiply
			Maths::HMat44 mViewProjOrigin = Maths::HMat44(mViewOrigin) * mProj;

			Display::C_Matrix viewProjOriginMatrix;
			Maths::Store(mViewProjOrigin, &viewProjOriginMatrix);
			Display::Device()->SetVertexShaderConstantF(CV_RENDER_VIEWPROJORIGIN_MATRIX, (F32*)&viewProjOriginMatrix, 4);
		}

		Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

		// shader constants
		{
			MathsStorage::Float4 vColourScale(g_debugSheetColourScale, g_debugSheetColourScale, g_debugSheetColourScale, g_debugSheetAlphaPow);
			MathsStorage::Float4 vUVScale(g_debugSheetUVScale, g_debugSheetUVScale, g_debugSheetUVScale, g_debugSheetUVScale);
			
			MathsStorage::Float4 vHeight(m_fHeight[0], m_fHeight[1], m_fHeight[2], m_fHeight[3]);
			MathsStorage::Float4 vWind(m_fWind[0], m_fWind[1], m_fWind[2], m_fWind[3]);
						
			Display::Device()->SetVertexShaderConstantF(CV_EFFECT_RAIN_COLOURSCALE, (F32*)&vColourScale, 1);
			Display::Device()->SetVertexShaderConstantF(CV_EFFECT_RAIN_SIZESCALE, (F32*)&vUVScale, 1);
			Display::Device()->SetVertexShaderConstantF(CV_EFFECT_RAIN_UP, (F32*)&vHeight, 1);
			
			Display::Device()->SetVertexShaderConstantF(CV_EFFECT_RAIN_RIGHT, (F32*)&vWind, 1);
					
			
			Display::Device()->SetPixelShaderConstantF(CP_EFFECT_RAIN_HEIGHT, (F32*)&vHeight, 1);
		}

		if(g_debugSheetWireframe)
			Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);

		Display::Device()->SetVertexShader(&m_VertexShader);
		Display::Device()->SetPixelShader(&m_PixelShader);
		
		Display::Device()->SetTexture(0, &m_Texture);
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);
		Display::Device()->SetVertexDeclaration(C_Vertex::m_pDeclaration);
		Display::Device()->SetVertexBuffer(0, &m_VertexBuffer, 0, sizeof(C_Vertex));
		Display::Device()->SetIndexBuffer(&m_IndexBuffer);

		Display::Device()->DrawIndexedPrimitive(Display::E_PrimitiveType_TriangleStrip, 0, 0, m_iVertexCount, 0, m_iIndexCount);

		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);

		if(g_debugSheetWireframe)
			Display::Device()->SetFillMode(Display::E_FillMode_Solid);
	}
}


