#include "stdafx.h"

#include <algorithm>

#include <Display/Buffer/VertexDeclarations.h>
#include <Display/Shaders/Constants.h>

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Core.h>

#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/Primitives.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/SimpleRenderer.h>
#include <EditorApp/Editor/ActorScene.h>

EXTERN_SHADER(vsBasic);
EXTERN_SHADER(vsBasicNormal);
EXTERN_SHADER(psBasic);
EXTERN_SHADER(psBasicColour);
EXTERN_SHADER(psBasicNdotL);
EXTERN_SHADER(psBasicNdotLColour);
EXTERN_SHADER(vsInstanceWireframe);
EXTERN_SHADER(psInstanceWireframe);
EXTERN_SHADER(vsInstanceSolid);
EXTERN_SHADER(psInstanceSolid);
EXTERN_SHADER(psInstanceSolidLit);

typedef Btl::StringBase<char, Btl::FixedStringBuffer<char, 256> > String1024;

using namespace Editor;

namespace PrimitiveData
{
	struct VertexDataWireframe
	{
		F32 x, y, z;
	};

	struct VertexDataSolid
	{
		F32 x, y, z;
		F32 nx, ny, nz;
		F32 u, v;
	};

	struct InstanceData
	{
		F32 ax[4];
		F32 ay[4];
		F32 az[4];
		F32 aw[4];

		F32 r, g, b, a;
	};

	struct Wireframe
	{
		static const U32						m_ElementCount = 2;
		static Display::C_VertexElement			m_Elements[m_ElementCount];
		static Display::C_VertexDeclaration*	m_pDeclaration;	
	};

	struct Solid
	{
		static const U32						m_ElementCount = 4;
		static Display::C_VertexElement			m_Elements[m_ElementCount];
		static Display::C_VertexDeclaration*	m_pDeclaration;
	};

	struct InstancedWireframe
	{
		static const U32						m_ElementCount = 7;
		static Display::C_VertexElement			m_Elements[m_ElementCount];
		static Display::C_VertexDeclaration*	m_pDeclaration;	
	};

	struct InstancedSolid
	{
		static const U32						m_ElementCount = 9;
		static Display::C_VertexElement			m_Elements[m_ElementCount];
		static Display::C_VertexDeclaration*	m_pDeclaration;
	};

	Display::C_VertexElement PrimitiveData::Wireframe::m_Elements[PrimitiveData::Wireframe::m_ElementCount] =
	{
		Display::C_VertexElement(0, 0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0 ),
		VERTEXELEMENT_END()
	};
	Display::C_VertexDeclaration* PrimitiveData::Wireframe::m_pDeclaration = NULL;

	Display::C_VertexElement PrimitiveData::Solid::m_Elements[PrimitiveData::Solid::m_ElementCount] =
	{
		Display::C_VertexElement(0, 0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0 ),
		Display::C_VertexElement(0, 12, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Normal, 0 ),
		Display::C_VertexElement(0, 24, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0 ),
		VERTEXELEMENT_END()
	};
	Display::C_VertexDeclaration* PrimitiveData::Solid::m_pDeclaration = NULL;

	Display::C_VertexElement PrimitiveData::InstancedWireframe::m_Elements[PrimitiveData::InstancedWireframe::m_ElementCount] =
	{
		Display::C_VertexElement(0, 0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0 ),
		Display::C_VertexElement(1, 0, Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_TexCoord, 1 ),
		Display::C_VertexElement(1, 16, Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_TexCoord, 2 ),
		Display::C_VertexElement(1, 32, Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_TexCoord, 3 ),
		Display::C_VertexElement(1, 48, Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_TexCoord, 4 ),
		Display::C_VertexElement(1, 64, Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_Colour, 0 ),
		VERTEXELEMENT_END()
	};
	Display::C_VertexDeclaration* PrimitiveData::InstancedWireframe::m_pDeclaration = NULL;


	Display::C_VertexElement PrimitiveData::InstancedSolid::m_Elements[PrimitiveData::InstancedSolid::m_ElementCount] =
	{
		Display::C_VertexElement(0, 0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0 ),
		Display::C_VertexElement(0, 12, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Normal, 0 ),
		Display::C_VertexElement(0, 24, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0 ),
		Display::C_VertexElement(1, 0, Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_TexCoord, 1 ),
		Display::C_VertexElement(1, 16, Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_TexCoord, 2 ),
		Display::C_VertexElement(1, 32, Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_TexCoord, 3 ),
		Display::C_VertexElement(1, 48, Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_TexCoord, 4 ),
		Display::C_VertexElement(1, 64, Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_Colour, 0 ),
		VERTEXELEMENT_END()
	};
	Display::C_VertexDeclaration* PrimitiveData::InstancedSolid::m_pDeclaration = NULL;
}



inline bool HasNormalData(const PrimitiveResourcePtr& ptr)
{
	return (ptr->GetVertexDataElement(AftkRender::E_ElementType_Normal) != 0);
}



