#include "stdafx.h"
// header

#include "Display/Render/BasicRender.h"
#include "Display/Buffer/VertexDeclarations.h"

#include "Dynamic/Renderer.h"
#include "Dynamic/Minimal/Minimal.h"

#include "Render/State/RenderState.h"
#include "Render/View/ViewManager.h"

#include "SceneGraph/View.h"

// constants
#include "Dynamic/Shaders/Constants.h"

#include <system\Debug\I_DebugRender.h>

EXTERN_SHADER(vsMinimal);
EXTERN_SHADER(psMinimal);
EXTERN_SHADER(psMinimalHDR);
EXTERN_SHADER(psMinimalHDRU32);

namespace DynamicEffect
{
typedef Display::Vertex_XYZ_HDR_UV_N	MinimalVertex;

bool C_Minimal::Create()
{
	if (!Display::CreateVertexShaderFromFunc("vsMinimal", 0, &m_VertexShader, REF_SHADER(vsMinimal)))
		return false;

	if(Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		// choose which set of shaders we need to use - we only have depth fog for shader tonemap at the moment
		if(eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("psMinimalHDRU32", 0, &m_PixelShader, REF_SHADER(psMinimalHDRU32)))
				return false;
		}
		else if(eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psMinimalHDR", 0, &m_PixelShader, REF_SHADER(psMinimalHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psMinimal", 0, &m_PixelShader, REF_SHADER(psMinimal)))
			return false;
	}

	m_LuminanceParam = m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");

	m_pRenderState = &Render::C_RenderState::Instance();

	return true;
}

void C_Minimal::Destroy()
{
	if(m_VertexShader.IsCreated())
		Display::Device()->Destroy(&m_VertexShader);
	if(m_PixelShader.IsCreated())
		Display::Device()->Destroy(&m_PixelShader);
}


void C_Minimal::AddObject(const Maths::C_RGBA_F32& colour, const Maths::HMat43& transform, bool transparent)
{
	m_Objects.PushBack();

	C_MinimalObject& object = m_Objects.Back();

	Maths::C_AxisBox axisBox(Maths::HVec3(-1.0f, -1.0f, -1.0f), Maths::HVec3(1.0f, 1.0f, 1.0f));
	object.SetBounds(axisBox, 1000.0f);

	object.m_pEffect = this;
	object.m_pNode = NULL;

	object.m_Colour = colour;
	Maths::Store<Maths::HMat44>(Maths::HMat44(transform), &object.m_Transform);
	object.m_Transparent = transparent;

	++m_nBlocks;

}

U32 C_Minimal::GetObjectCount() const
{
	return m_Objects.Size();
}

const C_MinimalObject* C_Minimal::GetObject(U32 index) const
{
	if(index < (U32) m_Objects.Size())
		return &m_Objects[index];
	else
		return NULL;
}

void C_Minimal::ClearObjects()
{
	m_Objects.Clear();

	m_nBlocks = 0;
}

void C_Minimal::OnGather(const SceneGraph::C_View& view) const
{
	for(S32 i = 0; i < m_Objects.Size(); ++i)
	{
		const C_MinimalObject& object = m_Objects[i];

		Maths::HVec4 vCentreOne(Maths::Load<Maths::HVec4>(&object.m_vCentre));
		Maths::HVec4 vSizeCull(Maths::Load<Maths::HVec4>(&object.m_vSize));
		Maths::HMat43 mTransform(Maths::Load<Maths::HMat44>(&object.m_Transform));

		Maths::HVec3 vCentre3(vCentreOne);
		Maths::HVec3 vSize3(vSizeCull);
		Maths::HF32 length = Maths::Length(vSize3);

		Maths::HVec4 vCentre(vCentre3 * mTransform);
		Maths::HVec4 vSize(vSize3 * mTransform.Rotation());

		vCentre.W(vCentreOne.W());
		vSize.W(vSizeCull.W());

		S32 visible;
		visible = view.GetViewFrustum().CullSphere(vCentre, length, vSizeCull.W());

		if(visible)
		{
			const DynamicEffect::C_View& dynamicView = m_pRenderer->GetView(view.GetIndex());
			// TODO: filter nodes into relevant lists on scenegraph view
			view.AddVisibleNode(&object, 0);
		}
	}
}

