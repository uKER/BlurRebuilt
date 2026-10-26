#include "stdafx.h"

// scenegraph includes
#include "SceneGraph/RenderingData/ModelInstanceNode.h"

#include "SceneGraph/Context.h"
#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/View.h"
#include "SceneGraph/MultiView.h"
#include "SceneGraph/Util.h"
#include "SceneGraph/Visitor.h"
#include "SceneGraph/VisibleDetect.h"

// debug
#include "System/Debug/I_DebugRender.h"

// rendering data
#include "RenderingData/RenderingData.h"
#include "RenderingData/Context.h"

namespace SceneGraph
{

Debug::Bool	g_BoundsModelInstances("Graphics.SceneGraph.Bounds.Nodes.ModelInstances", true);
extern Debug::Bool g_UsePredication;

extern Debug::Logger &g_Log;

// C_ModelInstance

C_ModelInstance::C_ModelInstance(const C_SceneGraph* pSceneGraph, const RenderingData::Target::Scene* pScene, const RenderingData::Target::RenderListNode* pRenderListNode) :
T_RenderListNode<C_ModelInstancePrimitive, C_ModelInstanceDummyObject, C_ModelInstanceDummyObject>(pScene, pRenderListNode),
I_RenderNode(E_RenderNodeType_ModelInstance),
m_pVisDetect(0),
m_pMaterialListTypes(0),
m_pMaterialListDmaData(0),
m_pCullNode(0),
m_pSceneGraph(pSceneGraph),
m_Elements(0),
m_VisibleElements(0),
m_VolatileInfoFlags(0),
m_SubtechniqueHandle(0),
m_VisibilityFlags(0xFFFFFFFF),
m_PerPrimitiveGather(false),
m_pParentMatrix(NULL),
m_pszFriendlyName(NULL),
m_Frame(0)
{
	MEMORY_TRACKER_PUSH("graphics.scenegraph.modelinstance");
	m_pMaterialListData = new RenderingData::Target::MaterialListData;

	// get default mappings for the different rendering data passes
	for(S32 i = 0; i < RenderingData::RENDER_PASS_MAX; ++i)
	{
		m_RdToSgPassMapping[i] = pSceneGraph->GetRenderingDataRenderPassMapping((RenderingData::RenderPass)i);
	}

	// extract primitives - TODO: move this to external obejct
	if(m_pRenderListNode)
	{
		// find max number of elements
		S32 maxElementIndex = 0;
		S32 numPrimitives =     m_pRenderListNode->GetNumPrimitives();
		S32 primitivesVisibilityFlags = UINT_MAX;
		S32 visibilityFlags = 0;
		m_PrimitiveNodes.Clear();
		m_PrimitiveNodes.Reserve(numPrimitives);

		for(S32 i = 0; i < numPrimitives; ++i)
		{
			const RenderingData::RenderListNode::Primitive& primitive = m_pRenderListNode->GetPrimitive(i);
			const RenderingData::Bounds& bounds = m_pRenderListNode->GetPrimitiveBounds(i);

			S32 elementIndex = primitive.flags & 0x0000ffff;
			U32 visibilityFlags = ~(primitive.flags >> 16);

			// OR in the primitive gather mask
			primitivesVisibilityFlags &= visibilityFlags;
			visibilityFlags |= visibilityFlags;

			// update max element index
			if (elementIndex > maxElementIndex)
				maxElementIndex = elementIndex;

			C_ModelInstancePrimitive primitiveNode(this, i, elementIndex, visibilityFlags, bounds.distanceMin, bounds.distanceMax);
			m_PrimitiveNodes.PushBack(primitiveNode);
		}

		// check to see if all primitives have the same gather mask
		m_PerPrimitiveGather = (primitivesVisibilityFlags != visibilityFlags);

		// calculate the number of visible elements
		m_VisibleElementFlags.Resize(maxElementIndex + 1);
		m_VisibleElementFlags.ClearBits();
		m_RenderableElementFlags.Resize(maxElementIndex + 1);
		m_RenderableElementFlags.ClearBits();
		for(S32 i = 0; i < numPrimitives; ++i)
		{
			C_ModelInstancePrimitive& primitive = m_PrimitiveNodes[i];

			// get the visible element flags
			if(!(m_VisibleElementFlags.GetBitValue(primitive.GetElementIndex())))
			{
				++m_Elements;
				++m_VisibleElements;
				m_VisibleElementFlags.SetBit(primitive.GetElementIndex());
				m_RenderableElementFlags.SetBit(primitive.GetElementIndex());
			}
		}
	}
	MEMORY_TRACKER_POP("graphics.scenegraph.modelinstance");
}


C_ModelInstance::~C_ModelInstance()
{
	if(m_pMaterialListData)
		delete m_pMaterialListData;
	
	if(m_pCullNode)
		delete m_pCullNode;
}

void C_ModelInstance::SetRenderingDataRenderPassMapping(RenderingData::RenderPass renderPass, const S32 passType)
{
	if(m_Frame == m_pSceneGraph->GetFrame())
	{
		// if this debug breaks here then the game is trying to modify a model instance after the model has been gathered but before the end of the frame
		// this is strictly not allowed
		DebugBreak();
	}
	m_RdToSgPassMapping[renderPass] = passType;
}

void C_ModelInstance::BuildRenderPasses()
{
	MEMORY_TRACKER_PUSH("graphics.scenegraph.modelinstance");
	// clear all render passes
	m_Passes.Clear();
	
	if(m_pCullNode)
		m_pCullNode->DisableRenderPass(0xFFFFFFFF);
	
	U32 passIndex = 0xFFFFFFFF;
	m_VolatileInfoFlags = 0;

	// resize the mapping array from scene graph render pass to RenderingData pass
	m_SgToRdPassMapping.Resize(m_pSceneGraph->GetNumRenderPasses());
	for(S32 i = 0; i < m_SgToRdPassMapping.Size(); ++i)
	{
		m_SgToRdPassMapping[i] = -1;
	}

	// figure out which passes this node is visible in
	for(S32 i = 0; i < m_pRenderListNode->GetRenderPassInfoSize(); ++i)
	{
		const RenderingData::RenderListNode::RenderPassInfo& renderPassInfo = m_pRenderListNode->GetRenderPassInfo((RenderingData::RenderPass)i);
		S32 renderPassType = m_RdToSgPassMapping[i];
		
		// set which rendering data pass this maps to
		m_SgToRdPassMapping[renderPassType] = i;
		
		// if this render pass has any prims or instances then enable the render pass
		if(renderPassInfo.nPrimitives > 0 || renderPassInfo.nInstances > 0 || renderPassInfo.nInstanceBatches > 0)
		{
			const C_RenderPass* pRenderPass = m_pSceneGraph->GetRenderPass(renderPassType);

			m_VolatileInfoFlags |= pRenderPass->GetVolatileInfoFlags();
			
			m_Passes.PushBack(renderPassType);
			passIndex = renderPassType;
			if(m_pCullNode)
				m_pCullNode->EnableRenderPass(Flag(renderPassType));
		}
	}

	// pass index optimisation for when model only exists in 1 render pass
	if(m_pCullNode)
	{
		if(m_Passes.Size() > 1)
			passIndex = 0xFFFFFFFF;

		m_pCullNode->m_VolatileInfoFlags = m_VolatileInfoFlags;
		m_pCullNode->m_PassIndex = passIndex;
		m_pCullNode->SetNearFarDistance(0.0f, 500.0);
		m_pCullNode->SetVisibilityFlags(m_VisibilityFlags);
	}
	MEMORY_TRACKER_POP("graphics.scenegraph.modelinstance");
}

// build render list nodes model instance
void C_ModelInstance::Build()
{
	BuildRenderPasses();
}

// set visibility detection flag. returns true if the modelinstance is already in the 'newly visible' list
void C_ModelInstance::SetVisible(U32 iViewBitfield) const
{
	if (!m_pVisDetect)
		return;

	if (MT::AtomicOr(&m_pVisDetect->flag, iViewBitfield | E_VisDetectFlag_AddedToNewList | E_VisDetectFlag_DefaultVisible) & (E_VisDetectFlag_AddedToNewList | E_VisDetectFlag_AddedToVisibleList) )
		return;

	m_pSceneGraph->AddNewlyVisible(m_pVisDetect);
}

// add to view. other nodes handle culling
void C_ModelInstance::Gather(const C_View& view, const U32 predication, const U32 volatileNodeInfoIndex) const
{
	U32 screenSize = volatileNodeInfoIndex & 0x000000FF;
	for(S32 i = 0; i < m_Passes.Size(); ++i)
		view.AddVisibleNode(this, m_Passes[i], predication, volatileNodeInfoIndex, screenSize);
}

void C_ModelInstance::FlagMovement()
{
	m_pSceneGraph->OnMove(this);
	return;
}

extern Debug::Bool g_BoundsDrawDistance;
extern Debug::Bool g_BoundsNames;
extern Debug::Bool g_BoundsPredication;

void C_ModelInstance::DebugRender(const C_VisibleNode& node) const
{
	if(g_BoundsModelInstances && m_pCullNode)
	{
		Maths::C_Sphere sphere = m_pCullNode->GetSphere();
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dSphere(sphere, Maths::RGBA(255, 255, 0, 128));

		if(g_BoundsDrawDistance)
		{
			Util::C_String drawDistance;
			if(m_pCullNode->GetNearDistance() > 0.0f)
				drawDistance.Printf("%.2f:%.2f", m_pCullNode->GetNearDistance(), m_pCullNode->GetFarDistance());
			else if(m_pCullNode->GetFarDistance() < FLT_MAX)
				drawDistance.Printf("%.2f", m_pCullNode->GetFarDistance());
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(sphere.Centre(), drawDistance);
		}
		if(g_BoundsNames)
		{
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(sphere.Centre(), m_pszFriendlyName);
		}
	}
}

void C_ModelInstance::SetElementVisibility(const S32 index, const bool visibility)
{
	if(index < m_VisibleElementFlags.Size())
	{
		if (m_RenderableElementFlags.GetBitValue(index))
		{
			if(visibility)
			{
				if(!(m_VisibleElementFlags.GetBitValue(index)))
				{
					++m_VisibleElements;
					m_VisibleElementFlags.SetBit(index);
				}
			}
			else
			{
				if(m_VisibleElementFlags.GetBitValue(index))
				{
					--m_VisibleElements;
					m_VisibleElementFlags.ClearBit(index);
				}
			}
		}
	}
}

bool C_ModelInstance::GetElementVisibility(const S32 index) const
{
	return m_VisibleElementFlags.GetBitValue(index);
}

void C_ModelInstance::SetCullNode(I_ModelInstanceCullNode* pCullNode)
{
	m_pCullNode = pCullNode;
}

void C_ModelInstance::SetVisibilityFlags( U32 visibilityFlags )
{
	m_VisibilityFlags = visibilityFlags;
	if (m_pCullNode)
	{
		m_pCullNode->SetVisibilityFlags(m_VisibilityFlags);
	}
}

void C_ModelInstance::EnableVisibilityFlags( U32 visibilityFlags )
{
	m_VisibilityFlags |= visibilityFlags;
	if (m_pCullNode)
	{
		m_pCullNode->SetVisibilityFlags(m_VisibilityFlags);
	}
}

void C_ModelInstance::DisableVisibilityFlags( U32 visibilityFlags )
{
	m_VisibilityFlags &= ~visibilityFlags;
	if (m_pCullNode)
	{
		m_pCullNode->SetVisibilityFlags(m_VisibilityFlags);
	}
}

// set the parent matrix pointer. this is used during rendering to calculate the PVS viewing direction
void C_ModelInstance::SetParentMatrixPtr(const Maths::HMat43 *pMtx)
{
	m_pParentMatrix = pMtx;
}

// C_ModelInstanceBasicCullNode

C_ModelInstanceBasicCullNode::C_ModelInstanceBasicCullNode(const C_ModelInstance* pModelInstanceNode) :
		I_ModelInstanceCullNode(pModelInstanceNode),
		I_GatherNode(-1, NULL)
{
}

C_ModelInstanceBasicCullNode::~C_ModelInstanceBasicCullNode()
{
}

void C_ModelInstanceBasicCullNode::Gather(const C_MultiView& multiView) const
{
	for(S32 i = 0; i < multiView.Size(); ++i)
		Gather(*multiView.GetView(i));
}

void C_ModelInstanceBasicCullNode::Gather(const C_View& view) const
{
	// culling. use data off model instance to get dynamic position
	if(view.RenderPassEnabled(RenderPasses()))
	{
		U32 typeFlag = view.GetTypeFlag();
		if(typeFlag == (typeFlag & GetVisibilityFlags()))
		{
			if(view.RenderNodeEnabled(Flag<E_RenderNodeType_ModelInstance>()))
			{
				Maths::HVec4 centreW(Maths::Load<Maths::HVec4>(&centre));
				Maths::HF32 radius(size.w);

				if(CullSphereDistance(view, view.GetViewData().modelInstanceDrawDistanceScale) || !(view.GetViewData().cullFlags & E_CullMethod_Frustum))
				{
					VolatileNodeInfo volatileNodeInfo;

					if((m_pModelInstance->GetVolatileInfoFlags() & E_VolatileNodeInfo_Distance) || view.GetViewData().distanceSubtechniqueEnable)
					{
						Maths::HF32 farClip(view.GetViewData().farClip);
						const Display::C_Matrix& viewMatrix = view.GetViewSettings3D().GetViewMatrix();
						Maths::HVec4 forward(viewMatrix._13, viewMatrix._23, viewMatrix._33, viewMatrix._43);
						Maths::HF32 depthScale = view.GetViewData().depthScale;;
						
						Maths::HF32 depth = (centreW | forward);
						Maths::HF32 depthScaled = depth * depthScale;
						depth -= radius;

						volatileNodeInfo.distance = (U32)depthScaled;
						volatileNodeInfo.distanceSubtechnique = (view.GetViewData().distanceSubtechniqueEnable && depth > view.GetViewData().distanceSubtechniqueDistance) ? 1 : 0;
					}

					U32 predication = 0;


					m_pModelInstance->Gather(view, predication, volatileNodeInfo.data);
					m_pModelInstance->SetVisible(view.GetTypeFlag());
				}
			}
		}
	}
}

void C_ModelInstanceBasicCullNode::Accept(I_Visitor& visitor) const
{
	visitor.Visit(*this);
}

// Add a dynamic vertex buffer to the model instance.
void C_ModelInstance::AddDynamicVertexBuffer(const RenderingData::Target::VertexBufferData& rkVertexBuffer)
{
	m_DynamicVertexBuffers.PushBack(rkVertexBuffer);
	return;
}

// Add a primitive stream to the model instance.
void C_ModelInstance::AddDynamicPrimitiveStreams(const RenderingData::Target::PrimitiveStream& rkPrimitiveStream)
{
	m_DynamicVertexBufferPrimitiveStreams.PushBack(rkPrimitiveStream);
	return;
}


const Btl::Vector<RenderingData::Target::VertexBufferData>& C_ModelInstance::GetDynamicVertexBuffers() const
{
	return m_DynamicVertexBuffers;
}

const Btl::Vector<RenderingData::Target::PrimitiveStream>& C_ModelInstance::GetDynamicPrimitiveStreams() const
{
	return m_DynamicVertexBufferPrimitiveStreams;
}


} // namespace SceneGraph