C_SimpleRenderer::C_SimpleRenderer()
: m_pVertexBuffer(0)
, m_pInstanceBuffer(0)
, m_pIndexBuffer(0)
, m_NonIndexedVertexCount(0)
, m_CurrentNumInstances(1000)
{
	m_ShaderManager.LoadVertexShader("shaders/vsBasic.shader", REF_SHADER(vsBasic));
	m_ShaderManager.LoadVertexShader("shaders/vsBasicNormal.shader", REF_SHADER(vsBasicNormal));
	m_ShaderManager.LoadPixelShader("shaders/psBasic.shader", REF_SHADER(psBasic));
	m_ShaderManager.LoadPixelShader("shaders/psBasicColour.shader", REF_SHADER(psBasicColour));
	m_ShaderManager.LoadPixelShader("shaders/psBasicNdotL.shader", REF_SHADER(psBasicNdotL));
	m_ShaderManager.LoadPixelShader("shaders/psBasicNdotLColour.shader", REF_SHADER(psBasicNdotLColour));
	

	m_ShaderManager.LoadVertexShader("shaders/vsInstanceWireframe.shader", REF_SHADER(vsInstanceWireframe));
	m_ShaderManager.LoadPixelShader("shaders/psInstanceWireframe.shader", REF_SHADER(psInstanceWireframe));

	m_ShaderManager.LoadVertexShader("shaders/vsInstanceSolid.shader", REF_SHADER(vsInstanceSolid));
	m_ShaderManager.LoadPixelShader("shaders/psInstanceSolid.shader", REF_SHADER(psInstanceSolid));
	m_ShaderManager.LoadPixelShader("shaders/psInstanceSolidLit.shader", REF_SHADER(psInstanceSolidLit));

	m_pVertexBuffer		= new Display::C_VertexBuffer();
	m_pInstanceBuffer	= new Display::C_VertexBuffer();
	m_pIndexBuffer		= new Display::C_IndexBuffer();

	memset(&m_Flags, 0, sizeof(C_SimpleRenderer::Flags));
}

C_SimpleRenderer::~C_SimpleRenderer()
{
	SAFE_DELETE_PTR(m_pVertexBuffer);
	SAFE_DELETE_PTR(m_pInstanceBuffer);
	SAFE_DELETE_PTR(m_pIndexBuffer);
}

void C_SimpleRenderer::Clear()
{
	m_PrimitiveInstances.Clear();
}

void C_SimpleRenderer::Update(AftkRender::E_RendererPass pass)
{
/*	m_NodeContextArray.Clear();		//not used by this renderer but we'll clear it anyway
	m_RenderContextArray.Clear();	//not used by this renderer but we'll clear it anyway
	m_PrimitiveInstances.Clear();

	Ut::UtVector< Ut::DynamicVector<AftkRender::C_SceneGraphNode*> >::Iterator it = m_RootNodePtrs.Begin();
	while (it != m_RootNodePtrs.End())
	{
		(*it)->AttachToRenderer(this, 0, pass);
		it++;
	}*/

	std::sort(m_PrimitiveInstances.Begin(), m_PrimitiveInstances.End(), Compare);

	if (m_Flags.vertexbufferdirty || m_Flags.indexbufferdirty)
	{
		DestroyVolatile();
		CreateVolatile();

		PopulateVertexBuffer();
		PopulateIndexBuffer();
	}

	if(Display::C_DeviceCaps::m_d3dCaps.VertexShaderVersion>=D3DVS_VERSION(3,0) && m_Flags.instancebufferdirty)
	{
		if (m_CurrentNumInstances*sizeof(PrimitiveData::InstanceData) > m_pInstanceBuffer->GetSize())
		{
			//destroy & recreate buffer for instance vertex data
			if (m_pInstanceBuffer->IsCreated())
				Display::Device()->DestroyVertexBuffer(m_pInstanceBuffer);

			Display::Device()->CreateVertexBuffer(
				m_CurrentNumInstances*sizeof(PrimitiveData::InstanceData), Display::E_Usage_Static|Display::E_Usage_Video, m_pInstanceBuffer);
		}

		PopulateInstanceBuffer();
	}
}

