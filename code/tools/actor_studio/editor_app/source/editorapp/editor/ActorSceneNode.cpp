#include "stdafx.h"

#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Dg/XtNodeHelpers.h>

#include <EditorLib/Level/Layer.h>
#include <EditorLib/Level/LayerManager.h>
#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Xt/XtVisualisation.h>
#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorApp/Editor/ActorSceneNode.h>
#include <EditorApp/Editor/IActorPropertyNode.h>
#include <EditorApp/Editor/ActorScene.h>
#include <EditorApp/Editor/Primitives.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/SimpleRenderer.h>
#include <EditorApp/Editor/SpriteRenderer.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/RenderHelpers.h>
#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Misc/MathsHelpers.h>

using namespace Editor;


C_DgNodeBase::C_DgNodeBase(
	C_ActorScene *pScene,
	const EditorLib::I_DgNode* pDgNode)
: 
AftkRender::C_SceneGraphNode(
	pScene, 
	"")
,
m_pDgNode(pDgNode)
{
	if (pDgNode)
		SetName(pDgNode->GetName().Chars());
}

C_DgNodeBase::~C_DgNodeBase()
{
}

void C_DgNodeBase::SetDgNode(const EditorLib::I_DgNode* pDgNode)
{
	m_pDgNode = pDgNode;
}

const EditorLib::I_DgNode* C_DgNodeBase::GetDgNode() const
{
	return m_pDgNode;
}

bool C_DgNodeBase::GetVisible(const RECT& region, C_DgNodeSelection* pResult)
{
	return false;
}

void C_DgNodeBase::AttachToRenderer(AftkRender::E_RendererPass pass)
{
}

void C_DgNodeBase::OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode)
{
	if (pDgNode && pDgNode!=m_pDgNode)
		return;

	SetName(m_pDgNode->GetName().Chars());
}

void C_DgNodeBase::OnNodeResolved(const EditorLib::I_DgNode* pDgNode)
{
}

Maths::C_AxisBox C_DgNodeBase::GetSelectionAABBox() const
{
	return m_WorldAABBox;
}

C_ActorSceneNode::C_ActorSceneNode(
   C_ActorScene *pScene, 
   const EditorLib::C_SceneXtNodeBase* pDgNode)
: 
C_DgNodeBase(pScene, (EditorLib::C_SceneXtNodeBase*)pDgNode)
{
	m_CustomTypeData = E_CustomNodeType_Actor;

	const EditorLib::C_SceneXtNodeBase *pActor = (const EditorLib::C_SceneXtNodeBase*)m_pDgNode;
	m_LocalMatrix = ConvertToHMat44(pActor->GetLocalMatrix());

	RefreshProperties();
	m_CacheFlags.dirty = true;
}

C_ActorSceneNode::~C_ActorSceneNode()
{
}

void C_ActorSceneNode::OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode)
{
	m_CacheFlags.dirty = true;
	if (!pDgNode)
		return;

	if (pDgNode->IsA(EditorLib::E_ObjType_DgXtPropNodeBase))
	{
		EditorLib::C_DgXtPropNodeBase* pNode = (EditorLib::C_DgXtPropNodeBase*)pDgNode;
		if (!pNode->GetParent() || pNode->GetParent()->GetParent() != m_pDgNode)
			return;
	}
	else if (pDgNode != m_pDgNode)
		return;

	RefreshProperties();

	EditorLib::C_SceneNode* pSceneNode = (EditorLib::C_SceneNode*)m_pDgNode;

	//Set Local Matrix
	Matrix44 newMatrix = pSceneNode->GetLocalMatrix();
	Maths::HMat44 localMatrix;
	Aftk::TemplateToNightmareMatrix44(localMatrix, newMatrix);
	SetLocalMatrix(localMatrix);

	//Set Visibility State
	EditorLib::C_SceneXtNodeBase* pSceneXtNodeBase = pSceneNode->IsA(EditorLib::E_ObjType_SceneXtNodeBase) ? 
		(EditorLib::C_SceneXtNodeBase*)pSceneNode : 0;
	if (pSceneXtNodeBase)
	{
		Xt::DomComponent* pComponent = Xt::AsComponent(pSceneXtNodeBase->GetXtData());
		assert(pComponent);

		XtString visible = Xt::AsText(Xt::At(&pComponent->data, "__visible"));
		AftkRender::C_SceneGraphNode::Flags flags = GetFlags();

		flags.hidden = (visible == "" || visible == "true") ? flags.hidden & ~0x1 : flags.hidden | 0x1;
		SetFlags(flags);
	}

	// HACK : Update cache if primitive type name hasn't yet been set
	if (!m_pPrimitiveTypeName)
		UpdateCache();
}

