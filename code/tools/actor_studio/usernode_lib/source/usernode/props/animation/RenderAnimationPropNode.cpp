#include "stdafx.h"

#include <Aftk/AftkEnum.h>

// aftk rendering
#include <AftkRender.h>

#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/HermiteSpline.h>
#include <Misc_Shared/NewCurve/MayaHermiteSpline.h>

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Core.h>

#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/ActorSceneNode.h>
#include <EditorApp/Editor/IActorPropertyNode.h>
#include <EditorApp/Editor/ActorScene.h>
#include <EditorApp/Editor/Primitives.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/SimpleRenderer.h>
#include <EditorApp/Editor/ComplexRenderer.h>
#include <EditorApp/Editor/PropertyVisibilityManager.h>
#include <EditorApp/Editor/RenderHelpers.h>
#include <EditorApp/Editor/VertexRenderer.h>
#include <EditorApp/Misc/MathsHelpers.h>


#include <UserNode/Props/Animation/DgAnimationPropNode.h>
#include <UserNode/Props/Animation/RenderAnimationPropNode.h>

#include <Display/Buffer/VertexDeclarations.h>

#include <core/Maths.h>

#include <wx/renderer.h>
#include <EditorApp/Editor/manipulators/contexts/IManipulatorContext.h>
#include <EditorApp/Editor/manipulators/Manipulator.h>
#include <EditorApp/Editor/manipulators/SelectManipulator.h>

void RegisterAnimationPropRenderNodes()
{
	REGISTER_RENDER_NODE(Editor::C_RenderAnimationPropNode)
}

