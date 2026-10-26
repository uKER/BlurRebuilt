#include "stdafx.h"

#include "SceneGraph/RenderingData/StaticRenderListNode.h"
#include "SceneGraph/RenderingData/StaticScene.h"

#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/Context.h"
#include "SceneGraph/View.h"
#include "SceneGraph/Util.h"
#include "SceneGraph/Visitor.h"

#include "RenderingData/RenderingData.h"
#include "RenderingData/Context.h"
#include "System/Debug/I_DebugRender.h"

#include <core/maths/hvec/new/IncludeMinimal.h>
#include <core/maths/hvec/new/IncludeMinimal.inl>

#include <fx/fx.h>

namespace SceneGraph
{

template<class Nodes>
void CullRenderingDataBounds(const C_View& view, const Nodes& nodes, const RenderingData::Bounds* bounds, const S32 begin, const S32 end, const U32 renderPassType)
{
	PERF_EVENT("StaticRenderListNode.CullBoundsST");

	Maths::HF32 farClip(view.GetViewData().farClip);

	for(S32 i = begin; i < end; ++i)
	{
		const RenderingData::Bounds& bb = bounds[i];

		// load rendering data bounds into the 3 vectors we need for culling
		Maths::HVec3 minimum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&bb.aabbMin));
		Maths::HVec3 maximum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&bb.aabbMax));
		Maths::HF32 distanceMin(bb.distanceMin);
		Maths::HF32 distanceMax(bb.distanceMax);
		Maths::HVec3 centre = (minimum + maximum) * Maths::Constants<Maths::HF32>::Half();
		Maths::HVec4 centreW(centre);
		centreW.W(Maths::Constants<Maths::HF32>::One());
		Maths::HVec3 size = maximum - centre;
		Maths::HVec4 clip(distanceMin, Maths::Min(distanceMax, farClip), Maths::Constants<Maths::HF32>::One(), Maths::Constants<Maths::HF32>::One());
		
		if(view.GetViewFrustum().CullAxisBox(centreW, Maths::HVec4(size), clip))
			view.AddVisibleNode(&nodes[i], renderPassType);
	}
}

template<class Nodes>
void CullRenderingDataBoundsST_Distance(const C_View& view, const Nodes& nodes, const Btl::Vector<RenderingData::Bounds>& bounds, const S32 begin, const S32 end, const U32 renderPassType)
{
	PERF_EVENT("StaticRenderListNode.CullBoundsST");

	Maths::HF32 farClip(view.GetViewData().farClip);
	const Display::C_Matrix& viewMatrix = view.GetViewSettings3D().GetViewMatrix();
	Maths::HVec4 forward(viewMatrix._13, viewMatrix._23, viewMatrix._33, viewMatrix._43);

	for(S32 i = begin; i < end; ++i)
	{
		const RenderingData::Bounds& bb = bounds[i];

		// load rendering data bounds into the 3 vectors we need for culling
		Maths::HVec3 minimum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&bb.aabbMin));
		Maths::HVec3 maximum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&bb.aabbMax));
		Maths::HF32 distanceMin(bb.distanceMin);
		Maths::HF32 distanceMax(bb.distanceMax);
		Maths::HVec3 centre = (minimum + maximum) * Maths::Constants<Maths::HF32>::Half();
		Maths::HVec4 centreW(centre);
		centreW.W(Maths::Constants<Maths::HF32>::One());
		Maths::HVec3 size = maximum - centre;
		Maths::HVec4 clip(distanceMin, Maths::Min(distanceMax, farClip), Maths::Constants<Maths::HF32>::One(), Maths::Constants<Maths::HF32>::One());
		
		if(view.GetViewFrustum().CullAxisBox(centreW, Maths::HVec4(size), clip))
		{
			Maths::HF32 depth = (centreW | forward);

			VolatileNodeInfo volatileNodeInfo;
			volatileNodeInfo.distance = depth;
			//S32 volatileNodeInfoIndex = view.AddVolatileNodeInfo(volatileNodeInfo);

			view.AddVisibleNode(&nodes[i], renderPassType, 0, volatileNodeInfo.data);
		}
	}
}

FORCE_INLINE void StoreMat43(MathsStorage::MatrixRow43* dest, const Maths::Mat43& src)
{
	Maths::Store(Maths::ColX(src), &dest->v[0]);
	Maths::Store(Maths::ColY(src), &dest->v[4]);
	Maths::Store(Maths::ColZ(src), &dest->v[8]);
}

