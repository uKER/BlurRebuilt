#include "stdafx.h"
#include <algorithm>

#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/HermiteSpline.h>
#include <Misc_Shared/NewCurve/MayaHermiteSpline.h>

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Misc/XtScratchData.h>

#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtDirty.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Xt/XtSemantic.h>

#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Collision/Collision.h>
#include <EditorLib/Core.h>

#include <UserNode/Props/Box/DgBoxPropNode.h>
#include <UserNode/Props/Misc/DgPropContextHelpers.h>
#include <UserNode/Props/Animation/DgAnimationPropNode.h>

#include <EditorLib/Level/XtLevel.h>

void RegisterAnimationPropDgNodes()
{
	//Register back-end nodes.
	REGISTER_DG_NODE(EditorLib::C_DgAnimationPropNode)
}

namespace EditorLib
{

//From I_DgNode
bool C_DgAnimationPropNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_DgAnimationPropNode::GetStaticType(), pHc);

	Xt::DomElement* pData = GetXtData();
	if (!pData) 
		return false;

	Xt::DomComponent* pAnimation(0);
	if (pData->kind == Xt::kDomRef)
	{
		Xt::DomRef* pRef = Xt::AsRef(pData);
		XtId id = pRef->value;
		C_NodeId nodeId(id, kNoChildId);
		EditorLib::I_DgNode* pSource = C_Core::Instance().GetDgManager().FindNodeById(nodeId);
		if (pSource && pSource->AsXtObject())
		{
			Xt::DomElement* pSourceData = pSource->AsXtObject()->GetXtData();
			if (m_RefTargetMonitor.element != pSourceData)
			{
				//Reattach monitor.
				Xt::Monitor(&m_RefTargetMonitor, pSourceData);
			}
		}
		else
		{
			Xt::Monitor(&m_RefTargetMonitor, 0);
		}
		pAnimation = Xt::AsComponent(m_RefTargetMonitor.element);
	}
	else
		pAnimation = Xt::AsComponent(pData);

	if (pAnimation)
	{
		Xt::Monitor(&m_RefTargetMonitor, pAnimation);
		m_RefTargetMonitor.SetCallback(C_DgAnimationPropNode::StaticOnXtDomExternalMonitorEvent, (EditorLib::C_XtObject*)this);

		bool success = true;

		// Mark all objects as deleted, loop through Xt data, and unmark ones that still exist

		Xt::DomArray* pObjectArray = pAnimation ? Xt::AsArray(Xt::At(&pAnimation->data, "Objects")) : 0;

		for (int objectLoop=0; objectLoop<m_Objects.Size(); objectLoop++)
		{
			m_Objects[objectLoop]->Delete(true);
		} 

		for (size_t objectLoop=0; objectLoop<Xt::Count(pObjectArray); objectLoop++)
		{
			Xt::DomComponent* pObject = Xt::AsComponent(Xt::At(pObjectArray, objectLoop));
			
			Object* object = 0;

			for (Btl::Vector<Object*>::Iterator objectIt = m_Objects.Begin(); objectIt != m_Objects.End(); objectIt++)
			{
				if (pObject->id == (*objectIt)->id)
				{
					object = *objectIt;
					break;
				}
			}

			if (!object)
			{
				object = new Object(this);
				m_Objects.PushBack(object);
			}

			object->id = pObject->id;

			object->Delete(false);

			object->XtDeserialise(pObject);
		}

		for (int objectLoop=0; objectLoop<m_Objects.Size(); objectLoop++)
		{
			if (m_Objects[objectLoop]->IsDeleted())
			{
				delete m_Objects[objectLoop];
				m_Objects.EraseByIndex(objectLoop);
				objectLoop--;
			}
		}

		// Deserialise global clip data
		if (pAnimation)
		{
			Xt::DomField* pField;

			pField = Xt::AsField(Xt::At(&pAnimation->data, "Name"));
			if (pField)
				m_Name = pField->value.Text();

			pField = Xt::AsField(Xt::At(&pAnimation->data, "Quantisation"));
			if (pField && pField->value != "")
				XtFieldHelper::GetScalar(m_Quantisation, pField);

			pField = Xt::AsField(Xt::At(&pAnimation->data, "ClipLength"));
			XtFieldHelper::GetScalar(m_ClipLength, pField);
		}
	}

	return __super::Update(&signaller.GetCollection());
}

bool C_DgAnimationPropNode::IsNonXtDirty() const
{
	return __super::IsNonXtDirty();
}



const char* C_DgAnimationPropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

