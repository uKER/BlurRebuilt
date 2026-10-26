
#include "stdafx.h"

#include <system/serialise/C_FileBlock.h>
#include <world_render/WorldRender.h>
#include <world_render/RenderModelData.h>
#include <world_render/RenderModelResourceUsage.h>
#include <renderingdata/renderingdata.h>
#include <fx/fx.h>


#include <system/Memory.h>

Debug::Bool g_DefaultMatrixParamsToIdentity("Graphics.WorldRender.Debug.DefaultMatrixParamsToIdentity", false);


namespace WorldRender
{

struct Element
{
	RenderingData::MaterialType			materialType;
	S32									materialIndex;
};

struct C_ModelInternalData
{
	Btl::Vector<Element>					m_Elements;
	Btl::Vector<S32>						m_AnimatedParams;

	RenderingData::MaterialType				m_MaterialType;
	RenderingData::Target::MaterialList		m_MaterialList;
};

// C_Model

C_Model::C_Model()
{
	m_pData = new C_ModelInternalData;
	m_pszFriendlyName = NULL;
}

C_Model::~C_Model()
{
	delete m_pData;
}

C_Model::C_Model(const C_Model &t)
{
	m_pData = new C_ModelInternalData;
	*m_pData = *t.m_pData;
	m_WorldMatrices = t.m_WorldMatrices;
	m_pRenderListNode = t.m_pRenderListNode;
	m_MaterialIndex = t.m_MaterialIndex;
	m_pszFriendlyName = t.m_pszFriendlyName;
}

const RenderingData::Target::MaterialList& C_Model::GetMaterialList() const
{
	return m_pData->m_MaterialList;
}

S32 C_Model::GetNumElements() const
{
	return m_pData->m_Elements.Size();
}

void C_Model::ExposeParameter(S32 elementIndex, const Btl::String& name, RenderingData::MaterialElementType type, S32 count)
{
	if (elementIndex >= m_pData->m_Elements.Size())
		return; 

	Element& element = m_pData->m_Elements[elementIndex];

	RenderingData::MaterialElement elementMaterialElement;
	elementMaterialElement.name = name;
	elementMaterialElement.type = type;
	elementMaterialElement.count = count;

	element.materialType.AddElement(elementMaterialElement);
}

void C_Model::ExposeParameter(const Btl::String& name, RenderingData::MaterialElementType type, S32 count)
{
	if (!m_pRenderListNode)
		return;

	RenderingData::MaterialElement modelMaterialElement;
	modelMaterialElement.name = name;
	modelMaterialElement.type = type;
	modelMaterialElement.count = count;

	m_pData->m_MaterialType.AddElement(modelMaterialElement);
}

MaterialParameter C_Model::GetParameter(S32 elementIndex, const char* pName) const
{
	if (elementIndex < m_pData->m_Elements.Size())
	{
		// get handle from element specific material type
		const RenderingData::MaterialType& materialType = m_pData->m_Elements[elementIndex].materialType;

		for(S32 i = 0; i < materialType.GetNumElements(); ++i)
		{
			const RenderingData::MaterialElement& element = materialType.GetElement(i);
			
			if (!strcmp(element.name.Chars(), pName))
			{
				MaterialParameter handle;
				handle.index = m_pData->m_Elements[elementIndex].materialIndex;
				handle.offset = materialType.GetOffset(i, 0);
				handle.size = materialType.GetElementDataSize(i);

				return handle;
			}
		}
	}

	return MaterialParameter();
}

MaterialParameter C_Model::GetParameter(const char* pName) const
{
	// no element index specified, look for parameter in model-wide material type
	for(S32 i = 0; i < m_pData->m_MaterialType.GetNumElements(); ++i)
	{
		const RenderingData::MaterialElement& element = m_pData->m_MaterialType.GetElement(i);
		
		if (!strcmp(element.name.Chars(), pName))
		{
			MaterialParameter handle;
			handle.index = m_MaterialIndex.index;
			handle.offset = m_pData->m_MaterialType.GetOffset(i, 0);
			handle.size = m_pData->m_MaterialType.GetElementDataSize(i);

			return handle;
		}
	}

	// now look in animated parameters
	for(S32 i = 0; i < m_pData->m_AnimatedParams.Size(); ++i)
	{
		int iMtl = m_pData->m_AnimatedParams[i];
		const RenderingData::MaterialList *pInputMtlList = GetDynamicMaterialInput();
		const RenderingData::MaterialType& nodeMaterialType = pInputMtlList->GetTypeOfMaterial(iMtl);
		const RenderingData::MaterialType& materialType = m_pData->m_MaterialList.GetTypeOfMaterial(iMtl);

		if (!strcmp(nodeMaterialType.GetElement(0).name.Chars(), pName))
		{
			MaterialParameter handle;
			handle.index = m_pData->m_AnimatedParams[i];
			handle.offset = materialType.GetOffset(0, 0);
			handle.size = materialType.GetElementDataSize(0);

			return handle;
		}
	}

	return MaterialParameter();
}

void C_Model::SetDefaultValues(RenderingData::Target::MaterialListData *pModelInstanceMtlList) const
{
	// for each parameter exposed on the model, set the default value if neccessary
	void *pGPUTex = FX::GetDefaultGPUTexture();
	for(S32 i = 0; i < m_pData->m_MaterialType.GetNumElements(); ++i)
	{
		const RenderingData::MaterialElement& element = m_pData->m_MaterialType.GetElement(i);
		RenderingData::Material& material = pModelInstanceMtlList->GetMaterial(m_MaterialIndex.index);

		if (element.type == RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_PTR)
		{
			material.SetData(m_pData->m_MaterialType.GetOffset(i, 0), &pGPUTex, m_pData->m_MaterialType.GetElementDataSize(i));
		}
	}

#ifndef CFG_SPEED
	if (g_DefaultMatrixParamsToIdentity)
	{
		MathsStorage::MatrixRow44 ident44;
		memset(&ident44, 0, sizeof(ident44));
		ident44.rx.x = ident44.ry.y = ident44.rz.z = ident44.rw.w = 1.f;
		MathsStorage::MatrixRow43 ident43;
		memset(&ident43, 0, sizeof(ident43));
		ident43.rx.x = ident43.ry.y = ident43.rz.z = 1.f;

		for(S32 matIndex = 0; matIndex < pModelInstanceMtlList->GetNumMaterials(); ++matIndex)
		{
			//for each material in the pModelInstanceMtlList
			RenderingData::Material& material = pModelInstanceMtlList->GetMaterial(matIndex);
			//Get the type of this material 
			S32 typeIndex = material.GetTypeIndex();
			const RenderingData::MaterialType& type = m_pData->m_MaterialList.GetMaterialType(typeIndex);
			//for each element in the type
			for (S32 elementIndex=0; elementIndex<type.GetNumElements(); elementIndex++)
			{
				const RenderingData::MaterialElement& element = type.GetElement(elementIndex);
				//if it looks matrixey, default it to identity
				if (element.type == RenderingData::MATERIAL_ELEMENT_TYPE_F32)
				{
					if (element.count == 16)
					{
						// 4x4 matrices (e.g. element world matrices)
						material.SetData(type.GetOffset(elementIndex, 0), &ident44, type.GetElementDataSize(elementIndex));
					}
					else if (element.count == 12)
					{
						// 4x3 matrices (e.g. skinning matrices)
						material.SetData(type.GetOffset(elementIndex, 0), &ident43, type.GetElementDataSize(elementIndex));
					}
				}
			}
		}
	}
#endif
}

void C_Model::ReadMaterialInput()
{
	MEMORY_TRACKER_PUSH("worldrender.model.materiallist.read");
	m_pData->m_Elements.Clear();

	if(m_pRenderListNode)
	{
		RenderingData::MaterialType renderListMaterialType;
		renderListMaterialType.SetName("RenderListMaterial");

		const RenderingData::MaterialList *pMaterialInput = GetDynamicMaterialInput();
		if (pMaterialInput)
		{
			for(S32 mat = 0; mat < pMaterialInput->GetNumMaterials(); ++mat)
			{
				// get the material and material type
				RenderingData::ConstMaterialReference input = pMaterialInput->GetMaterial(mat);
				const RenderingData::MaterialType& inputType = pMaterialInput->GetMaterialType(input.GetTypeIndex());

				// look for the element material
				if (inputType.GetName() == "Element")
				{
					assert(inputType.GetNumElements() == 1);
					assert(inputType.GetElement(0).name == "Index");

					S32 offset = inputType.GetOffset(0, 0);
					U32 index = input.GetData<U32>(offset);

					// create new material type for this element
					RenderingData::MaterialType elementMaterialType;

					// add world matrix element
					{
						RenderingData::MaterialElement worldMatrix;
						worldMatrix.name = "g_mWorld";
						worldMatrix.type = RenderingData::MATERIAL_ELEMENT_TYPE_F32;
						worldMatrix.count = 16;
						
						elementMaterialType.AddElement(worldMatrix, 16);
					}
					
					if(static_cast<U32>(m_pData->m_Elements.Size()) <= index)
					{
						m_pData->m_Elements.Resize(index + 1);
					}

					// assign the offset to the element - defer this until later
					m_pData->m_Elements[index].materialType = elementMaterialType;
					m_pData->m_Elements[index].materialIndex = -1;
				}
			}
		}

		m_pData->m_MaterialType = renderListMaterialType;
	}
	else
		LOGDEBUG(g_Log, "RenderModelData: Model has no RenderListNode.");

	MEMORY_TRACKER_POP("worldrender.model.materiallist.read");
}

const RenderingData::MaterialList *C_Model::GetDynamicMaterialInput() const
{
	if (m_pRenderListNode->GetNumDynamicMaterialLists() == 1)
		return &m_pRenderListNode->GetDynamicMaterialList(0);

	if (m_pRenderListNode->GetScenePtr()->GetNumDynamicMaterialLists() == 1)
		return m_pRenderListNode->GetScenePtr()->GetDynamicMaterialList(0);

	return NULL;
}

void C_Model::BuildDynamicMaterialList()
{
	MEMORY_TRACKER_PUSH("worldrender.model.materiallist.build");

	m_pData->m_AnimatedParams.Clear();

	if(m_pRenderListNode)
	{
		m_pData->m_MaterialList.ClearAll();
		m_pData->m_MaterialList.Begin();

		const RenderingData::MaterialList *pMaterialInput = GetDynamicMaterialInput();
		if (pMaterialInput)
		{
			for(S32 mat = 0; mat < pMaterialInput->GetNumMaterials(); ++mat)
			{
				// get the material and material type
				RenderingData::ConstMaterialReference input = pMaterialInput->GetMaterial(mat);
				const RenderingData::MaterialType& inputType = pMaterialInput->GetMaterialType(input.GetTypeIndex());

				// look for the element material
				if (inputType.GetName() == "Element")
				{
					MEMORY_TRACKER_PUSH("worldrender.model.materiallist.build.element");
					S32 offset = inputType.GetOffset(0, 0);
					U32 index = input.GetData<U32>(offset);

					S32 elementMaterialIndex = m_pData->m_MaterialList.AddMaterialType(m_pData->m_Elements[index].materialType);
					m_pData->m_Elements[index].materialIndex = m_pData->m_MaterialList.AddMaterial(elementMaterialIndex);
					MEMORY_TRACKER_POP("worldrender.model.materiallist.build.element");
				}
				else if(inputType.GetName() == "AnimatedParam")
				{
					MEMORY_TRACKER_PUSH("worldrender.model.materiallist.build.animated");
					// read data type and count
					U32 type = input.GetData<U32>(inputType.GetOffset(0, 0));
					U32 count = input.GetData<U32>(inputType.GetOffset(0, 1));

					// setup the material type
					RenderingData::MaterialType animatedMaterialType;

					// add the element
					RenderingData::MaterialElement animatedElement;
					animatedElement.name = ""; // not used
					animatedElement.type = (RenderingData::MaterialElementType)type;
					animatedElement.count = count;

					animatedMaterialType.AddElement(animatedElement, 16);

					// add the material to the material list
					S32 animatedMaterialIndex = m_pData->m_MaterialList.AddMaterialType(animatedMaterialType);
					m_pData->m_AnimatedParams.PushBack(m_pData->m_MaterialList.AddMaterial(animatedMaterialIndex));
					MEMORY_TRACKER_POP("worldrender.model.materiallist.build.animated");
				}
			}
		}
		else
		{
			LOGERROR( g_Log, "Model '%s' needs re-exporting\n", m_pszFriendlyName);
		}

		// add the material for the whole render list
		S32 renderListMaterialIndex = m_pData->m_MaterialList.AddMaterialType(m_pData->m_MaterialType);
		m_MaterialIndex.index = m_pData->m_MaterialList.AddMaterial(renderListMaterialIndex);
		m_MaterialIndex.listIndex = 1;

		m_pData->m_MaterialList.End();

		m_pRenderListNode->InitialiseDynamicMaterialList(0, &m_pData->m_MaterialList);
	}
	else
	{
		LOGDEBUG(g_Log, "RenderModelData: Model has no RenderListNode.");
	}

	// extract MaterialElementHandle for world matrices
	for(S32 i = 0; i < m_pData->m_Elements.Size(); ++i)
	{
		MaterialParameter parameter = GetParameter(i, "g_mWorld");
		m_WorldMatrices.PushBack(parameter);
	}
	MEMORY_TRACKER_POP("worldrender.model.materiallist.build");
}


// C_RenderModelData

C_RenderModelData::C_RenderModelData()
{
	m_pScene = new RenderingData::Target::Scene;
	m_pData = new C_ModelInternalData;
}

C_RenderModelData::~C_RenderModelData()
{
	delete m_pScene;
	delete m_pData;
}

RenderingData::Target::Scene& C_RenderModelData::GetScene()
{
	return *m_pScene;
}

const RenderingData::Target::Scene& C_RenderModelData::GetScene() const
{
	return *m_pScene;
}

void C_RenderModelData::BuildModelsStart()
{
	// extract renderlist nodes from scene
	BuildModels(m_pScene->GetRootNode(), -1);

	// build up dynamic material output list for models
	for(S32 m = 0; m < m_Models.Size(); ++m)
	{
		m_Models[m].ReadMaterialInput();
	}
}

void C_RenderModelData::BuildModelsEnd()
{
	BuildDynamicMaterialList();

	for(S32 m = 0; m < m_Models.Size(); ++m)
	{
		m_Models[m].BuildDynamicMaterialList();
	}
}

MaterialParameter C_RenderModelData::GetParameter(const char* pName) const
{
	if (m_pScene->GetNumDynamicMaterialLists() == 3)
	{
		for(S32 i = 0; i < m_pData->m_AnimatedParams.Size(); ++i)
		{
			int iMtl = m_pData->m_AnimatedParams[i];
			const RenderingData::MaterialList *pInputMtlList = m_pScene->GetDynamicMaterialList(2);
			const RenderingData::MaterialType& nodeMaterialType = pInputMtlList->GetTypeOfMaterial(iMtl);
			const RenderingData::MaterialType& materialType = m_pData->m_MaterialList.GetTypeOfMaterial(iMtl);

			if (!strcmp(nodeMaterialType.GetElement(0).name.Chars(), pName))
			{
				MaterialParameter handle;
				handle.index = m_pData->m_AnimatedParams[i];
				handle.offset = materialType.GetOffset(0, 0);
				handle.size = materialType.GetElementDataSize(0);

				return handle;
			}
		}
	}

	return MaterialParameter();
}

#ifndef CFG_GOLD

void C_RenderModelData::DumpParameters() const
{
	for(S32 i = 0; i < m_pData->m_AnimatedParams.Size(); ++i)
	{
		int iMtl = m_pData->m_AnimatedParams[i];
		const RenderingData::MaterialList *pInputMtlList = m_pScene->GetDynamicMaterialList(2);
		const RenderingData::MaterialType& nodeMaterialType = pInputMtlList->GetTypeOfMaterial(iMtl);

		LOGINFO(g_Log, "C_RenderModelData::GetParameter: %d/%d - '%s'", i, m_pData->m_AnimatedParams.Size(), nodeMaterialType.GetElement(0).name.Chars());
	}
}

#endif

void C_RenderModelData::BuildDynamicMaterialList()
{
	MEMORY_TRACKER_PUSH("worldrender.model.materiallist.build");

	m_pData->m_AnimatedParams.Clear();

	m_pData->m_MaterialList.ClearAll();
	m_pData->m_MaterialList.Begin();

	if (m_pScene->GetNumDynamicMaterialLists() == 3)
	{
		const RenderingData::MaterialList *pMaterialInput = m_pScene->GetDynamicMaterialList(2);

		for(S32 mat = 0; mat < pMaterialInput->GetNumMaterials(); ++mat)
		{
			// get the material and material type
			RenderingData::ConstMaterialReference input = pMaterialInput->GetMaterial(mat);
			const RenderingData::MaterialType& inputType = pMaterialInput->GetMaterialType(input.GetTypeIndex());

			// look for the element material
			BIZ_ASSERT(inputType.GetName() != "Element");
			if(inputType.GetName() == "AnimatedParam")
			{
				MEMORY_TRACKER_PUSH("worldrender.model.materiallist.build.animated");
				// read data type and count
				U32 type = input.GetData<U32>(inputType.GetOffset(0, 0));
				U32 count = input.GetData<U32>(inputType.GetOffset(0, 1));

				// setup the material type
				RenderingData::MaterialType animatedMaterialType;

				// add the element
				RenderingData::MaterialElement animatedElement;
				animatedElement.name = ""; // not used
				animatedElement.type = (RenderingData::MaterialElementType)type;
				animatedElement.count = count;

				animatedMaterialType.AddElement(animatedElement, 16);

				// add the material to the material list
				S32 animatedMaterialIndex = m_pData->m_MaterialList.AddMaterialType(animatedMaterialType);
				m_pData->m_AnimatedParams.PushBack(m_pData->m_MaterialList.AddMaterial(animatedMaterialIndex));
				MEMORY_TRACKER_POP("worldrender.model.materiallist.build.animated");
			}
		}
	}

	m_pData->m_MaterialList.End();

	MEMORY_TRACKER_POP("worldrender.model.materiallist.build");
}

void C_RenderModelData::BuildModels(RenderingData::Node& node, S32 modelIndex)
{
	if(node.GetName() == "Static")
		return;

	if (node.GetNodeType() == RenderingData::NODE_TYPE_ROOT)
	{
		for(S32 i = 0; i < node.GetNumSubNodes(); i++)
			BuildModels(*m_pScene->GetNodePtr(node.GetSubNode(i)), modelIndex);
	}
	else if(node.GetNodeType() == RenderingData::NODE_TYPE_CULL)
	{
		// check name probable
		modelIndex = m_Models.Size();
		m_Models.PushBack();

		m_Models[modelIndex].m_pRenderListNode = NULL;

		for(S32 i = 0; i < node.GetNumSubNodes(); i++)
			BuildSubModels(*m_pScene->GetNodePtr(node.GetSubNode(i)), modelIndex);
	}	
}

void C_RenderModelData::BuildSubModels(RenderingData::Node& node, S32 modelIndex)
{
	RenderingData::Target::RenderListNode* pRenderListNode = RenderingData::Target::RenderListNode::GetPointer(&node);
	if(pRenderListNode)
	{
		C_Model& model = m_Models[modelIndex];
		if(model.m_pRenderListNode == NULL)
			 model.m_pRenderListNode = pRenderListNode;
		else
			LOGWARN(g_Log, "RenderModelData: Model '%s' has more than 1 RenderListNode. This is not allowed", model.GetFriendlyName());

	}

	for(S32 i = 0; i < node.GetNumSubNodes(); i++)
		BuildSubModels(*m_pScene->GetNodePtr(node.GetSubNode(i)), modelIndex);
}

const RenderingData::Target::MaterialList& C_RenderModelData::GetMaterialList() const
{
	return m_pData->m_MaterialList;
}

void RenderModelResourceUsage::AccumulateRenderingDataTextures(const RenderingData::Target::TextureInfoList& renderingData)
{
	for (RenderingData::Target::TextureInfoList::ConstIterator it = renderingData.Begin(); it != renderingData.End(); ++it)
	{
		RenderModelTextureInfo info;
		{
			const RenderingData::Target::Texture* pRenderTexture = it->pTexture;
			const Display::C_Texture2D& displayTexture = pRenderTexture->GetData();

			U32 flags = 0;
			{
				if (it->flags & RenderingData::Target::TextureInfo::E_Flag_Shared)
				{
					flags |= RenderModelTextureInfo::E_Flag_Shared;
				}
			}

			info.flags = flags;
			info.uSizeInBytes = displayTexture.GetSize();
			info.uWidth = displayTexture.GetWidth();
			info.uHeight = displayTexture.GetHeight();
			info.uLevels = displayTexture.GetLevels();
			info.eFormat = Display::GetFormatFromTextureFormat(displayTexture.GetFormat());
			info.textureName = pRenderTexture->GetName();
			info.pDisplayTexture = &displayTexture;
		}

		textureInfoList.Insert(info);
	}
}

void C_RenderModelData::AccumulateResourceUsage(RenderModelResourceUsage& ioResourceUsage, S32 modelIndex) const
{
	// TODO: if (modelIndex != -1) then filter out resources not used by that model

	RenderingData::Target::TextureInfoList renderingData;
	m_pScene->AccumulateTextureInfo(renderingData);
	ioResourceUsage.AccumulateRenderingDataTextures(renderingData);
}

} // namespace WorldRender