namespace Editor
{

C_RenderAnimationPropNode::C_RenderAnimationPropNode(
	C_ActorScene* pScene, 
	const EditorLib::C_DgXtPropNodeBase* pDgNode)
: I_PropertyNode(pScene, pDgNode)
{
	m_CustomTypeData = E_CustomNodeType_Property;
	m_VertexCount = 0;
	m_pPositionCallback = 0;
	m_pRotationCallback = 0;
	m_Timeline = 0;

	Editor::C_RenderCore::Instance().GetRenderPipeline().GetComplexRenderer().CreateComplexObject(100, &m_pVertexBuffer, this);
}

C_RenderAnimationPropNode::~C_RenderAnimationPropNode()
{
	UnregisterCallbacks();
	Editor::C_RenderCore::Instance().GetRenderPipeline().GetComplexRenderer().DeleteComplexObject(&m_pVertexBuffer);
}

void C_RenderAnimationPropNode::ComplexRendererCallback(ComplexObject* object)
{
	FillVertexBuffer();
}

const char*	C_RenderAnimationPropNode::GetStaticTypeName()
{
	return EditorLib::C_DgAnimationPropNode::GetStaticTypeName();
}

const char* C_RenderAnimationPropNode::GetStaticDisplayName()
{
	static char* p = "Animation Path Property";
	return p;
}

const char*	C_RenderAnimationPropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

EditorLib::E_ObjType C_RenderAnimationPropNode::GetStaticType()
{
	return EditorLib::C_DgAnimationPropNode::GetStaticType();
}

void* C_RenderAnimationPropNode::Create()
{
	return new C_RenderAnimationPropNode(0,0);
}

void C_RenderAnimationPropNode::SimpleRendererRegister()
{
}

void C_RenderAnimationPropNode::AttachToRenderer(
	AftkRender::E_RendererPass pass)
{
	if (pass == AftkRender::E_RendererPass_2D)
		return;

	const C_DgNodeSelection& selection = C_RenderCore::Instance().GetRenderScene().GetSelection();
	const C_DgNodeSelection::C_Item *pItem = selection.Find(GetDgNode());
	const Ut::UtVector< Ut::DynamicVector<U32> >* pComponents = 0;
	if (pItem)
		pComponents = &pItem->components;

	Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>& objects = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects();

	Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>::Iterator object = objects.Begin();
	Btl::Vector<Maths::HMat44>::Iterator localAnimationTransform = m_LocalAnimationTransforms.Begin();
	Btl::Vector<Maths::HMat44>::Iterator worldAnimationTransform = m_WorldAnimationTransforms.Begin();
	Btl::Vector<Btl::String>::Iterator visualisation = m_Visualisations.Begin();
	for (int objectIndex=0; object != objects.End() && localAnimationTransform != m_LocalAnimationTransforms.End() && worldAnimationTransform != m_WorldAnimationTransforms.End(); ++object, ++worldAnimationTransform, ++localAnimationTransform, ++objectIndex, ++visualisation)
	{
		Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>& channels = (*object)->GetChannels();
		float timelineValue = (*object)->GetTimelineValue(GetTimelineValue());

		Maths::RGBA highlight = RenderHelpers::GetHighlightColour(m_pDgNode);
		
		if (Editor::C_PropertyVisibilityManager::Instance().IsVisible((EditorLib::C_DgAnimationPropNode*)m_pDgNode))
		{
			if (C_SelectManipulator::GetSelectionMode() == E_SelectionMode_Component) {
				S32 numKeyFrames = 0;
				
				for (S32 i=0; i<channels.Size(); i++)
				{
					if (channels[i]->metadata == EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Translation || channels[i]->metadata == EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Rotation)
					{
						for (int j=0; j<channels[i]->curves.Size(); j++)
						{
							numKeyFrames += channels[i]->curves[j]->pCurveData->GetNumKnots();
						}
					}
				}

				if (numKeyFrames != 0) {
					float pointSize = 4.0f;
					pointSize = (float)Display::Device()->GetD3DDevice()->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&pointSize));

					Display::Vertex_XYZ_C *pVertex = (Display::Vertex_XYZ_C*) Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().BeginVertices(
					Display::E_PrimitiveType_PointList, 
					numKeyFrames, 
					sizeof(Display::Vertex_XYZ_C), 
					Display::Vertex_XYZ_C::m_pDeclaration, true);

					Display::BasicRender()->BindColoured3D();

					if (pVertex) {
						for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>::Iterator channel = channels.Begin(); channel != channels.End(); ++channel)
						{
							if ((*channel)->metadata != EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Translation && (*channel)->metadata != EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Rotation)
								continue;

							for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator curve = (*channel)->curves.Begin(); curve != (*channel)->curves.End(); ++curve)
							{
								for (S32 i=0; i<(*curve)->pCurveData->GetNumKnots(); i++)
								{
									Maths::RGBA propColour = (*curve)->colour;
									propColour.A(255);

									Curve::C_MayaHermiteSplineKnotU<F32>& keyFrame = (*curve)->pCurveData->GetKnots()[i];

									F32 x, y, z;
									x = y = z = 0;

									for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>::Iterator evalChannel = channels.Begin(); evalChannel != channels.End(); ++evalChannel)
									{
										if ((*evalChannel)->metadata != EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Translation)
											continue;

										for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator evalCurve = (*evalChannel)->curves.Begin(); evalCurve != (*evalChannel)->curves.End(); ++evalCurve)
										{
											if (!(*evalCurve)->pCurveData->GetNumKnots())
												continue;

											if ((*evalCurve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_X)
												x = (*evalCurve)->Evaluate(keyFrame.u);
											else if ((*evalCurve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Y)
												y = (*evalCurve)->Evaluate(keyFrame.u);
											else if ((*evalCurve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Z)
												z = (*evalCurve)->Evaluate(keyFrame.u);
										}
									}

									Maths::HMat44 translateMatrix;
									translateMatrix[3][0] = x;
									translateMatrix[3][1] = y;
									translateMatrix[3][2] = z;
									translateMatrix[3][3] = 1;

									Maths::HMat44 componentMatrix = translateMatrix * m_WorldMatrix;

									pVertex->x = componentMatrix[3][0];
									pVertex->y = componentMatrix[3][1];
									pVertex->z = componentMatrix[3][2];
									pVertex->c = propColour;
									pVertex++;
								}
							}
						}

						Editor::C_RenderCore::Instance().GetRenderPipeline().GetVertexRenderer().EndVertices(pVertex);
					} else {
						Util::C_String debugMessage;
						debugMessage.Printf("Couldn't create point list for component rendering: %d points", numKeyFrames);
						LOGERROR(EditorLib::C_Core::Instance().GetLogger(), debugMessage.Chars());
					}

					Display::Device()->GetD3DDevice()->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&pointSize));
				}
			}

			FillVertexBuffer();

			U32 steps = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetClipLength()*30.0f;
			U32 startOffset = 0;

			for (int i=0; i<((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects().Size(); i++)
			{
				if (steps > 0) {
					Editor::C_RenderCore::Instance().GetRenderPipeline().GetComplexRenderer().DrawInstance(
						m_pVertexBuffer,
						startOffset,
						steps,
						&m_WorldMatrix,
						m_WorldAABBox,
						m_pDgNode->GetId(),
						highlight.Colour() ? &highlight : 0 );
				}
				startOffset += steps;
			}
		}

		Maths::HMat44 xRot;
		Maths::HMat44 yRot;
		Maths::HMat44 zRot;
		Maths::HMat44 translate;

		for (S32 i=0; i<channels.Size(); i++)
		{
			if (channels[i]->metadata == EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Translation)
			{
				for (int j=0; j<channels[i]->curves.Size(); j++)
				{
					if (channels[i]->curves[j]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_X)
						translate[3][0] = channels[i]->curves[j]->Evaluate(timelineValue) + channels[i]->curves[j]->GetOffset();
					else if (channels[i]->curves[j]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Y)
						translate[3][1] = channels[i]->curves[j]->Evaluate(timelineValue) + channels[i]->curves[j]->GetOffset();
					else if (channels[i]->curves[j]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Z)
						translate[3][2] = channels[i]->curves[j]->Evaluate(timelineValue) + channels[i]->curves[j]->GetOffset();
				}
			}
			else if (channels[i]->metadata == EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Rotation)
			{
				for (int j=0; j<channels[i]->curves.Size(); j++)
				{
					if (channels[i]->curves[j]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_X)
						xRot = Maths::HMat44(Maths::C_Quaternion(Maths::HVec3(1,0,0), Maths::DegToRad(channels[i]->curves[j]->Evaluate(timelineValue) + channels[i]->curves[j]->GetOffset())));
					else if (channels[i]->curves[j]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Y)
						yRot = Maths::HMat44(Maths::C_Quaternion(Maths::HVec3(0,1,0), Maths::DegToRad(channels[i]->curves[j]->Evaluate(timelineValue) + channels[i]->curves[j]->GetOffset())));
					else if (channels[i]->curves[j]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Z)
						zRot = Maths::HMat44(Maths::C_Quaternion(Maths::HVec3(0,0,1), Maths::DegToRad(channels[i]->curves[j]->Evaluate(timelineValue) + channels[i]->curves[j]->GetOffset())));
				}
			}
		}

		localAnimationTransform->SetIdentity();
		
		(*localAnimationTransform) = (*localAnimationTransform) * yRot;
		(*localAnimationTransform) = (*localAnimationTransform) * xRot;
		(*localAnimationTransform) = (*localAnimationTransform) * zRot;

		(*localAnimationTransform) = (*localAnimationTransform) * translate;

		(*worldAnimationTransform) = (*localAnimationTransform) * m_WorldMatrix;

		Maths::C_Vertex minimum(-2,-2,-2);
		Maths::C_Vertex maximum(2,2,2);

		Maths::C_AxisBox boundingBox;
		boundingBox.UpdateExtents(minimum*(*worldAnimationTransform));
		boundingBox.UpdateExtents(maximum*(*worldAnimationTransform));

		Maths::RGBA propColour(200,0,200,255);
		if (pComponents) 
		{
			for (S32 i=0; i < pComponents->Size(); ++i)
				if ((*pComponents)[i] == objectIndex)
					propColour = Maths::RGBA(255, 255, 128);
		}

		Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().AddPrimitive(
			EditorLib::C_SceneXtActorNode::GetStaticTypeName(),
			&(*worldAnimationTransform),
			boundingBox,
			m_pDgNode->GetId(),
			objectIndex,
			&propColour);

		Editor::C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().AddPrimitive(
			visualisation->Chars(),
			&(*worldAnimationTransform), 
			boundingBox,
			m_pDgNode->GetId(), 
			objectIndex, 
			&propColour);
	}	
}

