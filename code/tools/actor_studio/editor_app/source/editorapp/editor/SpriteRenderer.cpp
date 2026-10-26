#include "stdafx.h"

#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Core.h>

#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/SpriteRenderer.h>

#include <Display/Buffer/VertexDeclarations.h>
#include <Display/Shaders/Constants.h>

#include <EditorApp/Editor/ActorScene.h>
#include <algorithm>

EXTERN_SHADER(vsSpriteSolid);
EXTERN_SHADER(psSpriteSolid);
EXTERN_SHADER(psSpriteSolidGreyScale);
EXTERN_SHADER(vsSpriteSolidDepthBias);
EXTERN_SHADER(psSpriteSolidDepthBias);
EXTERN_SHADER(psSpriteSolidGreyScaleDepthBias);

namespace Editor
{

struct SpriteVertex
{
	F32 x, y, z;
	F32 dx, dy;
	F32 u, v;

	static const U32						m_ElementCount = 3;
	static Display::C_VertexElement			m_Elements[m_ElementCount];
	static Display::C_VertexDeclaration*	m_pDeclaration;
};

Display::C_VertexElement SpriteVertex::m_Elements[SpriteVertex::m_ElementCount] =
{
	Display::C_VertexElement(0, 0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0),
	Display::C_VertexElement(0, 12, Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_TexCoord, 0),
	VERTEXELEMENT_END()
};

Display::C_VertexDeclaration* SpriteVertex::m_pDeclaration = NULL;


bool C_SpriteInstance::TextureSort(const C_SpriteInstance& l, const C_SpriteInstance& r)
{
	if (l.m_SpriteTexture!=0 && r.m_SpriteTexture!=0)
		return Btl::Hash(l.m_SpriteTexture->GetFileName().Chars()) < Btl::Hash(r.m_SpriteTexture->GetFileName().Chars());
	else
		return l.m_SpriteTexture == 0;
}

bool C_SpriteInstance::DepthSortBackToFront(const C_SpriteInstance& l, const C_SpriteInstance& r)
{
	const Maths::HVec3 camPos = Display::Device()->GetView()->m_ViewSettings3D.GetCameraPosition();
	const Maths::HVec3 camZ = Display::Device()->GetView()->m_ViewSettings3D.GetCameraToWorld().AZ();

	const Maths::HF32 dl = camZ|(l.m_pivot - camPos);
	const Maths::HF32 dr = camZ|(r.m_pivot - camPos);

	return dl > dr;
}

bool C_SpriteInstance::DepthSortFrontToBack(const C_SpriteInstance& l, const C_SpriteInstance& r)
{
	const Maths::HVec3 camPos = Display::Device()->GetView()->m_ViewSettings3D.GetCameraPosition();
	const Maths::HVec3 camZ = Display::Device()->GetView()->m_ViewSettings3D.GetCameraToWorld().AZ();

	const Maths::HF32 dl = camZ|(l.m_pivot - camPos);
	const Maths::HF32 dr = camZ|(r.m_pivot - camPos);

	return dl < dr;
}


C_SpriteRenderer::C_SpriteRenderer()
: m_MaxInstanceCount(1000)
{
}

C_SpriteRenderer::~C_SpriteRenderer()
{
}

AftkRender::C_ResourcePtr<Display::C_RefTexture2D> C_SpriteRenderer::GetSprite(const Btl::String& filename)
{
	return AftkRender::C_Renderer::GetTextureManager().LoadTexture2DFromFile(filename.Chars(), 0);
}

bool C_SpriteRenderer::OnCreate()
{
	m_ShaderManager.LoadVertexShader("shaders/vsSpriteSolid.shader", REF_SHADER(vsSpriteSolid));
	m_ShaderManager.LoadPixelShader("shaders/psSpriteSolid.shader", REF_SHADER(psSpriteSolid));
	m_ShaderManager.LoadPixelShader("shaders/psSpriteSolidGreyScale.shader", REF_SHADER(psSpriteSolidGreyScale));
	m_ShaderManager.LoadVertexShader("shaders/vsSpriteSolidDepthBias.shader", REF_SHADER(vsSpriteSolidDepthBias));
	m_ShaderManager.LoadPixelShader("shaders/psSpriteSolidDepthBias.shader", REF_SHADER(psSpriteSolidDepthBias));
	m_ShaderManager.LoadPixelShader("shaders/psSpriteSolidGreyScaleDepthBias.shader", REF_SHADER(psSpriteSolidGreyScaleDepthBias));

	m_pVertexBuffer = new Display::C_VertexBuffer();
	SpriteVertex::m_pDeclaration = Display::Device()->CreateVertexDeclaration(SpriteVertex::m_Elements);

	return AftkRender::C_Renderer::OnCreate();
}

bool C_SpriteRenderer::OnCreateVolatile()
{
	CreateVertexBuffer();

	if (!AftkRender::C_Renderer::OnCreateVolatile())
		return false;


	return true;
}

void C_SpriteRenderer::OnDestroy()
{
	delete m_pVertexBuffer;
	
	AftkRender::C_Renderer::OnDestroy();
}

void C_SpriteRenderer::OnDestroyVolatile()
{
	Display::Device()->DestroyVertexBuffer(m_pVertexBuffer);

	AftkRender::C_Renderer::OnDestroyVolatile();
}

void C_SpriteRenderer::Clear()
{
	m_SpriteInstances.Clear();
}

void C_SpriteRenderer::Update(AftkRender::E_RendererPass pass)
{
	if (!m_SpriteInstances.Size())
		return;

	if (pass == AftkRender::E_RendererPass_VisibilityTest) 
		std::sort(m_SpriteInstances.Begin(), m_SpriteInstances.End(), C_SpriteInstance::DepthSortFrontToBack);
	else
	{
		//duplicate sprite instances

		U32 nSpriteInstances = (U32)m_SpriteInstances.Size();
		m_SpriteInstances.Reserve(2*nSpriteInstances);

		for (U32 i=0; i<nSpriteInstances; ++i)
			m_SpriteInstances.PushBack(m_SpriteInstances[i]);

		C_SpriteInstance *pSpriteInstances = &m_SpriteInstances[0];
		//sort the 1st half according to depth
		std::sort(pSpriteInstances, pSpriteInstances+nSpriteInstances, C_SpriteInstance::DepthSortBackToFront);
		//sort the 2nd half based on texture
		std::sort(pSpriteInstances+nSpriteInstances, pSpriteInstances+2*nSpriteInstances, C_SpriteInstance::TextureSort);
	}
	
	CreateVertexBuffer();
	PopulateVertexBuffer();
}

void C_SpriteRenderer::GetVisible(
			const RECT& region, 
			C_DgNodeSelection& selection)
{
	Display::Device()->SetVertexShader(AftkRender::C_Renderer::GetShaderManager().GetVertexShader("shaders/vsSpriteSolidDepthBias.shader"));
	Display::Device()->SetPixelShader(AftkRender::C_Renderer::GetShaderManager().GetPixelShader("shaders/psSpriteSolid.shader"));
	Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);
	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetFillMode(Display::E_FillMode_Solid);
	Display::Device()->SetAlphaTestRef(200);
	Display::Device()->SetDepthEnable(Display::E_Bool_False);