FORCE_INLINE void StoreMat43(MathsStorage::MatrixRow44* dest, const Maths::Mat43& src)
{
	Maths::Store(Maths::MakeVector4(Maths::RowX(src)), &dest->v[0]);
	Maths::Store(Maths::MakeVector4(Maths::RowY(src)), &dest->v[4]);
	Maths::Store(Maths::MakeVector4(Maths::RowZ(src)), &dest->v[8]);
	Maths::Store(Maths::MakeVector4(Maths::MakePoint3(Maths::RowW(src))), &dest->v[12]);
}

// C_StaticRenderListNode

C_StaticRenderListNode::C_StaticRenderListNode(const C_SceneGraph* pSceneGraph) :
	T_RenderListNode<C_StaticPrimitive, C_StaticInstance, C_StaticInstanceBatch>(NULL, NULL),
	I_GatherNode(E_GatherNodeType_StaticRenderListNode, pSceneGraph),
	C_BoundsCull(),
	C_PassCull()
{
}

C_StaticRenderListNode::~C_StaticRenderListNode()
{
}

void C_StaticRenderListNode::Build(const C_LevelInstance *pLevelInstance, const RenderingData::Target::RenderListNode* pRenderListNode)
{
	const RenderingData::Target::Scene* pScene = pLevelInstance->GetScene();

	pLevelInstance->GetDynamicMaterialData(&m_pLevelMaterialListData, &m_pLevelMaterialListTypes, &m_pLevelMaterialListDmaData);

	if(m_pRenderListNode)
	{
		m_pRenderListNode = pRenderListNode;
		m_pNodeDmaData = m_pRenderListNode->GetDmaData();
	}
	else
	{
		PERF_EVENT("StaticRenderListNode::Build");

		m_Location = pRenderListNode->GetLocation();

		// work out which passes this node is visible in
		for(S32 i = 0; i < pRenderListNode->GetRenderPassInfoSize(); ++i)
		{
			const RenderingData::RenderListNode::RenderPassInfo& renderPassInfo = pRenderListNode->GetRenderPassInfo((RenderingData::RenderPass)i);

			// get the pass which maps to this rendering pass
			U32 renderPassType = m_pSceneGraph->GetRenderingDataRenderPassMapping((RenderingData::RenderPass)i);
			
			// if this render pass has any prims or instances then enable the render pass
			if(renderPassInfo.nPrimitives > 0 || renderPassInfo.nInstances > 0 || renderPassInfo.nInstanceBatches > 0)
				EnableRenderPass(Flag(renderPassType));
		}

		m_pScene = pScene;

		// potentially this is the only thing which needs to be updated if we cache this node when it is deleted
		m_pRenderListNode = pRenderListNode;
		m_pNodeDmaData = m_pRenderListNode->GetDmaData();

		bool bFoundPaletteId = false;

		S32 numPrimitives =	m_pRenderListNode->GetNumPrimitives();
		S32 numInstances = m_pRenderListNode->GetNumInstances();
		S32 numInstanceBatches = m_pRenderListNode->GetNumInstanceBatches();

		m_PrimitiveNodes.Clear();
		m_InstanceNodes.Clear();
		m_InstanceBatchNodes.Clear();

		{
			// this is expensive. cache this entire class after its been deleted so we only ever do this once?
			PERF_EVENT("MemoryAllocation");
			m_PrimitiveNodes.Resize(numPrimitives);
			m_InstanceNodes.Resize(numInstances);
			m_InstanceBatchNodes.Resize(numInstanceBatches);
		}

		{
			PERF_EVENT("Primitives");
			for(S32 i = 0; i < numPrimitives; ++i)
			{
				m_PrimitiveNodes[i].m_Type = E_RenderNodeType_StaticPrimitive;
				m_PrimitiveNodes[i].m_pNode = this;
				m_PrimitiveNodes[i].m_PrimitiveIndex = i;
			}
		}
		
		{
			PERF_EVENT("Instances");
			for(S32 i = 0; i < numInstances; ++i)
			{
				const RenderingData::Material& instanceMaterial = m_pRenderListNode->GetConstantMaterialListData().GetMaterial(m_pRenderListNode->GetInstance(i).material.index);
				const RenderingData::MaterialType& instanceMaterialType = m_pRenderListNode->GetConstantMaterialTypes().GetMaterialType(instanceMaterial.GetTypeIndex());

				for (S32 iElement = 0, elementCount = instanceMaterialType.GetNumElements(); iElement < elementCount; ++iElement)
				{
					const RenderingData::MaterialElement& element = instanceMaterialType.GetElement(iElement);

					if (element.type != RenderingData::MATERIAL_ELEMENT_TYPE_DATA_INDEX || element.count != 1)
						continue;
					
					if (_stricmp(element.name.Chars(), "PaletteData") == 0)
					{
						const RenderingData::MaterialElement& indexMaterialElement = instanceMaterialType.GetElement(iElement);

						BIZ_ASSERT(indexMaterialElement.count > 0 && indexMaterialElement.type == RenderingData::MATERIAL_ELEMENT_TYPE_DATA_INDEX);

						S32 indexOffset = instanceMaterialType.GetOffset(iElement, 0);

						if (indexOffset >= 0)
						{
							RenderingData::MaterialIndex materialIndex = instanceMaterial.GetData<RenderingData::MaterialIndex>(indexOffset);

							const RenderingData::Material& paletteDataMaterial = m_pRenderListNode->GetConstantMaterialListData().GetMaterial(materialIndex.index);
							const RenderingData::MaterialType& paletteDataMaterialType = m_pRenderListNode->GetConstantMaterialTypes().GetMaterialType(paletteDataMaterial.GetTypeIndex());

							if (paletteDataMaterialType.GetNumElements() > 0)
							{
								const RenderingData::MaterialElement& paletteDataElement = paletteDataMaterialType.GetElement(0);

								if (paletteDataElement.count == 2 && paletteDataElement.type == RenderingData::MATERIAL_ELEMENT_TYPE_S16)
								{
									S32 offset = paletteDataMaterialType.GetOffset(0, 0);

									if (offset >= 0)
									{
										S16 paletteId = paletteDataMaterial.GetData<S16>(offset);
										m_InstanceNodes[i].m_PaletteId = paletteId;

										bFoundPaletteId = true;
									}

									if (bFoundPaletteId)
										break;
								}
								else
									break;
							}
						}
					}
				}

				m_InstanceNodes[i].m_Type = E_RenderNodeType_StaticInstance;
				m_InstanceNodes[i].m_pNode = this;
				m_InstanceNodes[i].m_InstanceIndex = i;
			}
		}

		{
			PERF_EVENT("InstanceBatches");
			for(S32 i = 0; i < numInstanceBatches; ++i)
			{
				m_InstanceBatchNodes[i].m_Type = E_RenderNodeType_StaticInstanceBatch;
				m_InstanceBatchNodes[i].m_pNode = this;
				m_InstanceBatchNodes[i].m_InstanceBatchIndex = i;
			}
		}

		if (m_pRenderListNode->GetNumDynamicMaterialLists() > 1 && m_pRenderListNode->GetDynamicMaterialList(1).GetNumMaterials())
		{
			if (bFoundPaletteId)
				BuildDynamicMaterialList(static_cast<U32>(m_pSceneGraph->GetNumMatrixPalettes()));
			else
				BuildDynamicMaterialList(0);
		}
		
		SetAxisBox(pRenderListNode->GetBounds());
	}
}