/*
void C_Minimal::OnGatherBlocks(DynamicEffect::BlockArrays &blockArrays, const U32 iViewIndex) const
{
	Maths::HVec4 vViewDirection(Maths::Load<Maths::HVec4>(&m_pRenderer->GetViewDirection()));
	Maths::HF32 vDepthScale(Maths::Load<Maths::HF32>(&m_pRenderer->GetViewPosition().w));
	Maths::HF32 vMaxInt = (F32)0xFFFFFFFF;

	for(S32 i = 0; i < m_Objects.Size(); ++i)
	{
		const C_MinimalObject& object = m_Objects[i];

		Maths::HVec4 vCentreOne(Maths::Load<Maths::HVec4>(&object.m_vCentre));
		Maths::HVec4 vSizeCull(Maths::Load<Maths::HVec4>(&object.m_vSize));
		Maths::HMat43 mTransform(Maths::Load<Maths::HMat44>(&object.m_Transform));

		Maths::HVec3 vCentre3(vCentreOne);
		Maths::HVec3 vSize3(vSizeCull);
		Maths::HF32 length = Maths::Length(vSize3);

		Maths::HVec4 vCentre(vCentre3 * mTransform);
		Maths::HVec4 vSize(vSize3 * mTransform.Rotation());

		vCentre.W(vCentreOne.W());
		vSize.W(vSizeCull.W());

		S32 visible;
		visible = Render::C_ViewManager::Instance().GetView(iViewIndex)->m_View.GetViewFrustum().CullSphere(vCentre, length, vSizeCull.W());

		if(visible)
		{
			C_RenderNode	block;
			block.pBlock = &object;
			block.pMatrix = &object.m_Transform;
			block.pInstance = NULL;

			if(object.m_Transparent)
			{
				// get a sort key for depth sorting
				Maths::HVec4 vCentreOne(Maths::Load<Maths::HVec4>(&object.m_vCentre));
				Maths::HMat43 mTransform(Maths::Load<Maths::HMat44>(&object.m_Transform));

				Maths::HVec4 vCentre(Maths::HVec3(vCentreOne) * mTransform);
				vCentre.W(vCentreOne.W());
				Maths::HF32 vDepth = (vCentre | vViewDirection);
				vDepth = vMaxInt - vDepth * vDepthScale;
				block.sortKey = (U32)vDepth;

				// push onto transparent blocks
				blockArrays.transparent.PushBack(block);
			}
			else
			{
				block.sortKey = m_iIndex;	// sort by effect instead of depth
				blockArrays.opaque.PushBack(block);
			}
		}
	}
}*/

void C_Minimal::OnBuild(const C_View& view, C_CompositeRenderNode& node) const
{
	node.iVertexBuffer = 0xFFFFFFFF;
}