	Display::Device()->SetVertexDeclaration(SpriteVertex::m_pDeclaration);
	Display::Device()->SetVertexBuffer(0, m_pVertexBuffer, 0, sizeof(SpriteVertex));
	for (S32 i=0; i<m_SpriteInstances.Size(); ++i)
	{
		//BeginVisibilityTest(region);
		Display::Device()->SetScissorRect(region.left, region.top, region.right-region.left, region.bottom-region.top);
		Display::Device()->SetScissorTest(Display::E_Bool_True);
		m_pOcclusionQuery->Issue(D3DISSUE_BEGIN);
		m_InVisibilityTest = true;

		Display::Device()->SetTexture(0, m_SpriteInstances[i].m_SpriteTexture);
		Display::Device()->DrawPrimitive(Display::E_PrimitiveType_TriangleList, i*6, 6);

		if (EndVisibilityTest())
		{
			if (m_SpriteInstances[i].m_ComponentId != 0xffffffff)
			{
				Ut::Vector<U32> components;
				components.PushBack(m_SpriteInstances[i].m_ComponentId);
				selection.Add(EditorLib::C_Core::Instance().GetDgManager().FindNodeById(m_SpriteInstances[i].m_NodeId), components);
			}
			else 
			{
				selection.Add(EditorLib::C_Core::Instance().GetDgManager().FindNodeById(m_SpriteInstances[i].m_NodeId));
			}
		}
	}

	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	Display::Device()->SetAlphaTestRef(0);
}