void C_RenderAnimationPropNode::FillVertexBuffer()
{
	S32 stepsPerObject = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetClipLength()*30.0f;
	S32 totalSteps = stepsPerObject*((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects().Size();
	if (totalSteps >= 2) {
		if (totalSteps > m_pVertexBuffer->GetVertexBufferSize()) {
			if (!Editor::C_RenderCore::Instance().GetRenderPipeline().GetComplexRenderer().ResizeComplexObject(totalSteps, m_pVertexBuffer))
				return;
		}

		if (!m_pVertexBuffer->GetVertexBuffer()->IsCreated())
			return;

		Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>& objects = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects();

		if (!objects.Size())
			return;

		U32 flags = 0;
		Display::Vertex_XYZ *pVertex = (Display::Vertex_XYZ *)m_pVertexBuffer->GetVertexBuffer()->Lock(0, totalSteps, flags);

		if (pVertex) {
			for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>::Iterator object = objects.Begin(); object != objects.End(); ++object)
			{
				Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>& channels = (*object)->GetChannels();

				U32 quantisation = 30.0f;
				for(S32 i = 0; i < stepsPerObject; ++i)
				{
					F32 timelineValue = (*object)->GetTimelineValue((float)i/(float)quantisation);
					F32 x , y, z;
					x = y = z = 0;

					for (S32 j=0; j<channels.Size(); j++)
					{
						if (channels[j]->metadata != EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Translation)
							continue;

						for (S32 k=0; k<channels[j]->curves.Size(); k++)
						{
							if (!channels[j]->curves[k]->pCurveData->GetNumKnots())
								continue;

							if (channels[j]->curves[k]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_X)
								x = channels[j]->curves[k]->Evaluate(timelineValue);
							else if (channels[j]->curves[k]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Y)
								y = channels[j]->curves[k]->Evaluate(timelineValue);
							else if (channels[j]->curves[k]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Z)
								z = channels[j]->curves[k]->Evaluate(timelineValue);
						}
					}

					Maths::RGBA propColour(200,0,200,255);

					pVertex->x = x;
					pVertex->y = y;
					pVertex->z = z;
					pVertex++;
				}
			}

			m_pVertexBuffer->GetVertexBuffer()->Unlock();
		} else {
			Util::C_String debugMessage;
			debugMessage.Printf("Couldn't create point list for path rendering: %d points", totalSteps);
			LOGERROR(EditorLib::C_Core::Instance().GetLogger(), debugMessage.Chars());
		}
	}
}

void C_RenderAnimationPropNode::UpdateAnimation(const Ut::UtVector< Ut::DynamicVector<U32> >* componentIds, const Maths::HVec3* offset, const float* tension)
{
	Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>& objects = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects();

	m_LocalAnimationTransforms.Resize(objects.Size());
	m_WorldAnimationTransforms.Resize(objects.Size());
	
	UpdateCache();
}

void C_RenderAnimationPropNode::ApplyTranslate(const Ut::UtVector< Ut::DynamicVector<U32> >* componentIds, const Maths::HVec3& offset)
{
	Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>& objects = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects();

	for (int i=0; i < componentIds->Size(); i++)
	{
		for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>::Iterator channel = objects[(*componentIds)[i]]->GetChannels().Begin(); channel != objects[(*componentIds)[i]]->GetChannels().End(); ++channel)
		{
			if ((*channel)->metadata != EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Translation)
				continue;

			for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator curve = (*channel)->GetCurves().Begin(); curve != (*channel)->GetCurves().End(); ++curve)
			{
				if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_X)
					(*curve)->SetOffset((*curve)->GetOffset()+offset.X());
				else if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Y)
					(*curve)->SetOffset((*curve)->GetOffset()+offset.Y());
				else if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Z)
					(*curve)->SetOffset((*curve)->GetOffset()+offset.Z());
			}
		}

		if (m_pPositionCallback)
		{
			(*m_pPositionCallback)(objects[(*componentIds)[i]], GetPosition(objects[(*componentIds)[i]]));
		}
	}
}