const char* C_DgAnimationPropNode::GetStaticTypeName()
{
	//Must match name in Xt schema exactly!
	static char* p = "AnimationClip";
	return p;
}

const char* C_DgAnimationPropNode::GetStaticDisplayName()
{
	static char* p = "Animation Path";
	return p;
}

E_ObjType C_DgAnimationPropNode::GetStaticType()
{
	return E_ObjType_DgAnimationPropNode;
}

void* C_DgAnimationPropNode::Create()
{
	return new C_DgAnimationPropNode();
}

C_DgAnimationPropNode::C_DgAnimationPropNode()
:
C_DgXtPropNodeBase()
{
	//m_pCurrentPosition = new C_XtScratchData();
	
	SetDefault();
	m_RefTargetMonitor.SetCallback(C_DgAnimationPropNode::StaticOnXtDomExternalMonitorEvent, (EditorLib::C_XtObject*)this);
}

C_DgAnimationPropNode::~C_DgAnimationPropNode()
{
	for (Btl::Vector<Object*>::Iterator it=m_Objects.Begin(); it != m_Objects.End(); ++it)
	{
		SAFE_DELETE_PTR(*it);
	}
	m_Objects.Clear();
	Xt::Monitor(&m_RefTargetMonitor, 0);
}

E_ObjType C_DgAnimationPropNode::GetType() const
{
	return E_ObjType_DgAnimationPropNode;
}

bool C_DgAnimationPropNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_DgAnimationPropNode::GetType()) 
		return true;

	return false;
}

bool C_DgAnimationPropNode::IsVisible() const
{
	return true;
}

C_Status C_DgAnimationPropNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	C_Status status = __super::XtInitialise(pLevel, pLarvaType, pData);
	if (!status) return status;

	return E_Status_Success;
}

C_DgAnimationPropNode::AnimationCurve::AnimationCurve(const C_DgAnimationPropNode::AnimationCurve& curve)
{
	m_pPropNode = curve.m_pPropNode;

	metadata = curve.metadata;

	name = curve.name;
	colour = curve.colour;
	pField = curve.pField;

	pChannel = curve.pChannel;

	Curve::C_MayaHermiteSplineFlags flags;
	flags.calcKnotU = Curve::E_ProcessU_None;
	flags.calcTangents = false;

	pCurveData = new CurveDataType();
	pCurveData->GetKnots() = curve.pCurveData->GetKnots();
	pCurve = new CurveType(flags);

	pCurve->SetData(pCurveData);
}

C_DgAnimationPropNode::AnimationCurve::AnimationCurve(C_DgAnimationPropNode* propNode, C_DgAnimationPropNode::Channel* channel )
{
	m_pPropNode = propNode;

	Curve::C_MayaHermiteSplineFlags flags;
	flags.calcKnotU = Curve::E_ProcessU_None;
	flags.calcTangents = false;

	pChannel = channel;

	pCurveData = new CurveDataType();
	pCurve = new CurveType(flags);

	pCurve->SetData(pCurveData);

	m_offset = 0;
	m_dirty = true;
}

C_DgAnimationPropNode::AnimationCurve::~AnimationCurve()
{
	delete pCurveData;
	delete pCurve;
}

void C_DgAnimationPropNode::AnimationCurve::Save()
{
	m_dirty = true;
	m_pPropNode->XtSave();
}

float C_DgAnimationPropNode::AnimationCurve::Evaluate(float t)
{
	if (pCurveData->GetNumKnots())
		return pCurve->Evaluate(t);

	return 0.0f;
}

float C_DgAnimationPropNode::AnimationCurve::GetOffset()
{
	return m_offset;
}

void C_DgAnimationPropNode::AnimationCurve::SetOffset(float value)
{
	m_offset = value;
}

void C_DgAnimationPropNode::AnimationCurve::Process()
{
	pCurve->Process();
}

bool ChannelFrameSort(const Curve::C_MayaHermiteSplineKnotU<F32>& a, const Curve::C_MayaHermiteSplineKnotU<F32>& b)
{
	return a.u < b.u;
}