void C_SimpleRenderer::GetVisible(
			const RECT& region, 
			C_DgNodeSelection& selection)
{
	Display::Device()->SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasic.shader"));
	Display::Device()->SetIndexBuffer(m_pIndexBuffer);

	for (S32 i=0; i<m_PrimitiveInstances.Size(); ++i)
	{
		bool visible = false;

		if (m_PrimitiveInstances[i].basePrimitive<0 || m_PrimitiveInstances[i].basePrimitive>m_PrimitiveData.Size())
			continue;

		BeginVisibilityTest(region);

		const Primitive *pPrimitive = &m_PrimitiveData[m_PrimitiveInstances[i].basePrimitive];

		{
			const U32 stride = HasNormalData(pPrimitive->meshData)
				? sizeof(PrimitiveData::VertexDataSolid)
				: sizeof(PrimitiveData::VertexDataWireframe);
			const Display::C_VertexDeclaration *pDecl = HasNormalData(pPrimitive->meshData)
				? PrimitiveData::Solid::m_pDeclaration
				: PrimitiveData::Wireframe::m_pDeclaration;
			Display::Device()->SetVertexShader(
				HasNormalData(pPrimitive->meshData)
					? m_ShaderManager.GetVertexShader("shaders/vsBasicNormal.shader")
					: m_ShaderManager.GetVertexShader("shaders/vsBasic.shader"));

			Display::Device()->SetVertexDeclaration(pDecl);
			Display::Device()->SetVertexBuffer(0, m_pVertexBuffer, pPrimitive->vertexOffset, stride);

			if (!HasNormalData(pPrimitive->meshData))
			{
				Display::Device()->SetCullMode(Display::E_CullMode_None);
				Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);
			}
			else
			{
				Display::Device()->SetCullMode((Display::E_CullMode)m_RenderFlags.cullmode);
				Display::Device()->SetFillMode((Display::E_FillMode)m_RenderFlags.fillmode);
			}
		}

		Display::C_Matrix colMatrix;
		Maths::Store(*m_PrimitiveInstances[i].pXform, &colMatrix);
		Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&colMatrix, 4);

		if (pPrimitive->meshData->GetNumIndexDataElements())
		{
			U32 startIndex = pPrimitive->indexOffset/sizeof(U16);
			for (U32 j=0; j<pPrimitive->meshData->GetNumIndexDataElements(); ++j)
			{
				const AftkRender::C_MeshClusterResource::IndexDataElement *pData = pPrimitive->meshData->GetIndexDataElement(j);
				Display::Device()->DrawIndexedPrimitive(AftkRender::PrimitiveTypeLookup[pData->type], 0, 0, pPrimitive->meshData->GetNumVertices(), startIndex, pData->count);
				startIndex += pData->count;
			}
		}
		else
			Display::Device()->DrawPrimitive(AftkRender::PrimitiveTypeLookup[pPrimitive->meshData->GetNonIndexedPrimitiveType()], 0, pPrimitive->meshData->GetNumVertices());

		if (EndVisibilityTest())
		{
			if (m_PrimitiveInstances[i].componentId != 0xffffffff)
			{
				Ut::Vector<U32> components;
				components.PushBack(m_PrimitiveInstances[i].componentId);
				selection.Add(EditorLib::C_Core::Instance().GetDgManager().FindNodeById(m_PrimitiveInstances[i].nodeId), components);
			}
			else 
			{
				selection.Add(EditorLib::C_Core::Instance().GetDgManager().FindNodeById(m_PrimitiveInstances[i].nodeId));
			}
		}
	}
}


// D3D Vertex/Index buffers are arranged as shown
//
// Vertex Buffer	| Prim1 | Prim2 | ....
//
// Index Buffer		| Reserved | Prim2 batch1 | Prim2 batch2 | ...
//
// The first m_NonIndexedVertexCount indices are reserved for rendering non-indexed primitives

void C_SimpleRenderer::SetObjTypePrimitive(
	const char* pTypeName_, 
	PrimitiveResourcePtr pPrimitive, 
	Maths::RGBA colour,
	const Ut::Vector<Btl::String> *pTexturePaths)
{
	String1024 typeName(pTypeName_);
	typeName.ConvertToLower();

	for (S32 i=0; i<m_PrimitiveData.Size(); ++i)
		if (m_PrimitiveData[i].typeName == typeName.Chars())
			return;

	if (pPrimitive->GetNumVertices())
	{
		U32 currentVertexOffset = 0;
		if (m_PrimitiveData.Size())
		{
			U32 lastPrimitiveVertexDataSize = HasNormalData(m_PrimitiveData.Back().meshData)
				? m_PrimitiveData.Back().meshData->GetNumVertices()*sizeof(PrimitiveData::VertexDataSolid)
				: m_PrimitiveData.Back().meshData->GetNumVertices()*sizeof(PrimitiveData::VertexDataWireframe);

			currentVertexOffset = m_PrimitiveData.Back().vertexOffset + lastPrimitiveVertexDataSize;
		}

		U32 currentIndexOffset = 0;
		{
			if (pPrimitive->GetNumIndexDataElements())
			{
				Primitive *pLastIndexedPrimitive = 0;
				for (S32 i=m_PrimitiveData.Size()-1; i>=0 && !pLastIndexedPrimitive; --i)
					if (m_PrimitiveData[i].indexOffset != 0)
						pLastIndexedPrimitive = &m_PrimitiveData[i];

				if (pLastIndexedPrimitive)
				{
					currentIndexOffset = pLastIndexedPrimitive->indexOffset;
					for (U32 i=0; i<pLastIndexedPrimitive->meshData->GetNumIndexDataElements(); ++i)
						currentIndexOffset += pLastIndexedPrimitive->meshData->GetIndexDataElement(i)->count*sizeof(U16);
				}
				else
					currentIndexOffset = m_NonIndexedVertexCount*sizeof(U16);
			}
			else
			{
				if (pPrimitive->GetNumVertices() > m_NonIndexedVertexCount)
				{
					U32 offset = sizeof(U16)*(pPrimitive->GetNumVertices()-m_NonIndexedVertexCount);
					m_NonIndexedVertexCount = pPrimitive->GetNumVertices();

					//shuffle vertex data
					for (S32 i=0; i<m_PrimitiveData.Size(); i++)
						if (m_PrimitiveData[i].indexOffset != 0)
							m_PrimitiveData[i].indexOffset += offset;
				}
			}

		}

		const F32 *pVertexData = (const F32*) pPrimitive->GetVertexDataElement(AftkRender::E_ElementType_Position);
		assert(pVertexData);

		Maths::C_AxisBox bounds;
		for (U32 i=0; i<pPrimitive->GetNumVertices(); ++i)
		{
			bounds.UpdateExtents(Maths::C_Vertex(pVertexData));
			pVertexData += 3;
		}

		Ut::Vector<TextureResourcePtr> textures;
		if (pTexturePaths)
		{
			for (S32 i=0; i<pTexturePaths->Size(); ++i)
			{
				if ((*pTexturePaths)[i].Length())
				{
					TextureResourcePtr ptr = new Display::C_RefTexture2D();
					ptr->SetFileName((*pTexturePaths)[i].Chars());
					textures.PushBack(ptr);
				}
				else
					textures.PushBack(TextureResourcePtr(0));
			}
		}

		Primitive p = { typeName.Chars(), pPrimitive, colour, currentVertexOffset, currentIndexOffset, bounds, textures };
		m_PrimitiveData.PushBack(p);

		m_Flags.vertexbufferdirty = true;
		m_Flags.indexbufferdirty = true;
	}
}