void C_RenderAnimationPropNode::ApplyRotate(const Ut::UtVector< Ut::DynamicVector<U32> >* componentIds, const Maths::C_Quaternion& quat)
{
	Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>& objects = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects();

	for (int i=0; i<componentIds->Size(); i++)
	{
		U32 objectId = (*componentIds)[i];

		Maths::HMat33 mat(quat);

		Maths::HVec3 rot;

		if ((F64)mat.AY().X() > 0.998) {
			rot = Maths::HVec3(0, atan2( mat.AX().Z(), mat.AZ().Z()), Maths::PiBy2);
		} else if ((F64)mat.AY().X() < -0.998) {
			rot = Maths::HVec3(0, atan2( mat.AX().Z(), mat.AZ().Z()), -Maths::PiBy2);
		} else {
			rot = Maths::HVec3(atan2(-mat.AY().Z(), mat.AY().Y()), atan2( -mat.AZ().X(), mat.AX().X()), asin(mat.AY().X()));
		}

		rot = Maths::RadToDeg(rot);

		for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>::Iterator channel = objects[(*componentIds)[i]]->GetChannels().Begin(); channel != objects[(*componentIds)[i]]->GetChannels().End(); ++channel)
		{
			if ((*channel)->metadata != EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Rotation)
				continue;

			for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator curve = (*channel)->GetCurves().Begin(); curve != (*channel)->GetCurves().End(); ++curve)
			{
				if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_X)
					(*curve)->SetOffset((*curve)->GetOffset()+rot.X());
				else if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Y)
					(*curve)->SetOffset((*curve)->GetOffset()+rot.Y());
				else if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Z)
					(*curve)->SetOffset((*curve)->GetOffset()+rot.Z());
			}
		}

		if (m_pRotationCallback)
		{
			(*m_pRotationCallback)(objects[(*componentIds)[i]], GetRotation(objects[(*componentIds)[i]]));
		}
	}
}