C_Status C_DgAnimationPropNode::AnimationCurve::AddKeyFrame(F32 position, F32 value, F32 tangentIn, F32 tangentOut, U32* newKeyIndex, bool* isNewKey)
{
	const float keyframeTolerance = 0.00001f;

	Curve::C_MayaHermiteSplineKnotU<F32> p;
	p.u = position;
	p.value = value;
	p.tangentIn = tangentIn;
	p.tangentOut = tangentOut;
	
	Btl::Vector<Curve::C_MayaHermiteSplineKnotU<F32>>::Iterator it;
	
	for (it = pCurveData->GetKnots().Begin(); it != pCurveData->GetKnots().End(); ++it)
	{
		if (it->u >= position - keyframeTolerance && it->u <= position + keyframeTolerance)
			break;
	}

	if (it == pCurveData->GetKnots().End()) {
		if (isNewKey)
			*isNewKey = true;
		pCurveData->GetKnots().PushBack(p);
		std::sort(pCurveData->GetKnots().Begin(), pCurveData->GetKnots().End(), ChannelFrameSort);
	} else {
		if (isNewKey)
			*isNewKey = false;
		*it = p;
	}

	if (newKeyIndex)
	{
		int i = 0;
		for (it = pCurveData->GetKnots().Begin(); it != pCurveData->GetKnots().End(); ++it, ++i)
		{
			if (it->u == p.u)
			{
				*newKeyIndex = i;
				break;
			}
		}
	}

	Process();

	m_dirty = true;
	return m_pPropNode->XtSave();
}

C_Status C_DgAnimationPropNode::AnimationCurve::RemoveKeyFrames(F32 startValue, F32 endValue)
{
	bool keyframesDeleted = false;

	for (int i=0; i<pCurveData->GetNumKnots(); i++)
	{
		F32 u = pCurveData->GetKnots()[i].u;
		if (u >= startValue && u <= endValue)
			InternalRemoveKeyFrame(i);
	}
	
	if (keyframesDeleted)
	{
		Process();
		m_dirty = true;
		return m_pPropNode->XtSave();
	}

	return E_Status_Success;
}

C_Status C_DgAnimationPropNode::AnimationCurve::RemoveKeyFrame(U32 index)
{
	InternalRemoveKeyFrame(index);

	Process();

	m_dirty = true;
	return m_pPropNode->XtSave();
}

C_Status C_DgAnimationPropNode::AnimationCurve::SmoothTangents(U32 index)
{
	if (index < 1 || index >= (U32)pCurveData->GetKnots().Size()-1)
		return E_Status_Success;

	Curve::C_MayaHermiteSplineKnotU<F32>& knot1 = pCurveData->GetKnots()[index-1];
	Curve::C_MayaHermiteSplineKnotU<F32>& knot2 = pCurveData->GetKnots()[index];
	Curve::C_MayaHermiteSplineKnotU<F32>& knot3 = pCurveData->GetKnots()[index+1];

	knot2.tangentIn = Maths::Tan(atan2(knot1.value-knot3.value, knot1.u-knot3.u));
	knot2.tangentOut = knot2.tangentIn;

	Process();

	m_dirty = true;
	return m_pPropNode->XtSave();
}

C_Status C_DgAnimationPropNode::AnimationCurve::EulerFilter()
{
	if (pChannel->metadata != Channel::E_AnimationChannel_Metadata_Rotation || pCurveData->GetNumKnots()<2)
		return EditorLib::E_Status_Success;

	Btl::Vector<Curve::C_MayaHermiteSplineKnotU<F32>>::Iterator it=pCurveData->GetKnots().Begin();
	bool modified = false;

	F32 value = it->value;
	for (it; it != pCurveData->GetKnots().End(); ++it)
	{
		F32 delta = (value>it->value) ? 360.f : -360.f;
		while (Maths::Abs(value - it->value) > 180.f)
		{
			it->value += delta;
			modified = true;
		}
		value = it->value;
	}

	if (!modified)
		return EditorLib::E_Status_Success;
	else
	{
		Process();
		m_dirty = true;
		return m_pPropNode->XtSave();
	}
}

C_Status C_DgAnimationPropNode::AnimationCurve::InternalRemoveKeyFrame(U32 index)
{
	Btl::Vector<Curve::C_MayaHermiteSplineKnotU<F32> >::Iterator it = pCurveData->GetKnots().Begin();
	
	// Advance iterator to correct position
	for (U32 i=0; i<index; ++i, ++it);

	pCurveData->GetKnots().Erase(it);

	return E_Status_Success;
}

void C_DgAnimationPropNode::AnimationCurve::ParseMetadataString(const Btl::String& metadataString)
{
	if (metadataString == "X")
		metadata = E_AnimationCurve_Metadata_X;
	else if (metadataString == "Y")
		metadata = E_AnimationCurve_Metadata_Y;
	else if (metadataString == "Z")
		metadata = E_AnimationCurve_Metadata_Z;
	else
		metadata = E_AnimationCurve_Metadata_None;
}

bool C_DgAnimationPropNode::AnimationCurve::IsDirty()
{
	return m_dirty;
}