void C_SimpleRenderer::RemoveObjTypePrimitive(const char *pTypeName)
{
	S32 index = -1;
	for (S32 i=0; i<m_PrimitiveData.Size() && index == -1; ++i)
		if (m_PrimitiveData[i].typeName == pTypeName)
			index = i;

	if (index != -1)
	{
		U32 nVertices = m_PrimitiveData[index].meshData->GetNumVertices();
		U32 nIndices = 0;
		for (U32 i=0; i<m_PrimitiveData[index].meshData->GetNumIndexDataElements(); ++i)
			nIndices += m_PrimitiveData[index].meshData->GetIndexDataElement(i)->count;

		U32 offset = HasNormalData(m_PrimitiveData[index].meshData)
			? nVertices*sizeof(PrimitiveData::VertexDataSolid)
			: nVertices*sizeof(PrimitiveData::VertexDataWireframe);

		//adjust offset information for trailing primitives
		for (S32 i=index+1; i<m_PrimitiveData.Size(); ++i)
		{
			m_PrimitiveData[i].vertexOffset -= offset;
			if (m_PrimitiveData[i].indexOffset)
				m_PrimitiveData[i].indexOffset -= nIndices*sizeof(U16);
		}

		m_PrimitiveData.EraseByIndex(index);

		m_Flags.vertexbufferdirty = true;
		m_Flags.indexbufferdirty = true;
	}
}

const Maths::C_AxisBox* C_SimpleRenderer::GetPrimitiveBounds(const char* pTypeName_) const
{
	const Primitive* pPrim = FindPrimitive(pTypeName_);
	if (pPrim)
		return &pPrim->bounds;

	return 0;
}

bool C_SimpleRenderer::PrimitiveExists(const char* pTypeName_) const
{
	if (FindPrimitive(pTypeName_))
		return true;
	return false;
}

bool C_SimpleRenderer::OnCreate()
{
	PrimitiveData::Wireframe::m_pDeclaration = Display::Device()->CreateVertexDeclaration(PrimitiveData::Wireframe::m_Elements);
	PrimitiveData::Solid::m_pDeclaration = Display::Device()->CreateVertexDeclaration(PrimitiveData::Solid::m_Elements);
	PrimitiveData::InstancedWireframe::m_pDeclaration = Display::Device()->CreateVertexDeclaration(PrimitiveData::InstancedWireframe::m_Elements);
	PrimitiveData::InstancedSolid::m_pDeclaration = Display::Device()->CreateVertexDeclaration(PrimitiveData::InstancedSolid::m_Elements);
	return AftkRender::C_Renderer::OnCreate();
}

void C_SimpleRenderer::OnDestroy()
{
	AftkRender::C_Renderer::OnDestroy();
}

bool C_SimpleRenderer::OnCreateVolatile()
{
	//Create Vertex Buffer
	{
		U32 vertexSize = 0;
		if (m_PrimitiveData.Size())
		{
			U32 lastPrimitiveVertexDataSize = HasNormalData(m_PrimitiveData.Back().meshData)
				? m_PrimitiveData.Back().meshData->GetNumVertices()*sizeof(PrimitiveData::VertexDataSolid)
				: m_PrimitiveData.Back().meshData->GetNumVertices()*sizeof(PrimitiveData::VertexDataWireframe);

			vertexSize = m_PrimitiveData.Back().vertexOffset+lastPrimitiveVertexDataSize;
		}

		if (vertexSize)
			Display::Device()->CreateVertexBuffer(vertexSize, Display::E_Usage_Static|Display::E_Usage_Video, m_pVertexBuffer);
	}

	//Create Instance Buffer
	if(m_CurrentNumInstances && Display::C_DeviceCaps::m_d3dCaps.VertexShaderVersion>=D3DVS_VERSION(3,0))
	{
		Display::Device()->CreateVertexBuffer(
			m_CurrentNumInstances*sizeof(PrimitiveData::InstanceData), Display::E_Usage_Static|Display::E_Usage_Video, m_pInstanceBuffer);
	}

	//Create Index Buffer
	{
		U32 indexSize = m_NonIndexedVertexCount*sizeof(U16);
		//find the last indexed primitive to determine number of indices required
		for (S32 i=m_PrimitiveData.Size()-1; i>=0; --i)
		{
			U32 nIndices = 0;
			for (U32 j=0; j<m_PrimitiveData[i].meshData->GetNumIndexDataElements(); ++j)
				nIndices += m_PrimitiveData[i].meshData->GetIndexDataElement(j)->count;

			if (nIndices)
			{
				assert(m_PrimitiveData[i].indexOffset >= m_NonIndexedVertexCount*sizeof(U16));
				indexSize = m_PrimitiveData[i].indexOffset + nIndices*sizeof(U16);
				break;
			}
		}

		if (indexSize)
			Display::Device()->CreateIndexBuffer(
				indexSize, Display::E_Usage_Static|Display::E_Usage_Video, Display::E_Format_Index16, m_pIndexBuffer);
	}

	m_Flags.vertexbufferdirty = true;
	m_Flags.instancebufferdirty = true;
	m_Flags.indexbufferdirty = true;

	if (!AftkRender::C_Renderer::OnCreateVolatile())
		return false;

	return true;
}

