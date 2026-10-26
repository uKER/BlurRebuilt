#include "stdafx.h"

#include <algorithm>

#include <Display/Buffer/VertexDeclarations.h>
#include <Display/Shaders/Constants.h>
#include <Display/Buffer/DynamicVertexBuffer.h>
#include <Display/View/ViewSettings.h>

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Core.h>

#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/Primitives.h>
#include <EditorApp/Editor/ComplexRenderer.h>
#include <EditorApp/Editor/ActorScene.h>


EXTERN_SHADER(vsBasic);
EXTERN_SHADER(psBasicColour);

using namespace Editor;

struct ComplexPrimitiveData
{
	struct VertexData
	{
		F32 x, y, z;
	};

	struct InstanceData
	{
		F32 ax[4];
		F32 ay[4];
		F32 az[4];
		F32 aw[4];

		F32 r, g, b, a;
	};
};

C_ComplexRenderer::C_ComplexRenderer()
{
	m_ShaderManager.LoadVertexShader("shaders/vsBasic.shader", REF_SHADER(vsBasic));
	m_ShaderManager.LoadPixelShader("shaders/psBasicColour.shader", REF_SHADER(psBasicColour));
}

C_ComplexRenderer::~C_ComplexRenderer()
{
	for (int i=0; i<m_OwnedComplexObjects.Size(); i++) {
		m_OwnedComplexObjects[i]->DestroyVertexBuffer();
		SAFE_DELETE_PTR(m_OwnedComplexObjects[i]);
	}
}

void C_ComplexRenderer::Clear()
{
	m_ComplexObjectInstances.Clear();
}

void C_ComplexRenderer::Update(AftkRender::E_RendererPass pass)
{

}

bool C_ComplexRenderer::OnCreate()
{
//	ComplexPrimitiveData::m_pDeclaration = Display::Device()->CreateVertexDeclaration(ComplexPrimitiveData::m_Elements);
	return AftkRender::C_Renderer::OnCreate();
}

void C_ComplexRenderer::OnDestroy()
{
	AftkRender::C_Renderer::OnDestroy();
}

bool C_ComplexRenderer::OnCreateVolatile()
{
	for (int i=0; i<m_OwnedComplexObjects.Size(); i++)
	{
		Display::Device()->CreateVertexBuffer(
			m_OwnedComplexObjects[i]->GetVertexBufferSize()*sizeof(ComplexPrimitiveData::VertexData),
			Display::E_Usage_Static|Display::E_Usage_Video,
			m_OwnedComplexObjects[i]->GetVertexBuffer());
	}

	if (!AftkRender::C_Renderer::OnCreateVolatile())
		return false;

	for (int i=0; i<m_OwnedComplexObjects.Size(); i++)
	{
		if (m_OwnedComplexObjects[i]->m_Callback != 0) {
			m_OwnedComplexObjects[i]->m_Callback->ComplexRendererCallback(m_OwnedComplexObjects[i]);
		}
	}

	return true;
}

void C_ComplexRenderer::OnDestroyVolatile()
{
	for (int i=0; i<m_OwnedComplexObjects.Size(); i++) {
		if (m_OwnedComplexObjects[i]->GetVertexBuffer()->IsCreated())
			Display::Device()->DestroyVertexBuffer(m_OwnedComplexObjects[i]->GetVertexBuffer());
	}

	AftkRender::C_Renderer::OnDestroyVolatile();
}

void C_ComplexRenderer::GetVisible(
			const RECT& region, 
			C_DgNodeSelection& selection)
{
	Display::Device()->SetVertexShader(m_ShaderManager.GetVertexShader("shaders/vsBasic.shader"));
	Display::Device()->SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicColour.shader"));
	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);

	for (int i=0; i<m_ComplexObjectInstances.Size(); i++) {
		BeginVisibilityTest(region);

		Display::C_Matrix colMatrix;
		Maths::Store(*m_ComplexObjectInstances[i].m_pXform, &colMatrix);
		Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&colMatrix, 4);

		Display::Device()->SetVertexBuffer(0, m_ComplexObjectInstances[i].m_pObject->GetVertexBuffer(), 0, sizeof(Display::Vertex_XYZ));
		Display::Device()->SetVertexDeclaration(Display::Vertex_XYZ::m_pDeclaration);

		Display::Device()->DrawPrimitive(
			Display::E_PrimitiveType_LineStrip, m_ComplexObjectInstances[i].m_VertexOffset, m_ComplexObjectInstances[i].m_VertexCount);

		if (EndVisibilityTest())
			selection.Add(EditorLib::C_Core::Instance().GetDgManager().FindNodeById(m_ComplexObjectInstances[i].m_DgNodeId));
	}

	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetFillMode(Display::E_FillMode_Solid);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
}