void C_DgAnimationPropNode::AnimationCurve::ClearDirty()
{
	m_dirty = false;
}

C_DgAnimationPropNode::Channel::Channel(C_DgAnimationPropNode* propNode, Object* object)
{
	name = "";
	hasNameField = false;
	m_pPropNode = propNode;
	m_pObject = object;
	m_deleted = false;
}

C_DgAnimationPropNode::Channel::~Channel()
{
	for (Btl::Vector<AnimationCurve*>::Iterator it=curves.Begin(); it != curves.End(); ++it)
	{
		SAFE_DELETE_PTR(*it);
	}
}

C_DgAnimationPropNode::Object* C_DgAnimationPropNode::Channel::GetObject()
{
	return m_pObject;
}

Btl::Vector<C_DgAnimationPropNode::AnimationCurve*>& C_DgAnimationPropNode::Channel::GetCurves()
{
	return curves;
}

const Btl::Vector<C_DgAnimationPropNode::AnimationCurve*>& C_DgAnimationPropNode::Channel::GetCurves() const
{
	return curves;
}

void C_DgAnimationPropNode::Channel::Delete(bool doDelete)
{
	m_deleted = doDelete;
}

bool C_DgAnimationPropNode::Channel::IsDeleted()
{
	return m_deleted;
}

void C_DgAnimationPropNode::Channel::ParseMetadataString(const Btl::String& metadataString)
{
	if (metadataString == "Translation")
		metadata = E_AnimationChannel_Metadata_Translation;
	else if (metadataString == "Rotation")
		metadata = E_AnimationChannel_Metadata_Rotation;
	else if (metadataString == "Time")
		metadata = E_AnimationChannel_Metadata_Time;
	else
		metadata = E_AnimationChannel_Metadata_None;
}

Btl::String C_DgAnimationPropNode::Channel::GetName()
{
	return name;
}

void C_DgAnimationPropNode::Channel::SetName(Btl::String name)
{
	this->name = name;
}

void C_DgAnimationPropNode::Channel::ParseName(Xt::DomComponent* pChannelComponent, LtStruct* pChannelType)
{
	hasNameField = false;

	for(LtStruct::FieldIterator it(&pChannelType->fields); it; ++it)
	{
		LarvaParam* fieldParam = it->attrs.FindParam("Animation", "field");

		if (!fieldParam || fieldParam->AsString() != "name")
			continue;

		hasNameField = true;
		
		Xt::DomField* nameField = Xt::AsField(Xt::At(&pChannelComponent->data, it));
		
		if (!nameField)
			continue;

		name = nameField->value.Text();
	}

	if (!hasNameField)
		name = pChannelType->label.Text();
}

void C_DgAnimationPropNode::Channel::XtDeserialise(LtStruct* pChannelType, Xt::DomComponent* pChannelComponent)
{
	LarvaParam* metadataParam = pChannelType->attrs.FindParam("Animation", "channel");
	if (metadataParam)
		ParseMetadataString(metadataParam->AsString().Text());

	ParseName(pChannelComponent, pChannelType);

	for(LtStruct::FieldIterator it(&pChannelType->fields); it; ++it)
	{
		LarvaParam* param = it->attrs.FindParam("SceneView", "animation");
		if (param && param->AsString() == "channel")
		{
			AnimationCurve* curve = 0;

			for (int j=0; j<curves.Size(); j++)
			{
				if (curves[j]->pField == it)
				{
					curve = curves[j];
					break;
				}
			}

			if (!curve)
			{
				curve = new AnimationCurve(m_pPropNode, this);
				curve->pField = &(*it);
				curve->name = it->label.Text();

				LarvaParam* colourParam = it->attrs.FindParam("SceneView", "animationcolour");
				if (colourParam)
				{
					curve->colour = Maths::RGBA(colourParam->AsColour());
				} else {
					curve->colour = Maths::RGBA(128, 128, 128, 0);
				}

				LarvaParam* metadataParam = it->attrs.FindParam("SceneView", "animationcurve");
				if (metadataParam)
					curve->ParseMetadataString(metadataParam->AsString().Text());

				curves.PushBack(curve);
			}
		}
	}

	for (int j=0; j<curves.Size(); j++)
	{
		curves[j]->pCurveData->GetKnots().Clear();

		Xt::DomBlock* pCurve = Xt::AsBlock(Xt::At(&pChannelComponent->data, curves[j]->pField));

		if (!pCurve)
			continue;

		Xt::DomArray* pKeyArray = Xt::AsArray(Xt::At(pCurve, "curve"));

		if (!pKeyArray)
			continue;

		for (size_t k=0; k<Xt::Count(pKeyArray); k++)
		{
			Xt::DomComponent* pKey = Xt::AsComponent(Xt::At(pKeyArray, k));

			if (!pKey)
				continue;

			F32 position, value, tangentIn, tangentOut;
			Xt::DomField *pField;
			
			pField = Xt::AsField(Xt::At(&pKey->data, "position"));
			XtFieldHelper::GetScalar(position, pField);

			pField = Xt::AsField(Xt::At(&pKey->data, "value"));
			XtFieldHelper::GetScalar(value, pField);

			pField = Xt::AsField(Xt::At(&pKey->data, "tangentIn"));
			XtFieldHelper::GetScalar(tangentIn, pField);

			pField = Xt::AsField(Xt::At(&pKey->data, "tangentOut"));
			XtFieldHelper::GetScalar(tangentOut, pField);

			Curve::C_MayaHermiteSplineKnotU<F32> p;
			p.tangentIn = tangentIn;
			p.tangentOut = tangentOut;
			p.u = position;
			p.value = value;
			p.tangentScale = 1.f;
			p.tangentInType = Curve::E_TangentType_Clamped;
			p.tangentOutType = Curve::E_TangentType_Clamped;
			curves[j]->pCurveData->GetKnots().PushBack(p);
		}

		curves[j]->Process();
	}
}