void C_SimpleRenderer::OnDestroyVolatile()
{
	if (m_pVertexBuffer->IsCreated())
		Display::Device()->DestroyVertexBuffer(m_pVertexBuffer);

	if (m_pInstanceBuffer->IsCreated())
		Display::Device()->DestroyVertexBuffer(m_pInstanceBuffer);

	if (m_pIndexBuffer->IsCreated())
		Display::Device()->DestroyIndexBuffer(m_pIndexBuffer);

	return AftkRender::C_Renderer::OnDestroyVolatile();
}


bool C_SimpleRenderer::AddPrimitive(
	const char* pTypeName_, 
	const Maths::HMat44* pMatrix, 
	const Maths::C_AxisBox& bounds, 
	const EditorLib::C_NodeId& id, 
	U32 component, 
	const Maths::RGBA* pColourOverride)
{
	String1024 typeName(pTypeName_);
	typeName.ConvertToLower();

	S32 basePrimitiveIndex = -1;
	for (S32 i=0; i<m_PrimitiveData.Size() && basePrimitiveIndex==-1; ++i)
		if (m_PrimitiveData[i].typeName == typeName.Chars())
			basePrimitiveIndex = i;

	if (basePrimitiveIndex == -1)
		return false;	//don't know how to render it :(

	const Display::C_ViewFrustum& frustum = Display::Device()->GetViewFrustum();
	S32 cullResult = frustum.CullAxisBox(bounds);

	if (cullResult > 0)
	{
		PrimitiveInstance pi = {basePrimitiveIndex, pMatrix, pColourOverride ? *pColourOverride : m_PrimitiveData[basePrimitiveIndex].defaultColour,  id, component};
		m_PrimitiveInstances.PushBack(pi);
		m_Flags.instancebufferdirty = true;
		if ((U32)m_PrimitiveInstances.Size() > m_CurrentNumInstances)
			m_CurrentNumInstances = m_PrimitiveInstances.Size();
	}
	return true;
}

const C_SimpleRenderer::RenderFlags& C_SimpleRenderer::GetRenderFlags() const 
{
	return m_RenderFlags;
}

void C_SimpleRenderer::SetRenderFlags(const RenderFlags& flags)
{
	m_RenderFlags = flags;
}

void C_SimpleRenderer::LoadTextures()
{
	/*
	for (S32 i=0; i<m_PrimitiveData.Size(); ++i)
	{
		for (S32 j=0; j<m_PrimitiveData[i].textures.Size(); ++j)
		{
			if (m_PrimitiveData[i].textures[j] != 0 && !m_PrimitiveData[i].textures[j]->IsCreated())
			{
				TextureResourcePtr pNewTexture = m_TextureManager.LoadTexture2DFromFile(m_PrimitiveData[i].textures[j]->GetFileName().Chars(), 256);
				if (pNewTexture != 0)
					m_PrimitiveData[i].textures[j] = pNewTexture;
			}
		}	
	}
	*/
}