void C_StaticRenderListNode::BeginFrame() const
{
}

// we are gathering
void C_StaticRenderListNode::Gather(const C_View& view) const
{
	if(!view.RenderListEnabled())
		return;

	// see if we can pass cull the entire thing
	if(view.RenderPassEnabled(RenderPasses()))
	{
		// frustum cull this entire render list
		S32 visible = CullAxisBox(view);
		
		if(visible)
		{
			PERF_EVENT("StaticRenderListNode::Gather");
			
			if(view.RenderNodeEnabled(Flag<E_RenderNodeType_StaticPrimitive>()))
			{
				for(S32 i = 0; i < RenderingData::RENDER_PASS_MAX; ++i)
				{
					const RenderingData::RenderListNode::RenderPassInfo& renderPassInfo = GetRenderListNode()->GetRenderPassInfo((RenderingData::RenderPass)i);
					
					U32 renderPassType = m_pSceneGraph->GetRenderingDataRenderPassMapping((RenderingData::RenderPass)i);
			
					if(view.RenderPassEnabled(Flag(renderPassType)))
					{
						S32 nodeBegin = renderPassInfo.primitiveOffset;
						S32 nodeEnd = renderPassInfo.primitiveOffset + renderPassInfo.nPrimitives;

						CullRenderingDataBounds(view, GetPrimitiveNodes(), GetRenderListNode()->GetPrimitiveBounds(), nodeBegin, nodeEnd, renderPassType);
					}
				}
			}
			
			if(view.RenderNodeEnabled(Flag<E_RenderNodeType_StaticInstance>()))
			{
				for(S32 i = 0; i < RenderingData::RENDER_PASS_MAX; ++i)
				{
					const RenderingData::RenderListNode::RenderPassInfo& renderPassInfo = GetRenderListNode()->GetRenderPassInfo((RenderingData::RenderPass)i);
					
					//U32 renderPassType = GetSceneGraphRenderPass((RenderingData::RenderPass)i);
					U32 renderPassType = m_pSceneGraph->GetRenderingDataRenderPassMapping((RenderingData::RenderPass)i);
			
					if(view.RenderPassEnabled(Flag(renderPassType)))
					{
						S32 nodeBegin = renderPassInfo.instanceOffset;
						S32 nodeEnd = renderPassInfo.instanceOffset + renderPassInfo.nInstances;

						CullRenderingDataBounds(view, GetInstanceNodes(), GetRenderListNode()->GetInstanceBounds(), nodeBegin, nodeEnd, renderPassType);
					}
				}	
			}

			// gather up instance batch nodes
			for(S32 i = 0; i < GetInstanceBatchNodes().Size(); ++i)
			{
				
			}
			
		}
	}
}