C_DgAnimationPropNode::Object::Object(C_DgAnimationPropNode* propNode)
{
	m_pPropNode = propNode;
	m_deleted = false;
}

C_DgAnimationPropNode::Object::~Object()
{
	for (Btl::Vector<Channel*>::Iterator it=m_Channels.Begin(); it != m_Channels.End(); ++it)
	{
		SAFE_DELETE_PTR(*it);
	}
	m_Channels.Clear();
}

I_DgNode* C_DgAnimationPropNode::Object::GetActorDgNode()
{
	return m_ActorDgNode;
}

const Btl::String& C_DgAnimationPropNode::Object::GetName() const
{
	return m_Name;
}

const C_DgAnimationPropNode* C_DgAnimationPropNode::Object::GetPropNode() const
{
	return m_pPropNode;
}

float C_DgAnimationPropNode::Object::GetTimelineValue(float timeline)
{
	for (Btl::Vector<Channel*>::Iterator channelIt = m_Channels.Begin(); channelIt != m_Channels.End(); ++channelIt)
	{
		if ((*channelIt)->metadata == C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Time)
			return (*channelIt)->GetCurves()[0]->Evaluate(timeline);
	}

	return timeline;
}

Btl::Vector<C_DgAnimationPropNode::Channel*>& C_DgAnimationPropNode::Object::GetChannels()
{
	return m_Channels;
}

const Btl::Vector<C_DgAnimationPropNode::Channel*>& C_DgAnimationPropNode::Object::GetChannels() const
{
	return m_Channels;
}

C_Status C_DgAnimationPropNode::Object::AddChannel(LtStruct* type)
{
	Channel* channel = new Channel(m_pPropNode, this);
	channel->xtType = type;
	channel->id = 0;

	LarvaParam* metadataParam = type->attrs.FindParam("SceneView", "animationchannel");
	if (metadataParam)
		channel->ParseMetadataString(metadataParam->AsString().Text());

	int j = 0;
	for(LtStruct::FieldIterator it(&type->fields); it; ++it)
	{
		LarvaParam* param = it->attrs.FindParam("SceneView", "animation");
		if (param && param->AsString() == "channel")
		{
			AnimationCurve* curve = new AnimationCurve(m_pPropNode, channel);

			curve->pField = &(*it);
			curve->name = it->label.Text();

			LarvaParam* colourParam = it->attrs.FindParam("SceneView", "animationcolour");
			if (colourParam)
			{
				curve->colour = Maths::RGBA(colourParam->AsColour());
			} else {
				curve->colour = Maths::RGBA(128, 128, 128, 0);
			}

			LarvaParam* metadataParam = it->attrs.FindParam("SceneView", "animationcurve");
			if (metadataParam)
				curve->ParseMetadataString(metadataParam->AsString().Text());

			channel->curves.PushBack(curve);
			 
			j++;
		}
	}

	m_Channels.PushBack(channel);

	return m_pPropNode->XtSave();
}