void C_ComplexRenderer::Render()
{
	Display::Device()->SetVertexShader(m_ShaderManager.GetVertexShader("shaders/vsBasic.shader"));
	Display::Device()->SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicColour.shader"));
	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);

	for (int i=0; i<m_ComplexObjectInstances.Size(); i++) {
		Display::C_Matrix colMatrix;
		Maths::Store(*m_ComplexObjectInstances[i].m_pXform, &colMatrix);
		Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&colMatrix, 4);		

		const F32 d = 1.f/255.f;
		F32 colour[4];

		colour[0] = d*m_ComplexObjectInstances[i].m_Colour.R();
		colour[1] = d*m_ComplexObjectInstances[i].m_Colour.G();
		colour[2] = d*m_ComplexObjectInstances[i].m_Colour.B();
		colour[3] = d*m_ComplexObjectInstances[i].m_Colour.A();

		Display::Device()->SetPixelShaderConstantF(CP_DISPLAY_COLOUR, colour, 1);

		Display::Device()->SetVertexBuffer(0, m_ComplexObjectInstances[i].m_pObject->GetVertexBuffer(), 0, sizeof(Display::Vertex_XYZ));
		Display::Device()->SetVertexDeclaration(Display::Vertex_XYZ::m_pDeclaration);

		Display::Device()->DrawPrimitive(m_ComplexObjectInstances[i].m_pObject->GetPrimitiveType(), m_ComplexObjectInstances[i].m_VertexOffset, m_ComplexObjectInstances[i].m_VertexCount);
	}

	Display::Device()->SetCullMode(Display::E_CullMode_Default);
	Display::Device()->SetFillMode(Display::E_FillMode_Solid);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);

	m_ComplexObjectInstances.Clear();
}

int C_ComplexRenderer::CreateComplexObject(int vertexCount, ComplexObject** complexObject, I_ComplexObjectCallback* callback)
{
	Display::C_VertexBuffer* buffer = new Display::C_VertexBuffer();
	Display::Device()->CreateVertexBuffer(
		vertexCount*sizeof(ComplexPrimitiveData::VertexData), Display::E_Usage_Static|Display::E_Usage_Video, buffer);

	*complexObject = new ComplexObject(buffer, vertexCount, Display::E_PrimitiveType_LineStrip, callback);

	m_OwnedComplexObjects.PushBack(*complexObject);

	return 0;
}

bool C_ComplexRenderer::ResizeComplexObject(int vertexCount, ComplexObject* complexObject)
{
	if (Display::Device()->GetLost())
		return false;
	
	Display::Device()->DestroyVertexBuffer(complexObject->GetVertexBuffer());

	Display::Device()->CreateVertexBuffer(
		vertexCount*sizeof(ComplexPrimitiveData::VertexData), Display::E_Usage_Static|Display::E_Usage_Video, complexObject->GetVertexBuffer());

	complexObject->SetVertexBufferSize(vertexCount);

	if (complexObject->m_Callback != 0) {
		complexObject->m_Callback->ComplexRendererCallback(complexObject);
	}

	return true;
}

void C_ComplexRenderer::DeleteComplexObject(ComplexObject** complexObject)
{
	S32 i;
	for (i=0; i<m_OwnedComplexObjects.Size(); i++) 
	{
		if( m_OwnedComplexObjects[i] == *complexObject)
		{
			Display::Device()->DestroyVertexBuffer(m_OwnedComplexObjects[i]->GetVertexBuffer());
			m_OwnedComplexObjects[i]->DestroyVertexBuffer();
			SAFE_DELETE_PTR(m_OwnedComplexObjects[i]);
			break;
		}
	}
	m_OwnedComplexObjects.EraseByIndex(i);
}

void C_ComplexRenderer::DrawInstance(
	ComplexObject* complexObject,
	int vertexOffset,
	int vertexCount, 
	const Maths::HMat44 *pMatrix, 
	const Maths::C_AxisBox &bounds, 
	const EditorLib::C_NodeId& id,
	Maths::RGBA* pColour)
{
	const Display::C_ViewFrustum& frustum = Display::Device()->GetViewFrustum();
	S32 cullResult = frustum.CullAxisBox(bounds);

	if (cullResult > 0)
	{
		ComplexObjectInstance instance;
		instance.m_pXform = pMatrix;
		instance.m_Colour = *pColour;
		instance.m_pObject = complexObject;
		instance.m_DgNodeId = id;
		instance.m_VertexOffset = vertexOffset;
		instance.m_VertexCount = vertexCount;
		m_ComplexObjectInstances.PushBack(instance);
	}
}