void C_StaticRenderListNode::Accept(I_Visitor& visitor) const
{
	visitor.Visit(*this);
}

void C_StaticRenderListNode::BuildDynamicMaterialList(const U32 numMatrixPalettes)
{
	if (m_pSceneGraph->GetStaticNodeInternalMaterialData() && m_pSceneGraph->GetStaticNodeInternalMaterialData()->m_pMaterialList)
		return;

	C_CachedStaticNodeInternalMaterialData* pInternalData = new C_CachedStaticNodeInternalMaterialData();
	pInternalData->m_AnimatedParamIndices.Clear();

	if(m_pRenderListNode)
	{
		pInternalData->m_pMaterialList = new RenderingData::Target::MaterialList();
		pInternalData->m_pMaterialList->ClearAll();
		pInternalData->m_pMaterialList->Begin();

		const RenderingData::MaterialList& materialInput = m_pRenderListNode->GetDynamicMaterialList(1);

		pInternalData->m_MaterialListSize = materialInput.GetNumMaterials();

		for (U32 iPalette = 0, paletteCount = numMatrixPalettes + 1; iPalette < paletteCount; ++iPalette)
		{
			for (S32 mat = 0, matCount = materialInput.GetNumMaterials(); mat < matCount; ++mat)
			{
				// get the material and material type
				RenderingData::ConstMaterialReference input = materialInput.GetMaterial(mat);
				const RenderingData::MaterialType& inputType = materialInput.GetMaterialType(input.GetTypeIndex());

				if (inputType.GetName() == "AnimatedParam")
				{
					// read data type and count
					U32 type = input.GetData<U32>(inputType.GetOffset(0, 0));
					U32 count = input.GetData<U32>(inputType.GetOffset(0, 1));
					
					if (count == 16)
						count = 12;

					// setup the material type
					RenderingData::MaterialType animatedMaterialType;
					animatedMaterialType.SetName(inputType.GetElement(0).name);

					// add the element
					RenderingData::MaterialElement animatedElement;
					animatedElement.name = ""; // Not used
					animatedElement.type = (RenderingData::MaterialElementType)type;
					animatedElement.count = count;

					animatedMaterialType.AddElement(animatedElement, 16);

					// add the material to the material list
					S32 animatedMaterialIndex	= pInternalData->m_pMaterialList->AddMaterialType(animatedMaterialType);
					S32 animatedParamIndex		= pInternalData->m_pMaterialList->AddMaterial(animatedMaterialIndex);

					if (iPalette == 0)
					{
						if (pInternalData->m_AnimatedParamIndices.Size() < pInternalData->m_AnimatedParamIndices.Capacity())
							pInternalData->m_AnimatedParamIndices.PushBack(animatedParamIndex);
					}
				}
			}
		}

		pInternalData->m_pMaterialList->End();

		//const_cast<RenderingData::Target::RenderListNode*>(m_pRenderListNode)->InitialiseDynamicMaterialList(1, pInternalData->m_pMaterialList);

		// move to a single buffer
		{
			RenderingData::Target::MaterialList temp = *pInternalData->m_pMaterialList;
			Btl::SwapByMemcopy(temp, *pInternalData->m_pMaterialList);
		}

		pInternalData->m_pMaterialListDmaData = pInternalData->m_pMaterialList->GetDmaData();

		if (numMatrixPalettes > 0)
		{
			pInternalData->m_MaterialDataList.Destroy();
			pInternalData->m_MaterialDataList.Resize(numMatrixPalettes);
		}
	}

	Maths::HMat43 identity;
	identity.SetIdentity();

	for(S32 i = 0, paramCount = pInternalData->m_AnimatedParamIndices.Size(); i < paramCount; ++i)
	{
		const RenderingData::MaterialType& materialType = pInternalData->m_pMaterialList->GetTypeOfMaterial(pInternalData->m_AnimatedParamIndices[i]);

		for (S32 j = 0, elementCount = materialType.GetNumElements(); j < elementCount; ++j)
		{
			if (materialType.GetElementDataSize(j) == 48)
			{
				RenderingData::Material& material = pInternalData->m_pMaterialList->GetMaterial(pInternalData->m_AnimatedParamIndices[i]);

				U8* pData = material.GetData(materialType.GetOffset(0, 0));

				MathsStorage::MatrixRow43* destMatrix = reinterpret_cast<MathsStorage::MatrixRow43*>(pData);
				StoreMat43(destMatrix, identity);
			}
		}
	}

	m_pSceneGraph->SetStaticNodeInternalMaterialData(pInternalData);
}