void C_RenderAnimationPropNode::SetAbsoluteTransform(EditorLib::C_DgAnimationPropNode::AnimationCurve* curve, float value)
{
	int objectIndex;

	Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>& objects = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects();

	for (objectIndex=0; objectIndex<objects.Size(); objectIndex++)
	{
		if (objects[objectIndex] == curve->pChannel->GetObject())
			break;
	}

	curve->SetOffset(value - curve->Evaluate(GetTimelineValue()));
}

void C_RenderAnimationPropNode::OnNodeDeserialised(const EditorLib::I_DgNode* pDgNode)
{
	bool isReference = false;

	Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>& objects = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects();
	for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>::Iterator object = objects.Begin(); object != objects.End(); ++object)
	{
		if (pDgNode == (*object)->GetActorDgNode())
		{
			isReference = true;
			break;
		}
	}

	if (!isReference && (!pDgNode || pDgNode && pDgNode != m_pDgNode))
		return;

	__super::OnNodeDeserialised(pDgNode);

	UpdateAnimation();

	// HACK: Update visualisation def in deserialise and resolve

	UpdateVisualisationDef();
}

void C_RenderAnimationPropNode::OnNodeResolved(const EditorLib::I_DgNode* pDgNode)
{
	// HACK: Update visualisation def in deserialise and resolve

	bool isReference = false;

	Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>& objects = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects();
	for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>::Iterator object = objects.Begin(); object != objects.End(); ++object)
	{
		if (pDgNode == (*object)->GetActorDgNode())
		{
			isReference = true;
			break;
		}
	}

	if (isReference)
		UpdateVisualisationDef();
}