void C_SimpleRenderer::PopulateVertexBuffer()
{
	if (!m_pVertexBuffer->IsCreated())
		return;

	//Repopulate our vertex buffer
	U32 vertexSize = 0;
	if (m_PrimitiveData.Size())
	{
		U32 lastPrimitiveVertexDataSize = HasNormalData(m_PrimitiveData.Back().meshData)
			? m_PrimitiveData.Back().meshData->GetNumVertices()*sizeof(PrimitiveData::VertexDataSolid)
			: m_PrimitiveData.Back().meshData->GetNumVertices()*sizeof(PrimitiveData::VertexDataWireframe);

		vertexSize = m_PrimitiveData.Back().vertexOffset+lastPrimitiveVertexDataSize;		
	}

	if (vertexSize)
	{
		U8 *pData = (U8*)m_pVertexBuffer->Lock(0 ,vertexSize, 0);
		if (!pData)
		{
			LOGERROR(g_Log, "Vertex buffer lock failed");
			return;
		}

		U8 *pCurrentData = pData;
		for (S32 i=0; i<m_PrimitiveData.Size(); ++i)
		{
			const Primitive* pPrimitive = &m_PrimitiveData[i];
			assert(pPrimitive->vertexOffset == pCurrentData-pData);

			if (HasNormalData(pPrimitive->meshData))
			{
				//interleave position, normal, uv data...
				const F32 *pPosition = (const F32*)pPrimitive->meshData->GetVertexDataElement(AftkRender::E_ElementType_Position);
				const F32 *pNormal = (const F32*)pPrimitive->meshData->GetVertexDataElement(AftkRender::E_ElementType_Normal);
				const F32 *pUV = (const F32*)pPrimitive->meshData->GetVertexDataElement(AftkRender::E_ElementType_TexCoord0);

				assert(pPosition);
				assert(pNormal);

				PrimitiveData::VertexDataSolid *pVertexData = (PrimitiveData::VertexDataSolid*) pCurrentData;
				for (U32 j=0; j<pPrimitive->meshData->GetNumVertices(); ++j)
				{
					memcpy(&pVertexData->x, pPosition, sizeof(F32)*3);
					memcpy(&pVertexData->nx, pNormal, sizeof(F32)*3);
					if (pUV)
						memcpy(&pVertexData->u, pUV, sizeof(F32)*2);
					else
						memset(&pVertexData->u, 0, sizeof(F32)*2);

					pVertexData++;	
					pPosition += 3;
					pNormal += 3;
					if (pUV) pUV += 2;
				}
				pCurrentData = (U8*) pVertexData;
			}
			else
			{
				memcpy(pCurrentData, pPrimitive->meshData->GetVertexDataElement(AftkRender::E_ElementType_Position), sizeof(Display::Vertex_XYZ)*pPrimitive->meshData->GetNumVertices());
				pCurrentData += pPrimitive->meshData->GetNumVertices()*sizeof(PrimitiveData::VertexDataWireframe);
			}
		}
		m_pVertexBuffer->Unlock();
	}
	m_Flags.vertexbufferdirty = false;
}

void C_SimpleRenderer::PopulateIndexBuffer()
{
	if (!m_pIndexBuffer->IsCreated())
		return;

	//repopulate our index buffer (for supporting HW Instancing)
	U16 *pIndexData = (U16*)m_pIndexBuffer->Lock();
	if (!pIndexData)
	{
		LOGERROR(g_Log, "Index buffer lock failed");
		return;
	}

	for (U16 i=0; i<m_NonIndexedVertexCount; ++i)
		(*pIndexData++) = i;

	U32 indexOffset = sizeof(U16)*m_NonIndexedVertexCount;

	for (S32 i=0; i<m_PrimitiveData.Size(); ++i)
	{
		if (m_PrimitiveData[i].indexOffset)
		{
			assert(indexOffset==m_PrimitiveData[i].indexOffset);

			for (U32 j=0; j<m_PrimitiveData[i].meshData->GetNumIndexDataElements(); ++j)
			{
				const AftkRender::C_MeshClusterResource::IndexDataElement *pIndexElementData = m_PrimitiveData[i].meshData->GetIndexDataElement(j);
				U32 idxCount = pIndexElementData ->count;
				const U32 *pIdxData = pIndexElementData ->pData;

				for (U32 z=0; z<idxCount; ++z)
					(*pIndexData++) = (U16)(*pIdxData++);

				indexOffset += sizeof(U16)*idxCount;
			}
		}
	}

	m_pIndexBuffer->Unlock();
	m_Flags.indexbufferdirty = false;
}

const C_SimpleRenderer::Primitive* C_SimpleRenderer::FindPrimitive(const char* pTypeName_) const
{
	String1024 typeName(pTypeName_);
	typeName.ConvertToLower();

	for (S32 i=0; i<m_PrimitiveData.Size(); ++i)
		if (m_PrimitiveData[i].typeName == typeName.Chars())
			return &m_PrimitiveData[i];

	return 0;
}

void C_SimpleRenderer::PopulateInstanceBuffer()
{
	//should only be called after primitive data has been sorted...
	if (!m_pInstanceBuffer->IsCreated() || !m_PrimitiveInstances.Size())
		return;

	assert(m_pInstanceBuffer->IsCreated() && m_CurrentNumInstances*sizeof(PrimitiveData::InstanceData) <= m_pInstanceBuffer->GetSize());

	PrimitiveData::InstanceData *pVertex = (PrimitiveData::InstanceData*)m_pInstanceBuffer->Lock(
		0, sizeof(PrimitiveData::InstanceData)*m_PrimitiveInstances.Size(), 0);

	if (!pVertex)
	{
		LOGERROR(g_Log, "Vertex buffer lock failed");
		return;
	}

	for (S32 i=0; i<m_PrimitiveInstances.Size(); ++i)
	{
		const F32 d = 1.f/255.f;
		MathsStorage::MatrixRow44A16 rowMatrix;
		Maths::Store(*m_PrimitiveInstances[i].pXform, &rowMatrix);

		memcpy(pVertex, &rowMatrix, sizeof(F32)*16);
		pVertex->r = d*m_PrimitiveInstances[i].colour.R();
		pVertex->g = d*m_PrimitiveInstances[i].colour.G();
		pVertex->b = d*m_PrimitiveInstances[i].colour.B();
		pVertex->a = d*m_PrimitiveInstances[i].colour.A();

		pVertex++;
	}

	m_pInstanceBuffer->Unlock();
	m_Flags.instancebufferdirty = false;
}