C_Status C_DgAnimationPropNode::Object::RemoveChannel(Channel* pChannel)
{
	for (Btl::Vector<Channel*>::Iterator channelIt = m_Channels.Begin(); channelIt != m_Channels.End(); ++channelIt)
	{
		if ((*channelIt) == pChannel)
		{
			(*channelIt)->Delete(true);
			return m_pPropNode->XtSave();
		}
	}

	return E_Status_Failure;
}

C_Status C_DgAnimationPropNode::Object::ChangeActor(XtId newActor)
{
	m_ActorId = newActor;

	return m_pPropNode->XtSave();
}

void C_DgAnimationPropNode::Object::Delete(bool doDelete)
{
	m_deleted = doDelete;
}

bool C_DgAnimationPropNode::Object::IsDeleted()
{
	return m_deleted;
}

void C_DgAnimationPropNode::Object::XtDeserialise(Xt::DomComponent* pObject)
{
	LarvaSchema *pSchema = EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema;
	LtStruct* animationObjectType = AsStruct(FindTypeByName(pSchema, "AnimationObject"));

	Xt::DomArray* pChannelArray = pObject ? Xt::AsArray(Xt::At(&pObject->data, "channels")) : 0;

	for (int channelLoop=0; channelLoop<m_Channels.Size(); channelLoop++)
	{
		m_Channels[channelLoop]->Delete(true);
	}

	for (size_t i=0; i<Xt::Count(pChannelArray); i++)
	{
		Xt::DomComponent* pChannel = Xt::AsComponent(Xt::At(pChannelArray, i));

		if (!pChannel)
			continue;

		LtStruct* channelType = AsStruct(FindTypeByName(pSchema, pChannel->type));

		if (!channelType)
			continue;

		Channel* channel = 0;

		for (int channelLoop=0; channelLoop<m_Channels.Size(); channelLoop++)
		{
			if (pChannel->id == m_Channels[channelLoop]->id)
			{
				channel = m_Channels[channelLoop];
				break;
			}
		}

		if (!channel)
		{
			channel = new Channel(m_pPropNode, this);
			channel->xtType = channelType;
			m_Channels.PushBack(channel);
		}

		channel->id = pChannel->id;

		channel->Delete(false);

		channel->XtDeserialise(channelType, pChannel);
	}

	for (int channelLoop=0; channelLoop<m_Channels.Size(); channelLoop++)
	{
		if (m_Channels[channelLoop]->IsDeleted())
		{
			delete m_Channels[channelLoop];
			m_Channels.EraseByIndex(channelLoop);
			channelLoop--;
		}
	}

	m_Name = EvaluateSummaryText(EditorLib::C_Core::Instance().GetXtManager().GetDb(), pSchema,
								 AsStruct(FindTypeByName(pSchema, "AnimationObject")), pObject, "Object").Text();

	Xt::DomRef* pRef = Xt::AsRef(FindBySemantic(animationObjectType, &pObject->data, "SceneView", "animation", "actor").dom);
	if (pRef)
	{
		m_ActorId = pRef->value;
		m_ActorDgNode = C_Core::Instance().GetDgManager().FindNodeById(EditorLib::C_NodeId(pRef->value, kNoChildId));
	} else {
		m_ActorId = 0;
		m_ActorDgNode = 0;
	}
}