void C_RenderAnimationPropNode::UpdateVisualisationDef()
{
	m_Visualisations.Clear();

	Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>& objects = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects();
	for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>::Iterator object = objects.Begin(); object != objects.End(); ++object)
	{
		C_ActorSceneNode* pActorNode = (C_ActorSceneNode*)
			C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode((*object)->GetActorDgNode()); 
	
		if (pActorNode)
		{
			const char* primitiveTypeName = pActorNode->GetPrimitiveTypeName();
			if (primitiveTypeName && strcmp(primitiveTypeName, "") != 0)
				m_Visualisations.PushBack(Btl::String(primitiveTypeName));
			else
				m_Visualisations.PushBack(Btl::String("BoundingBox"));
		}
		else
			m_Visualisations.PushBack(Btl::String("BoundingBox"));
	}
}

void C_RenderAnimationPropNode::UpdateCache() const
{
	AftkRender::C_SceneGraphNode::UpdateCache();

	Maths::C_AxisBox localAABBox;
	localAABBox.SetInvalidExtents();
	m_WorldAABBox.SetInvalidExtents();

	S32 steps = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetClipLength()*30.0f;
	if (steps >= 2) {

		for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>::Iterator object = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects().Begin(); object != ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects().End(); ++object)
		{
			Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>& channels = (*object)->GetChannels();

			F32 x, y, z;

			U32 quantisation = 30.0f;
			for(S32 i = 0; i < steps; i++)
			{
				F32 timelineValue = (*object)->GetTimelineValue((float)i/(float)quantisation);
				x = y = z = 0;

				for (int j=0; j<channels.Size(); j++)
				{
					if (channels[j]->metadata != EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Translation)
						continue;

					for (S32 k=0; k<channels[j]->curves.Size(); k++)
					{
						if (!channels[j]->curves[k]->pCurveData->GetNumKnots())
							continue;

						if (channels[j]->curves[k]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_X)
							x = channels[j]->curves[k]->Evaluate(timelineValue);
						else if (channels[j]->curves[k]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Y)
							y = channels[j]->curves[k]->Evaluate(timelineValue);
						else if (channels[j]->curves[k]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Z)
							z = channels[j]->curves[k]->Evaluate(timelineValue);		
					}
				}

				localAABBox.UpdateExtents(Maths::HVec3(x,y,z));
			}

			m_WorldAABBox.UpdateExtents(localAABBox.Min() * m_WorldMatrix);
			m_WorldAABBox.UpdateExtents(localAABBox.Max() * m_WorldMatrix);
		}
	}
}

Maths::C_AxisBox C_RenderAnimationPropNode::GetSelectionAABBox() const
{
	return m_WorldAABBox;
}

Maths::HMat44& C_RenderAnimationPropNode::GetLocalTransform(int object)
{
	return m_LocalAnimationTransforms[object];
}

void C_RenderAnimationPropNode::SetTimelineValue(double timeline)
{
	C_RenderCore::Instance().GetManipulatorManager().Dirty();

	Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>& objects = ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects();

	for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>::Iterator object=((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects().Begin(); object != ((EditorLib::C_DgAnimationPropNode*)m_pDgNode)->GetObjects().End(); ++object)
	{
		for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>::Iterator channel=(*object)->GetChannels().Begin(); channel != (*object)->GetChannels().End(); ++channel)
		{
			for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator curve=(*channel)->GetCurves().Begin(); curve != (*channel)->GetCurves().End(); ++curve)
			{
				(*curve)->SetOffset(0);
			}
		}
	}

	m_Timeline = timeline;
}