void C_Minimal::OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const
{
#ifndef PLATFORM_PC
	U32 nNumPrimitives = 6;
	U32 nNumVertices = 4;
	Display::E_PrimitiveType	ePrimitiveType = Display::E_PrimitiveType_QuadList;
#else
	U32 nNumPrimitives = 12;
	U32 nNumVertices = 3;
	Display::E_PrimitiveType	ePrimitiveType = Display::E_PrimitiveType_TriangleList;
#endif

	Maths::HVec3 vVertex0(1.0f, 1.0f, 1.0f);
	Maths::HVec3 vVertex1(-1.0f, 1.0f, 1.0f);
	Maths::HVec3 vVertex2(-1.0f, -1.0f, 1.0f);
	Maths::HVec3 vVertex3(1.0f, -1.0f, 1.0f);
	Maths::HVec3 vVertex4(1.0f, 1.0f, -1.0f);
	Maths::HVec3 vVertex5(-1.0f, 1.0f, -1.0f);
	Maths::HVec3 vVertex6(-1.0f, -1.0f, -1.0f);
	Maths::HVec3 vVertex7(1.0f, -1.0f, -1.0f);
	
	U32 indices[] = {	0, 1, 2, 3,
						4, 5, 1, 0,
						7, 6, 5, 4,
						3, 2, 6, 7,
						3, 7, 4, 0,
						5, 6, 2, 1 };

	U32 vertexCount = node.blocks.Size() * nNumPrimitives * nNumVertices;

	if(states & E_States_BindEffect)
	{
		Display::Device()->SetTexture(0, &m_pRenderer->GetGreyTexture());
		Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);

		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		Display::Device()->SetVertexShader(&m_VertexShader);
		Display::Device()->SetVertexDeclaration(MinimalVertex::m_pDeclaration);
		
		
		MathsStorage::Float4 gainParam = m_pRenderState->ShaderHDR() ? m_pRenderState->HDR()->GetShaderParameters() : m_pRenderState->GetGainShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_LuminanceParam, (F32*)&gainParam);
		Display::Device()->SetPixelShader(&m_PixelShader);

		MathsStorage::Float4 lightDirection(0.707f, 0.707f, 0.0f, 0.0f);
		Display::Device()->SetPixelShaderConstantF(CP_RENDER_LIGHT_DIRECTION, (F32*)&lightDirection, 1);
	}

	MinimalVertex* pVertices = (MinimalVertex*)Display::Device()->BeginVertices(ePrimitiveType, vertexCount, sizeof(MinimalVertex), MinimalVertex::m_pDeclaration, true); // This can fail
			
	if(pVertices)
	{
		for(S32 i = 0; i < node.blocks.Size(); ++i)
		{
			const C_MinimalObject* pObject = (C_MinimalObject*)node.blocks[i];

			Maths::HMat43 mTransform(Maths::Load<Maths::HMat44>(&pObject->m_Transform));
			Maths::C_RGBA_F32 vColour = pObject->m_Colour;

			Maths::HVec3 vTransformedVertex0(vVertex0 * mTransform);
			Maths::HVec3 vTransformedVertex1(vVertex1 * mTransform);
			Maths::HVec3 vTransformedVertex2(vVertex2 * mTransform);
			Maths::HVec3 vTransformedVertex3(vVertex3 * mTransform);
			Maths::HVec3 vTransformedVertex4(vVertex4 * mTransform);
			Maths::HVec3 vTransformedVertex5(vVertex5 * mTransform);
			Maths::HVec3 vTransformedVertex6(vVertex6 * mTransform);
			Maths::HVec3 vTransformedVertex7(vVertex7 * mTransform);

			// store the final positions into aligned stack memory
			MathsStorage::Float4A16 vFinalPosition[8];
			Maths::Store<Maths::HVec3>(vTransformedVertex0, &vFinalPosition[0]);
			Maths::Store<Maths::HVec3>(vTransformedVertex1, &vFinalPosition[1]);
			Maths::Store<Maths::HVec3>(vTransformedVertex2, &vFinalPosition[2]);
			Maths::Store<Maths::HVec3>(vTransformedVertex3, &vFinalPosition[3]);
			Maths::Store<Maths::HVec3>(vTransformedVertex4, &vFinalPosition[4]);
			Maths::Store<Maths::HVec3>(vTransformedVertex5, &vFinalPosition[5]);
			Maths::Store<Maths::HVec3>(vTransformedVertex6, &vFinalPosition[6]);
			Maths::Store<Maths::HVec3>(vTransformedVertex7, &vFinalPosition[7]);

			for(S32 i = 0; i < 21; i+=4)
			{
				// reload appropriate vectors
				Maths::HVec3 vFinalPosition0(Maths::Load<Maths::HVec3>(&vFinalPosition[indices[i]]));
				Maths::HVec3 vFinalPosition1(Maths::Load<Maths::HVec3>(&vFinalPosition[indices[i+1]]));
				Maths::HVec3 vFinalPosition2(Maths::Load<Maths::HVec3>(&vFinalPosition[indices[i+2]]));
				Maths::HVec3 vFinalPosition3(Maths::Load<Maths::HVec3>(&vFinalPosition[indices[i+3]]));

				Maths::HVec3 v0 = vFinalPosition3 - vFinalPosition0;
				Maths::HVec3 v1 = vFinalPosition1 - vFinalPosition0;
				Maths::HVec3 vNormal = v0 ^ v1;
				vNormal.Normalise();
				
				// 0
				Maths::Store<Maths::HVec3>(vFinalPosition0, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::C_RGBA_F32>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = 0.0f;
				pVertices->v = 0.0f;
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
				// 1
				Maths::Store<Maths::HVec3>(vFinalPosition1, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::C_RGBA_F32>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = 1.0f;
				pVertices->v = 0.0f;
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
				// 2
				Maths::Store<Maths::HVec3>(vFinalPosition2, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::C_RGBA_F32>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = 1.0f;
				pVertices->v = 1.0f;
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
	#ifndef PLATFORM_PC
				// 3
				Maths::Store<Maths::HVec3>(vFinalPosition3, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::C_RGBA_F32>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = 0.0f;
				pVertices->v = 1.0f;
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;	
	#else
				// 2
				Maths::Store<Maths::HVec3>(vFinalPosition2, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::C_RGBA_F32>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = 1.0f;
				pVertices->v = 1.0f;
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
				// 3
				Maths::Store<Maths::HVec3>(vFinalPosition3, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::C_RGBA_F32>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = 0.0f;
				pVertices->v = 1.0f;
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
				// 0
				Maths::Store<Maths::HVec3>(vFinalPosition0, (MathsStorage::Float3*)&pVertices->x);
				Maths::Store<Maths::C_RGBA_F32>(vColour, (MathsStorage::Float4*)&pVertices->r);
				pVertices->u = 0.0f;
				pVertices->v = 0.0f;
				Maths::Store<Maths::HVec3>(vNormal, (MathsStorage::Float3*)&pVertices->nx);
				++pVertices;
	#endif
			}
		}
		Display::Device()->EndVertices(pVertices);
	}

}

} // DynamicEffect