// C_StaticRenderListItem

extern Debug::Bool g_BoundsDrawDistance;
extern Debug::Bool g_BoundsPredication;

void C_StaticRenderListItem::DebugRenderBounds(const C_VisibleNode& node, const RenderingData::Bounds& bounds, const Maths::RGBA& colour) const
{
	//
	Maths::HVec3 minimum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&bounds.aabbMin));
	Maths::HVec3 maximum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&bounds.aabbMax));
	Maths::HVec3 centre = (minimum + maximum) * Maths::Constants<Maths::HF32>::Half();
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(minimum, maximum, Maths::HMat43(), colour);

	if(g_BoundsDrawDistance)
	{
		Util::C_String drawDistance;
		if(bounds.distanceMin > -FLT_MAX && bounds.distanceMax < FLT_MAX)
			drawDistance.Printf("%.2f:%.2f", bounds.distanceMin, bounds.distanceMax);
		else if(bounds.distanceMin > -FLT_MAX)
			drawDistance.Printf("%.2f:INF", bounds.distanceMin);
		else if(bounds.distanceMax < FLT_MAX)
			drawDistance.Printf("%.2f", bounds.distanceMax);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(centre, drawDistance);
	}

}

void C_StaticRenderListItem::DebugRenderPaletteId(const RenderingData::Bounds& bounds, const S16 paletteId) const
{
	Maths::HVec3 minimum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&bounds.aabbMin));
	Maths::HVec3 maximum(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&bounds.aabbMax));
	Maths::HVec3 centre = (minimum + maximum) * Maths::Constants<Maths::HF32>::Half();

	Util::C_String paletteIdString;
	paletteIdString.Printf("%d", paletteId);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(centre, paletteIdString);
}


extern Debug::Bool g_RenderingDataRender;

// C_StaticPrimitive

Debug::Bool	g_BoundsPrimitives("Graphics.SceneGraph.Bounds.Nodes.Primitives", true);

void C_StaticPrimitive::DebugRender(const C_VisibleNode& visibleNode) const
{
	if(g_BoundsPrimitives)
		DebugRenderBounds(visibleNode, m_pNode->GetRenderListNode()->GetPrimitiveBounds(m_PrimitiveIndex), Maths::RGBA(255, 0, 0, 128));
}

// C_StaticInstance

Debug::Bool	g_BoundsInstances("Graphics.SceneGraph.Bounds.Nodes.Instances", true);
Debug::Bool g_PaletteIdsInstances("Graphics.SceneGraph.Bounds.Nodes.InstancePaletteIds", false);

void C_StaticInstance::DebugRender(const C_VisibleNode& visibleNode) const
{
	if(g_BoundsInstances)
		DebugRenderBounds(visibleNode, m_pNode->GetRenderListNode()->GetInstanceBounds(m_InstanceIndex), Maths::RGBA(255, 255, 0, 128));
	if(g_PaletteIdsInstances && m_PaletteId >= 0)
		DebugRenderPaletteId(m_pNode->GetRenderListNode()->GetInstanceBounds(m_InstanceIndex), m_PaletteId);
}

} // namespace SceneGraph