double C_RenderAnimationPropNode::GetTimelineValue()
{
	return m_Timeline;
}

Maths::HVec3 C_RenderAnimationPropNode::GetPosition(EditorLib::C_DgAnimationPropNode::Object* object)
{
	Maths::HVec3 position;

	for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>::Iterator channel = object->GetChannels().Begin(); channel != object->GetChannels().End(); ++channel)
	{
		if ((*channel)->metadata != EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Translation)
			continue;

		for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator curve = (*channel)->GetCurves().Begin(); curve != (*channel)->GetCurves().End(); ++curve)
		{
			if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_X)
			{
				if ((*curve)->pCurveData->GetNumKnots())
					position.X((*curve)->Evaluate(GetTimelineValue()) + (*curve)->GetOffset());
				else
					position.X((*curve)->GetOffset());
			}
			else if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Y)
			{
				if ((*curve)->pCurveData->GetNumKnots())
					position.Y((*curve)->Evaluate(GetTimelineValue()) + (*curve)->GetOffset());
				else
					position.Y((*curve)->GetOffset());
			}
			else if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Z)
			{
				if ((*curve)->pCurveData->GetNumKnots())
					position.Z((*curve)->Evaluate(GetTimelineValue()) + (*curve)->GetOffset());
				else
					position.Z((*curve)->GetOffset());
			}
		}
	}
	
	return position;
}

Maths::HVec3 C_RenderAnimationPropNode::GetRotation(EditorLib::C_DgAnimationPropNode::Object* object)
{
	Maths::HVec3 rotation;

	for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>::Iterator channel = object->GetChannels().Begin(); channel != object->GetChannels().End(); ++channel)
	{
		if ((*channel)->metadata != EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Rotation)
			continue;

		for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator curve = (*channel)->GetCurves().Begin(); curve != (*channel)->GetCurves().End(); ++curve)
		{
			if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_X)
			{
				if ((*curve)->pCurveData->GetNumKnots())
					rotation.X((*curve)->Evaluate(GetTimelineValue()) + (*curve)->GetOffset());
				else
					rotation.X((*curve)->GetOffset());
			}
			else if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Y)
			{
				if ((*curve)->pCurveData->GetNumKnots())
					rotation.Y((*curve)->Evaluate(GetTimelineValue()) + (*curve)->GetOffset());
				else
					rotation.Y((*curve)->GetOffset());
			}
			else if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Z)
			{
				if ((*curve)->pCurveData->GetNumKnots())
					rotation.Z((*curve)->Evaluate(GetTimelineValue()) + (*curve)->GetOffset());
				else
					rotation.Z((*curve)->GetOffset());
			}
		}
	}
	
	return rotation;
}

void C_RenderAnimationPropNode::RegisterPositionCallback(const Ut::Delegate<void(EditorLib::C_DgAnimationPropNode::Object*, const Maths::HVec3&)>& pHandler)
{
	if (m_pPositionCallback)
		SAFE_DELETE_PTR(m_pPositionCallback);

	m_pPositionCallback = new Ut::Delegate<void(EditorLib::C_DgAnimationPropNode::Object*, const Maths::HVec3&)>(pHandler);
}

void C_RenderAnimationPropNode::RegisterRotationCallback(const Ut::Delegate<void(EditorLib::C_DgAnimationPropNode::Object*, const Maths::HVec3&)>& pHandler)
{
	if (m_pRotationCallback)
		SAFE_DELETE_PTR(m_pRotationCallback);

	m_pRotationCallback = new Ut::Delegate<void(EditorLib::C_DgAnimationPropNode::Object*, const Maths::HVec3&)>(pHandler);
}

void C_RenderAnimationPropNode::UnregisterCallbacks()
{
	SAFE_DELETE_PTR(m_pPositionCallback);
	SAFE_DELETE_PTR(m_pRotationCallback);
}

}