void C_SpriteRenderer::Render()
{
	Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);
	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetFillMode(Display::E_FillMode_Solid);
	Display::Device()->SetAlphaTestRef(200);
	Display::Device()->SetVertexDeclaration(SpriteVertex::m_pDeclaration);
	Display::Device()->SetVertexBuffer(0, m_pVertexBuffer, 0, sizeof(SpriteVertex));

	const U32 nInstances = (U32)m_SpriteInstances.Size() / 2;
	{
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
		//the stencil pattern and reference values should already be initialised (by C_RenderManager)
		Display::Device()->SetStencilEnable(Display::E_Bool_True);

		Display::Device()->SetVertexShader(AftkRender::C_Renderer::GetShaderManager().GetVertexShader("shaders/vsSpriteSolidDepthBias.shader"));

		{
			U32 instanceCount = 0;
			E_SpriteEffect currentEffect = m_SpriteInstances.Size() ? m_SpriteInstances[0].m_Effect : E_SpriteEffect_None;
			AftkRender::C_ResourcePtr<Display::C_RefTexture2D> pCurrentTexture = m_SpriteInstances.Size() ? m_SpriteInstances[0].m_SpriteTexture : 0;
			for (U32 i=0; i<nInstances; ++i)
			{
				if ((m_SpriteInstances[i].m_SpriteTexture != pCurrentTexture || m_SpriteInstances[i].m_Effect != currentEffect) && instanceCount)
				{
					if (currentEffect == E_SpriteEffect_None)
						Display::Device()->SetPixelShader(AftkRender::C_Renderer::GetShaderManager().GetPixelShader("shaders/psSpriteSolidDepthBias.shader"));
					else
						Display::Device()->SetPixelShader(AftkRender::C_Renderer::GetShaderManager().GetPixelShader("shaders/psSpriteSolidGreyScaleDepthBias.shader"));

					Display::Device()->SetTexture(0, pCurrentTexture);
					Display::Device()->DrawPrimitive(Display::E_PrimitiveType_TriangleList, (i-instanceCount)*6, instanceCount*6);
					pCurrentTexture = m_SpriteInstances[i].m_SpriteTexture;
					currentEffect = m_SpriteInstances[i].m_Effect;
					instanceCount = 1;
				}
				else
				{
					instanceCount++;
				}
			}

			if (instanceCount)
			{
				if (currentEffect == E_SpriteEffect_None)
					Display::Device()->SetPixelShader(AftkRender::C_Renderer::GetShaderManager().GetPixelShader("shaders/psSpriteSolidDepthBias.shader"));
				else
					Display::Device()->SetPixelShader(AftkRender::C_Renderer::GetShaderManager().GetPixelShader("shaders/psSpriteSolidGreyScaleDepthBias.shader"));

				Display::Device()->SetTexture(0, pCurrentTexture);
				Display::Device()->DrawPrimitive(Display::E_PrimitiveType_TriangleList, (nInstances-instanceCount)*6, instanceCount*6);
			}
		}

		Display::Device()->SetStencilEnable(Display::E_Bool_False);
		Display::Device()->SetDepthBias(0.f);
		Display::Device()->SetSlopeScaledBias(0.f);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	}

	{
		Display::Device()->SetVertexShader(AftkRender::C_Renderer::GetShaderManager().GetVertexShader("shaders/vsSpriteSolid.shader"));

		U32 instanceCount = 0;
		E_SpriteEffect currentEffect = m_SpriteInstances.Size() ? m_SpriteInstances[nInstances].m_Effect : E_SpriteEffect_None;
		AftkRender::C_ResourcePtr<Display::C_RefTexture2D> pCurrentTexture = m_SpriteInstances.Size() ? m_SpriteInstances[nInstances].m_SpriteTexture : 0;
		for (U32 i=nInstances; i<2*nInstances; ++i)
		{
			if ((m_SpriteInstances[i].m_SpriteTexture != pCurrentTexture || m_SpriteInstances[i].m_Effect != currentEffect) && instanceCount)
			{
				if (currentEffect == E_SpriteEffect_None)
					Display::Device()->SetPixelShader(AftkRender::C_Renderer::GetShaderManager().GetPixelShader("shaders/psSpriteSolid.shader"));
				else
					Display::Device()->SetPixelShader(AftkRender::C_Renderer::GetShaderManager().GetPixelShader("shaders/psSpriteSolidGreyScale.shader"));

				Display::Device()->SetTexture(0, pCurrentTexture);

				Display::Device()->DrawPrimitive(Display::E_PrimitiveType_TriangleList, (i-instanceCount)*6, instanceCount*6);
				pCurrentTexture = m_SpriteInstances[i].m_SpriteTexture;
				currentEffect = m_SpriteInstances[i].m_Effect;
				instanceCount = 1;
			}
			else
			{
				instanceCount++;
			}
		}

		if (instanceCount)
		{
			if (currentEffect == E_SpriteEffect_None)
				Display::Device()->SetPixelShader(AftkRender::C_Renderer::GetShaderManager().GetPixelShader("shaders/psSpriteSolid.shader"));
			else
				Display::Device()->SetPixelShader(AftkRender::C_Renderer::GetShaderManager().GetPixelShader("shaders/psSpriteSolidGreyScale.shader"));

			Display::Device()->SetTexture(0, pCurrentTexture);
			Display::Device()->DrawPrimitive(Display::E_PrimitiveType_TriangleList, (2*nInstances-instanceCount)*6, instanceCount*6);
		}
	}

	Display::Device()->SetAlphaTestRef(0);
	m_SpriteInstances.Clear();
}