void C_SimpleRenderer::Render()
{
	if (!m_PrimitiveInstances.Size())
		return;

	if (Display::C_DeviceCaps::m_d3dCaps.VertexShaderVersion>=D3DVS_VERSION(3,0))
	{
		//HW instancing is supported... our vertex/index buffers *should* have been initialised by this stage
		Display::Device()->SetIndexBuffer(m_pIndexBuffer);

		U32 primitiveCount = 1;
		S32 currentPrimitive = m_PrimitiveInstances[0].basePrimitive;
		
		for (S32 i=0; i<m_PrimitiveInstances.Size(); ++i)
		{
			if (i==m_PrimitiveInstances.Size()-1 || currentPrimitive!=m_PrimitiveInstances[i+1].basePrimitive)
			{
				//primitive type has changed...render batch

				const Primitive* pPrimitive = &m_PrimitiveData[currentPrimitive];
				const U32 stride = HasNormalData(pPrimitive->meshData)
					? sizeof(PrimitiveData::VertexDataSolid)
					: sizeof(PrimitiveData::VertexDataWireframe);
				const Display::C_VertexDeclaration *pDecl = HasNormalData(pPrimitive->meshData)
					? PrimitiveData::InstancedSolid::m_pDeclaration
					: PrimitiveData::InstancedWireframe::m_pDeclaration;

				Display::Device()->SetVertexDeclaration(pDecl);
				Display::Device()->SetVertexBuffer(0, m_pVertexBuffer, pPrimitive->vertexOffset, stride);
				Display::Device()->SetVertexBuffer(1, m_pInstanceBuffer, (i-primitiveCount+1)*sizeof(PrimitiveData::InstanceData), sizeof(PrimitiveData::InstanceData));

				Display::Device()->GetD3DDevice()->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA|primitiveCount);
				Display::Device()->GetD3DDevice()->SetStreamSourceFreq(1, D3DSTREAMSOURCE_INSTANCEDATA | 1);

				if (!HasNormalData(pPrimitive->meshData))
				{
					Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);
					Display::Device()->SetCullMode(Display::E_CullMode_None);
					Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);
					Display::Device()->SetVertexShader(m_ShaderManager.GetVertexShader("shaders/vsInstanceWireframe.shader"));
					Display::Device()->SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psInstanceWireframe.shader"));
				}
				else
				{
					Display::Device()->SetBlendMode((m_RenderFlags.fillmode == Display::E_FillMode_Wireframe || m_PrimitiveInstances[i].colour.A() != 0xff) ? Display::E_BlendMode_Alpha : Display::E_BlendMode_Solid);
					Display::Device()->SetCullMode((Display::E_CullMode)m_RenderFlags.cullmode);
					Display::Device()->SetFillMode((Display::E_FillMode)m_RenderFlags.fillmode);

					Display::Device()->SetVertexShader(m_ShaderManager.GetVertexShader("shaders/vsInstanceSolid.shader"));

					if (m_RenderFlags.lit)					
						Display::Device()->SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psInstanceSolidLit.shader"));
					else
						Display::Device()->SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psInstanceSolid.shader"));
				}

				if (pPrimitive->meshData->GetNumIndexDataElements())
				{
					U32 startIndex = pPrimitive->indexOffset/sizeof(U16);
					for (U32 j=0; j<pPrimitive->meshData->GetNumIndexDataElements(); ++j)
					{
						const AftkRender::C_MeshClusterResource::IndexDataElement *pIndexElementData = pPrimitive->meshData->GetIndexDataElement(j);
						if (m_RenderFlags.textured)
						{
							S32 materialIndex = pIndexElementData->materialId;
							if (materialIndex != -1 && materialIndex < pPrimitive->textures.Size() && pPrimitive->textures[materialIndex] && pPrimitive->textures[materialIndex]->IsCreated())
								Display::Device()->SetTexture(0, pPrimitive->textures[materialIndex]);
							else
								Display::Device()->SetTexture(0, m_TextureManager.GetDefault2DTexture());
						}
						else
							Display::Device()->SetTexture(0, m_TextureManager.GetDefault2DTexture());

						Display::Device()->DrawIndexedPrimitive(AftkRender::PrimitiveTypeLookup[pIndexElementData->type], 0, 0, pPrimitive->meshData->GetNumVertices(), startIndex, pIndexElementData->count);
						startIndex += pIndexElementData->count;
					}
				}
				else
					Display::Device()->DrawIndexedPrimitive(AftkRender::PrimitiveTypeLookup[pPrimitive->meshData->GetNonIndexedPrimitiveType()], 0, 0, pPrimitive->meshData->GetNumVertices(), pPrimitive->indexOffset/sizeof(U16), pPrimitive->meshData->GetNumVertices());

				primitiveCount = 1;
				currentPrimitive = i<m_PrimitiveInstances.Size()-1 ? m_PrimitiveInstances[i+1].basePrimitive : -1;
			}
			else
				primitiveCount++;
		}

		Display::Device()->GetD3DDevice()->SetStreamSourceFreq(0, 1);
		Display::Device()->GetD3DDevice()->SetStreamSourceFreq(1, 1);
	}
	else
	{
		//fallback to spamming draw calls per instance :(
		Display::Device()->SetIndexBuffer(m_pIndexBuffer);

		for (S32 i=0; i<m_PrimitiveInstances.Size(); ++i)
		{
			if (m_PrimitiveInstances[i].basePrimitive<0 || m_PrimitiveInstances[i].basePrimitive>m_PrimitiveData.Size())
				continue;

			const Primitive *pPrimitive = &m_PrimitiveData[m_PrimitiveInstances[i].basePrimitive];
			
			if (!i || m_PrimitiveInstances[i].basePrimitive != m_PrimitiveInstances[i-1].basePrimitive)
			{
				const U32 stride = HasNormalData(pPrimitive->meshData)
					? sizeof(PrimitiveData::VertexDataSolid)
					: sizeof(PrimitiveData::VertexDataWireframe);
				const Display::C_VertexDeclaration *pDecl = HasNormalData(pPrimitive->meshData)
					? PrimitiveData::Solid::m_pDeclaration
					: PrimitiveData::Wireframe::m_pDeclaration;

				Display::Device()->SetVertexDeclaration(pDecl);
				Display::Device()->SetVertexBuffer(0, m_pVertexBuffer, pPrimitive->vertexOffset, stride);

				if (!HasNormalData(pPrimitive->meshData))
				{
					Display::Device()->SetBlendMode(Display::E_BlendMode_Alpha);
					Display::Device()->SetCullMode(Display::E_CullMode_None);
					Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);
					Display::Device()->SetVertexShader(m_ShaderManager.GetVertexShader("shaders/vsBasic.shader"));
					Display::Device()->SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasic.shader"));
				}
				else
				{
					Display::Device()->SetBlendMode((m_RenderFlags.fillmode == Display::E_FillMode_Wireframe || m_PrimitiveInstances[i].colour.A() != 0xff) ? Display::E_BlendMode_Alpha : Display::E_BlendMode_Solid);
					Display::Device()->SetCullMode((Display::E_CullMode)m_RenderFlags.cullmode);
					Display::Device()->SetFillMode((Display::E_FillMode)m_RenderFlags.fillmode);
					Display::Device()->SetVertexShader(m_ShaderManager.GetVertexShader("shaders/vsBasic.shader"));
					if (m_RenderFlags.lit)
						Display::Device()->SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicNdotLColour.shader"));
					else
						Display::Device()->SetPixelShader(m_ShaderManager.GetPixelShader("shaders/psBasicColour.shader"));
				}				
			}

			if (!i || m_PrimitiveInstances[i].colour != m_PrimitiveInstances[i-1].colour)
			{
				const F32 d = 1.f/255.f;
				F32 colour[4];

				colour[0] = d*m_PrimitiveInstances[i].colour.R();
				colour[1] = d*m_PrimitiveInstances[i].colour.G();
				colour[2] = d*m_PrimitiveInstances[i].colour.B();
				colour[3] = d*m_PrimitiveInstances[i].colour.A();

				Display::Device()->SetPixelShaderConstantF(CP_DISPLAY_COLOUR, colour, 1);
			}

			Display::C_Matrix colMatrix;
			Maths::Store(*m_PrimitiveInstances[i].pXform, &colMatrix);
			Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&colMatrix, 4);

			if (pPrimitive->meshData->GetNumIndexDataElements())
			{
				U32 startIndex = pPrimitive->indexOffset/sizeof(U16);
				for (U32 j=0; j<pPrimitive->meshData->GetNumIndexDataElements(); ++j)
				{
					const AftkRender::C_MeshClusterResource::IndexDataElement *pIndexElementData = pPrimitive->meshData->GetIndexDataElement(j);
					if (m_RenderFlags.textured)
					{
						S32 materialIndex = pIndexElementData->materialId;
						if (materialIndex != -1 && materialIndex < pPrimitive->textures.Size() && pPrimitive->textures[materialIndex]->IsCreated())
							Display::Device()->SetTexture(0, pPrimitive->textures[materialIndex]);
						else
							Display::Device()->SetTexture(0, m_TextureManager.GetDefault2DTexture());
					}
					else
						Display::Device()->SetTexture(0, m_TextureManager.GetDefault2DTexture());

					Display::Device()->DrawIndexedPrimitive(AftkRender::PrimitiveTypeLookup[pIndexElementData->type], 0, 0, pPrimitive->meshData->GetNumVertices(), startIndex, pIndexElementData->count);
					startIndex += pIndexElementData->count;
				}
			}
			else
			{
				Display::Device()->SetTexture(0, m_TextureManager.GetDefault2DTexture());
				Display::Device()->DrawPrimitive(AftkRender::PrimitiveTypeLookup[pPrimitive->meshData->GetNonIndexedPrimitiveType()], 0, pPrimitive->meshData->GetNumVertices());
			}
		}

		Display::Device()->SetCullMode(Display::E_CullMode_Default);
		Display::Device()->SetFillMode(Display::E_FillMode_Solid);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	}

	m_PrimitiveInstances.Clear();	
}


bool C_SimpleRenderer::Compare(const PrimitiveInstance& l, const PrimitiveInstance& r)
{
	bool lSolid = l.colour.A() == 0xff;
	bool rSolid = r.colour.A() == 0xff;

	if (lSolid == rSolid)
	{
		//order by type
		return (l.basePrimitive < r.basePrimitive);
	}
	else
	{
		return lSolid;
	}
}