void C_DgAnimationPropNode::Object::XtSave(Xt::DomComponent* pObject)
{
	LarvaSchema *pSchema = EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema;
	LtStruct* animationObjectType = AsStruct(FindTypeByName(pSchema, "AnimationObject"));

	Xt::DomArray* pChannels = Xt::AcquireArray(&pObject->data, "channels");

	// Erase removed channels
	for (size_t i=0; i<Xt::Count(pChannels);)
	{
		bool deleted = true;
		for (Btl::Vector<Channel*>::Iterator channel = m_Channels.Begin(); channel != m_Channels.End(); ++channel)
		{
			Xt::DomComponent* pComponent = Xt::AsComponent(Xt::At(pChannels, i));
			if((*channel)->id == pComponent->id)
				deleted = (*channel)->IsDeleted();
		}

		if (deleted)
			Xt::Remove(pChannels, i);
		else
			i++;
	}

	// Save existing channels
	for (int i=0; i<m_Channels.Size(); i++)
	{
		Xt::DomComponent* pChannel = 0;

		if (m_Channels[i]->id == 0)
		{
			pChannel = new Xt::DomComponent();
			GenerateIdForComponent(pChannel, false);
			SetType(pChannel, m_Channels[i]->xtType->name);

			Xt::Append(pChannels, pChannel);
			m_Channels[i]->id = pChannel->id;
		} else {
			for (size_t j=0; j<Xt::Count(pChannels); j++)
			{
				Xt::DomComponent* pComponent = Xt::AsComponent(Xt::At(pChannels, j));
				if (pComponent->id == m_Channels[i]->id)
				{
					pChannel = pComponent;
					break;
				}
			}
		}

		if (!pChannel)
			continue;

		for (int j=0; j<m_Channels[i]->curves.Size(); j++)
		{
			if (!m_Channels[i]->curves[j]->IsDirty())
				continue;

			Xt::DomBlock* pCurve = Xt::AcquireBlock(&pChannel->data, m_Channels[i]->curves[j]->pField);

			if (!pCurve)
				continue;

			Xt::DomArray* pCurveArray = Xt::AcquireArray(pCurve, "curve");

			if (!pCurveArray)
				continue;

			if ((size_t)m_Channels[i]->curves[j]->pCurveData->GetNumKnots()>Xt::Count(pCurveArray))
			{
				int itemsToAdd=m_Channels[i]->curves[j]->pCurveData->GetNumKnots()-Xt::Count(pCurveArray);
				for (int j=0; j<itemsToAdd; j++)
				{
					Xt::DomComponent* pXtItemComponent = new Xt::DomComponent;
					GenerateIdForComponent(pXtItemComponent, false);
					Xt::Insert(pCurveArray, 0, pXtItemComponent);
				}
			} else {
				int itemsToRemove = Xt::Count(pCurveArray)-m_Channels[i]->curves[j]->pCurveData->GetNumKnots();
				for (int j=0; j<itemsToRemove; j++)
				{
					Xt::Remove(pCurveArray, 0);
				}
			}

			for (int k=0; k<m_Channels[i]->curves[j]->pCurveData->GetNumKnots(); k++)
			{
				Xt::DomComponent* pKeyframe = Xt::AcquireComponent(pCurveArray,k);
				if (!pKeyframe)
					continue;

				Xt::DomBlock* pKeyframeBlock = &pKeyframe->data;

				Xt::DomField* pField;
				
				Curve::C_MayaHermiteSplineKnotU<F32>& keyFrame = m_Channels[i]->curves[j]->pCurveData->GetKnots()[k];

				pField = Xt::AcquireField(pKeyframeBlock, "position");
				EditorLib::XtFieldHelper::SetScalar(pField, keyFrame.u);

				pField = Xt::AcquireField(pKeyframeBlock, "value");
				EditorLib::XtFieldHelper::SetScalar(pField, keyFrame.value);

				pField = Xt::AcquireField(pKeyframeBlock, "tangentIn");
				EditorLib::XtFieldHelper::SetScalar(pField, keyFrame.tangentIn);

				pField = Xt::AcquireField(pKeyframeBlock, "tangentOut");
				EditorLib::XtFieldHelper::SetScalar(pField, keyFrame.tangentOut);
			}

			m_Channels[i]->curves[j]->ClearDirty();
		}

		if (m_Channels[i]->hasNameField)
		{
			for(LtStruct::FieldIterator it(&m_Channels[i]->xtType->fields); it; ++it)
			{
				LarvaParam* fieldParam = it->attrs.FindParam("Animation", "field");

				if (!fieldParam || fieldParam->AsString() != "name")
					continue;
				
				Xt::DomField* nameField = Xt::AcquireField(&pChannel->data, it);
				
				if (!nameField)
					continue;

				nameField->value = m_Channels[i]->GetName().Chars();	
			}
		}
	}

	Xt::DomRef* pRef = Xt::AsRef(AcquireBySemantic(animationObjectType, &pObject->data, "SceneView", "animation", "actor").dom);
	pRef->value = m_ActorId;
}

void C_DgAnimationPropNode::SetDefault()
{
	m_ClipLength = 2.0f;
	m_Quantisation = 30;
	
	for (Btl::Vector<Object*>::Iterator it=m_Objects.Begin(); it != m_Objects.End(); ++it)
	{
		SAFE_DELETE_PTR(*it);
	}
	m_Objects.Clear();
}

EditorLib::C_NodeId C_DgAnimationPropNode::GetAnimationId()
{
	Xt::DomComponent* pComponent = Xt::AsComponent(GetXtData());

	if (pComponent)
		return EditorLib::C_NodeId(pComponent->id, kNoChildId);
	else
		return EditorLib::C_NodeId();
}

void C_DgAnimationPropNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);

	GetXtDirty().SetDirty(pEvent->from);
}

void C_DgAnimationPropNode::UpdateCollisionSource()
{
}

void C_DgAnimationPropNode::UpdateCollisionInstance()
{
}