void C_SpriteRenderer::Draw(
	const Btl::String& spriteFile, 
	E_SpriteEffect effect,
	float spriteSize, 
	const Maths::HMat44* pXform, 
	const EditorLib::C_NodeId& id, 
	U32 componentId)
{
	C_SpriteInstance instance;
	instance.m_SpriteTexture = GetSprite(spriteFile);

	if (!instance.m_SpriteTexture)
		return;

	Maths::C_AxisBox bounds(Maths::HVec3(pXform->AW().X()-spriteSize, pXform->AW().Y()-spriteSize, pXform->AW().Z()-spriteSize),
							Maths::HVec3(pXform->AW().X()+spriteSize, pXform->AW().Y()+spriteSize, pXform->AW().Z()+spriteSize));

	const Display::C_ViewFrustum& frustum = Display::Device()->GetViewFrustum();
	S32 cullResult = frustum.CullAxisBox(bounds);

	if (cullResult > 0)
	{
		instance.m_Effect = effect;
		instance.m_SpriteSize = spriteSize;
		instance.m_pivot = Maths::HVec3(pXform->AW().X(), pXform->AW().Y(), pXform->AW().Z());
		instance.m_NodeId = id;
		instance.m_ComponentId = componentId;
		m_SpriteInstances.PushBack(instance);
	}
}

bool C_SpriteRenderer::CreateVertexBuffer()
{
	if (m_pVertexBuffer->IsCreated())
	{
		if (m_pVertexBuffer->GetSize() < 6*m_SpriteInstances.Size()*sizeof(SpriteVertex))
		{
			Display::Device()->DestroyVertexBuffer(m_pVertexBuffer);
			m_MaxInstanceCount = m_SpriteInstances.Size();
		}
		else
			return true;
	}

	return Display::Device()->CreateVertexBuffer(
		6*m_MaxInstanceCount*sizeof(SpriteVertex), Display::E_Usage_Static|Display::E_Usage_Video, m_pVertexBuffer);
}

void C_SpriteRenderer::PopulateVertexBuffer()
{
	assert(m_pVertexBuffer->IsCreated());

	if (!m_SpriteInstances.Size())
		return;

	SpriteVertex *pVertex = (SpriteVertex*)m_pVertexBuffer->Lock();

	for (S32 i=0; i<m_SpriteInstances.Size(); ++i)
	{
		const C_SpriteInstance& sprite = m_SpriteInstances[i];
		SpriteVertex v[4] = 
		{
			{sprite.m_pivot[0], sprite.m_pivot[1], sprite.m_pivot[2], 0.f, sprite.m_SpriteSize, 0.f, 0.f},
			{sprite.m_pivot[0], sprite.m_pivot[1], sprite.m_pivot[2], sprite.m_SpriteSize, sprite.m_SpriteSize, 1.f, 0.f},
			{sprite.m_pivot[0], sprite.m_pivot[1], sprite.m_pivot[2], sprite.m_SpriteSize, 0.f, 1.f, 1.f},
			{sprite.m_pivot[0], sprite.m_pivot[1], sprite.m_pivot[2], 0.f, 0.f, 0.f, 1.f}
		};

		pVertex[0] = v[0];
		pVertex[1] = v[1];
		pVertex[2] = v[2];
		pVertex[3] = v[0];
		pVertex[4] = v[2];
		pVertex[5] = v[3];

		pVertex += 6;
	}

	m_pVertexBuffer->Unlock();
}

}