void C_ActorSceneNode::UpdateCache() const
{
	AftkRender::C_SceneGraphNode::UpdateCache();

	//update bounding box using extents of transformed unit cube
	m_WorldAABBox.SetInvalidExtents();

	m_pDef = EditorLib::C_Core::Instance().GetXtVisualisationData().GetXtVisualisationDef(m_pDgNode);
	m_pPrimitiveTypeName = m_pDef ? m_pDef->id.Chars() : 0;

	if (m_pDef)
	{
		EditorLib::C_AftkRef* ref = EditorLib::C_Core::Instance().GetAftkRefManager().GetRef(m_pDef->filePath);
		if (ref && !ref->IsLoaded())
			ref->Load();
	}

	Btl::String typeName;
	if(!m_pPrimitiveTypeName || 
		!strcmp(m_pPrimitiveTypeName, "") || 
		!Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().PrimitiveExists(m_pPrimitiveTypeName))
	{
		typeName = EditorLib::C_SceneXtActorNode::GetStaticTypeName();
	} 
	else
	{
		typeName = m_pPrimitiveTypeName;
	}

		
	const Maths::C_AxisBox *pBox = Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().GetPrimitiveBounds(typeName.Chars());
	if (pBox)
	{
		Maths::HVec3 p1 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Min[0],pBox->m_Min[1],pBox->m_Min[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p2 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Min[0],pBox->m_Min[1],pBox->m_Max[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p3 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Min[0],pBox->m_Max[1],pBox->m_Min[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p4 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Min[0],pBox->m_Max[1],pBox->m_Max[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p5 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Max[0],pBox->m_Min[1],pBox->m_Min[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p6 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Max[0],pBox->m_Min[1],pBox->m_Max[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p7 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Max[0],pBox->m_Max[1],pBox->m_Min[2],1.f) * m_WorldMatrix);
		Maths::HVec3 p8 = (const Maths::HVec3&)(Maths::HVec4(pBox->m_Max[0],pBox->m_Max[1],pBox->m_Max[2],1.f) * m_WorldMatrix);
	
		m_WorldAABBox.UpdateExtents(p1);
		m_WorldAABBox.UpdateExtents(p2);
		m_WorldAABBox.UpdateExtents(p3);
		m_WorldAABBox.UpdateExtents(p4);
		m_WorldAABBox.UpdateExtents(p5);
		m_WorldAABBox.UpdateExtents(p6);
		m_WorldAABBox.UpdateExtents(p7);
		m_WorldAABBox.UpdateExtents(p8);
	}
	m_CacheFlags.dirty = false;
}

const char* C_ActorSceneNode::GetPrimitiveTypeName()
{
	return m_pPrimitiveTypeName;
}

void C_ActorSceneNode::AttachToRenderer(AftkRender::E_RendererPass pass)
{
	Maths::RGBA overrideColour(0,0,0,0);
	bool useDefaultColour = true;
	const EditorLib::I_Layer* pLayer = 0;

	if (m_CacheFlags.dirty)
		UpdateCache();

	pLayer = EditorLib::C_Core::Instance().GetLayerManager().GetLayer(m_pDgNode);

	EditorLib::I_Level* pLevel = EditorLib::C_Core::Instance().GetDgManager().LevelOf(GetDgNode()->GetId());

	bool actorHidden = (m_Flags.hidden > 0) ||
						(pLevel->IsHidden()) ||
						(pLayer && (!pLayer->GetLayerFlags().visible || (pass==AftkRender::E_RendererPass_VisibilityTest && !pLayer->GetLayerFlags().selectable)));

	if (!actorHidden)
	{
		if ((pass == AftkRender::E_RendererPass_Normal || pass == AftkRender::E_RendererPass_VisibilityTest) && 
			!(m_pDef && m_pDef->visualisationType==EditorLib::XtVisualisationUnknown))
		{
			//Is the node currently selected?
			{
				overrideColour = RenderHelpers::GetHighlightColour(m_pDgNode, overrideColour);
				if (overrideColour.Colour())
					useDefaultColour = false;
			}

			//Try to use layer display colour
			if (useDefaultColour && pLayer)
			{
				overrideColour = pLayer->GetDisplayColour();
				if (overrideColour.A())
					useDefaultColour = false;
			}

			if(C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState().spriteVisible)
			{
				if (m_pDef && m_pDef->visualisationType == EditorLib::XtVisualisationSprite) {
					if (pLevel->IsReadOnly())
						Editor::C_RenderCore::Instance().GetRenderPipeline().GetSpriteRenderer().Draw(m_pDef->filePath.Chars(), E_SpriteEffect_Greyscale, m_pDef->scale, &m_WorldMatrix, m_pDgNode->GetId(), -1);
					else
						Editor::C_RenderCore::Instance().GetRenderPipeline().GetSpriteRenderer().Draw(m_pDef->filePath.Chars(), E_SpriteEffect_None, m_pDef->scale, &m_WorldMatrix, m_pDgNode->GetId(), -1);
				}
			}

			if(C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState().locatorVisible)
			{
				C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().AddPrimitive(
					EditorLib::C_SceneXtActorNode::GetStaticTypeName(),
					&m_WorldMatrix,
					m_WorldAABBox,
					m_pDgNode->GetId(), -1,
					(useDefaultColour) ? 0 : &overrideColour);
			}

			if(C_RenderCore::Instance().GetRenderPipeline().GetGlobalRenderState().meshVisible && m_pPrimitiveTypeName && strcmp(m_pPrimitiveTypeName, ""))
			{
				C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().AddPrimitive(
					m_pPrimitiveTypeName,
					&m_WorldMatrix,
					m_WorldAABBox,
					m_pDgNode->GetId(), -1,
					(useDefaultColour) ? 0 : &overrideColour);
			}
		}

		AftkRender::C_SceneGraphNode *pChild = m_pChild;
		while (pChild)
		{
			pChild->AttachToRenderer(0, pass);
			pChild = pChild->GetNextSiblingPtr();
		}
	}
}

S32 FindChild(AftkRender::C_SceneGraphNode* pRenderNode, const EditorLib::I_DgNode* pDgNode)
{
	AftkRender::C_SceneGraphNode* pChild = pRenderNode->GetChildPtr();
	for(S32 idx = 0; pChild; ++idx)
	{
		if (!pChild->IsA(AftkRender::E_SceneGraphNode_Custom))
			continue;

		if (pChild->GetCustomTypeData() != E_CustomNodeType_Property)
			continue;

		I_PropertyNode* pRenderPropNode = (I_PropertyNode*)pChild;
		if (pRenderPropNode->GetDgNode() == pDgNode)
			return idx;

		pChild = pChild->GetNextSiblingPtr();
	}

	return -1;
}

void C_ActorSceneNode::RefreshProperties()
{
	const EditorLib::C_SceneXtNodeBase *pActor = (const EditorLib::C_SceneXtNodeBase*)m_pDgNode;

	//Iterate components.
	const EditorLib::ConstDgActorComponentArray& components = pActor->GetComponents();
	for (EditorLib::ConstDgActorComponentArray::ConstIterator compIt=components.Begin();
		compIt!=components.End();
		++compIt)
	{
		//Iterate component properties.
		const EditorLib::DgPropArray& properties = (*compIt)->GetProperties();
		for (S32 i = 0; i < properties.Size(); ++i)
		{
			const EditorLib::C_DgXtPropNodeBase* pProp = properties[i].RawPtr();
			if (!pProp)
				//Skip null props.
				continue;

			S32 idx = FindChild(this, pProp);
			if (idx != -1)
				//If property already present continue.
				continue;

			//If this prop is of a visible type, create a render node.
			if (pProp->IsVisible())
			{
				//Use object factory to create a new render property node.
				I_PropertyNode* pNode = (I_PropertyNode*)
					C_RenderCore::Instance().GetRenderScene().GetDgNodeFactory().Create(pProp->GetTypeName());
				//Associate with scene and EditorLib node. 
				pNode->SetScene(m_pScene);
				pNode->SetDgNode((EditorLib::I_DgNode*)pProp->RawPtr());
				AddChild(pNode);
				m_CacheFlags.dirty = true;
			}
		}
	}
}

C_GroupSceneNode::C_GroupSceneNode(C_ActorScene *pScene, const EditorLib::C_SceneXtNode* pDgNode)
: C_DgNodeBase(pScene, (EditorLib::C_SceneXtNodeBase*)pDgNode)
{
	m_CustomTypeData = E_CustomNodeType_Actor;

	const EditorLib::C_SceneXtNodeBase *pActor = (const EditorLib::C_SceneXtNodeBase*)m_pDgNode;
	m_LocalMatrix = ConvertToHMat44(pActor->GetLocalMatrix());
}

C_GroupSceneNode::~C_GroupSceneNode()
{
}

void C_GroupSceneNode::OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode)
{
	if (pDgNode && pDgNode != m_pDgNode)
		return;

	Matrix44 newMatrix = ((EditorLib::C_SceneNode*)m_pDgNode)->GetLocalMatrix();
	Maths::HMat44 localMatrix;
	Aftk::TemplateToNightmareMatrix44(localMatrix, newMatrix);
	SetLocalMatrix(localMatrix);
}

void C_GroupSceneNode::UpdateCache() const
{
	AftkRender::C_SceneGraphNode::UpdateCache();
	m_WorldAABBox.SetInvalidExtents();

	if (m_pChild)
	{
		AftkRender::C_SceneGraphNodeConstIterator it(m_pChild);
		while ((AftkRender::C_SceneGraphNode*)it)
		{
			m_WorldAABBox.UpdateExtents(it->GetWorldAABBox());
			++it;
		}
	}
	else
	{
		Maths::C_Sphere sphere(m_WorldMatrix[3][0], m_WorldMatrix[3][1], m_WorldMatrix[3][2], 1.f);
		m_WorldAABBox = Maths::C_AxisBox(sphere);
	}
}

void C_GroupSceneNode::AttachToRenderer(AftkRender::E_RendererPass pass)
{
	if (pass == AftkRender::E_RendererPass_2D)
		return;

	const EditorLib::I_Layer* pLayer = EditorLib::C_Core::Instance().GetLayerManager().GetLayer(m_pDgNode);

	bool actorHidden = (m_Flags.hidden > 0) ||
						(pLayer && (!pLayer->GetLayerFlags().visible || (pass==AftkRender::E_RendererPass_VisibilityTest && !pLayer->GetLayerFlags().selectable)));

	if (!actorHidden)
	{
		AftkRender::C_SceneGraphNode *pChild = m_pChild;
		while (pChild)
		{
			pChild->AttachToRenderer(pass);
			pChild = pChild->GetNextSiblingPtr();
		}
	}
}