C_Status C_DgAnimationPropNode::XtSave()
{
	Xt::DomElement* pData = GetXtData();
	Xt::DomComponent* pAnimation(0);
	if (pData->kind == Xt::kDomRef)
	{
		 pAnimation = Xt::AsComponent(m_RefTargetMonitor.element);
	}
	else
		pAnimation = Xt::AsComponent(pData);

	if (!pAnimation)
		return EditorLib::E_Status_Failure;

	Xt::DomArray* pObjects = Xt::AcquireArray(&pAnimation->data, "Objects");

	Xt::DomComponent* pObject = 0;

	// Erase removed objects
	for (size_t i=0; i<Xt::Count(pObjects);)
	{
		bool deleted = true;
		for (Btl::Vector<Object*>::Iterator object = m_Objects.Begin(); object != m_Objects.End(); ++object)
		{
			Xt::DomComponent* pComponent = Xt::AsComponent(Xt::At(pObjects, i));
			if((*object)->id == pComponent->id)
			{
				deleted = (*object)->IsDeleted();
				break;
			}
		}

		if (deleted)
			Xt::Remove(pObjects, i);
		else
			i++;
	}

	for (Btl::Vector<Object*>::Iterator object=m_ObjectsToAdd.Begin(); object != m_ObjectsToAdd.End(); ++object)
	{
		Xt::DomComponent* pObject = new Xt::DomComponent();
		GenerateIdForComponent(pObject, false);

		Xt::Append(pObjects, pObject);
		(*object)->id = pObject->id;

		(*object)->XtSave(pObject);
		
		delete (*object);
	}
	m_ObjectsToAdd.Clear();

	// Save all objects
	for (Btl::Vector<Object*>::Iterator object=m_Objects.Begin(); object != m_Objects.End(); ++object)
	{
		if ((*object)->IsDeleted())
			continue;

		Xt::DomComponent* pObject = 0;

		if ((*object)->id == 0)
		{
			pObject = new Xt::DomComponent();
			GenerateIdForComponent(pObject, false);

			Xt::Append(pObjects, pObject);
			(*object)->id = pObject->id;
		} else {
			for (size_t j=0; j<Xt::Count(pObjects); j++)
			{
				Xt::DomComponent* pComponent = Xt::AsComponent(Xt::At(pObjects, j));
				if (pComponent->id == (*object)->id)
				{
					pObject = pComponent;
					break;
				}
			}
		}

		if (!pObject)
			assert(!"Object not set!");

		(*object)->XtSave(pObject);
	}

	Xt::DomField* pField;

	pField = Xt::AcquireField(&pAnimation->data, "Name");
	Xt::Set(pField, m_Name.Chars());

	return E_Status_Success;
}

C_Status C_DgAnimationPropNode::SetClipLength(F32 clipLength)
{
	m_ClipLength = clipLength;
	
	Xt::DomComponent* pAnimation = Xt::AsComponent(GetXtData());

	if (!pAnimation)
		return EditorLib::E_Status_Failure;

	Xt::DomField* pField = Xt::AcquireField(&pAnimation->data, "ClipLength");
	XtFieldHelper::SetScalar(pField, m_ClipLength);

	return XtSave();
}

F32 C_DgAnimationPropNode::GetClipLength()
{
	return m_ClipLength;
}

C_Status C_DgAnimationPropNode::SetQuantisation(U32 quantisation)
{
	m_Quantisation = quantisation;

	Xt::DomComponent* pAnimation = Xt::AsComponent(GetXtData());

	if (!pAnimation)
		return EditorLib::E_Status_Failure;

	Xt::DomField* pField = Xt::AcquireField(&pAnimation->data, "Quantisation");
	XtFieldHelper::SetScalar(pField, m_Quantisation);

	return EditorLib::E_Status_Success;
}

U32 C_DgAnimationPropNode::GetQuantisation()
{
	return m_Quantisation;
}

Btl::Vector<C_DgAnimationPropNode::Object*>& C_DgAnimationPropNode::GetObjects()
{
	return m_Objects;
}

const Btl::Vector<C_DgAnimationPropNode::Object*>& C_DgAnimationPropNode::GetObjects() const
{
	return m_Objects;
}


C_Status C_DgAnimationPropNode::AddObject()
{
	Object* object = new Object(this);
	object->id = 0;

	m_ObjectsToAdd.PushBack(object);
	
	return XtSave();
}

C_Status C_DgAnimationPropNode::RemoveObject(Object* pObject)
{
	pObject->Delete(true);
	return XtSave();
}

}

