#include "SharedActorsPch.h"

#include <shared_actors/XtLevelAnimation.h>

#include <btl/containers/vector.h>

#include <core/UtMaths.h>

#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/HermiteSpline.h>
#include <system/debug/I_DebugRender.h>

namespace Level
{
using namespace Maths;

// ========================= C_XtAnimationCurve =========================

C_XtAnimationCurve::C_XtAnimationCurve()
{
	//m_CurveData = (Curve::C_Hermite1DData *)0xdeadbeef;
	m_CurveData = new Curve::C_Hermite1DData;
	m_Imported = false;
}

C_XtAnimationCurve::~C_XtAnimationCurve()
{
	SAFE_DELETE_PTR(m_CurveData)
}

// Convert Sorcery data into C_Hermite1D
static void ImportCurve(const XtArray<C_XtAnimationCurveKey>& keyframes, Curve::C_Hermite1DData* curve)
{
	if (!curve)
		return;
		
	Btl::Vector<typename Curve::C_Hermite1DData::KnotType >& samples = curve->GetKnots();
	samples.Clear();
	
	samples.Resize(keyframes.count);
	for (S32 i = 0; i < keyframes.count; i++)
	{
		const C_XtAnimationCurveKey& src = keyframes[i];
		Curve::C_Hermite1DData::KnotType& dst = samples[i];
	
		dst.u = src.m_Position;
		dst.value = src.m_Value;
		dst.tangentIn = src.m_TangentIn;
		dst.tangentOut = src.m_TangentOut;
		dst.tangentScale = 1.f;
	}
}

const F32 C_XtAnimationCurve::GetValueFromT(C_AnimTValue TValue) const
{
	if (!m_Imported)
	{
		ImportCurve(m_Curve, m_CurveData);
		m_Imported = true;
	}
	Curve::C_Hermite1D evaluator;
	evaluator.SetData(m_CurveData);
	return evaluator.Evaluate(TValue);
}

// ========================= C_XtTranslationAnimationChannel =========================
const Maths::HVec3 C_XtTranslationAnimationChannel::GetTranslationFromTValue(C_AnimTValue TValue) const
{
	BIZ_ASSERT(m_ChannelID == E_AnimationChannelID_Translation);
	F32 x = m_XPosition.GetValueFromT(TValue);
	F32 y = m_YPosition.GetValueFromT(TValue);
	F32 z = m_ZPosition.GetValueFromT(TValue);
	return Maths::HVec3(x, y, z);
}

// ========================= C_XtRotationAnimationChannel =========================
const Maths::Quaternion C_XtRotationAnimationChannel::GetRotationFromTValue(C_AnimTValue TValue) const
{
	BIZ_ASSERT(m_ChannelID == E_AnimationChannelID_Rotation);
	F32 rx = m_XRotation.GetValueFromT(TValue);
	F32 ry = m_YRotation.GetValueFromT(TValue);
	F32 rz = m_ZRotation.GetValueFromT(TValue);

	rx = Maths::DegToRad(rx);
	ry = Maths::DegToRad(ry);
	rz = Maths::DegToRad(rz);
	
	Maths::C_Quaternion orient = Maths::C_Quaternion(0.f, 0.f, sin(rz * 0.5f), cos(rz * 0.5f));
	orient = orient * Maths::C_Quaternion(sin(rx * 0.5f), 0.f, 0.f, cos(rx * 0.5f));
	orient = orient * Maths::C_Quaternion(0.f, sin(ry * 0.5f), 0.f, cos(ry * 0.5f));

	return orient;
}

// ================================================================================

// Animation Clip Methods
const C_XtAnimationObject *C_XtAnimationClip::GetAnimationObject(S32 objectID) const
{
	if(objectID >= 0 && objectID < m_AnimationObjects.count)
		return &m_AnimationObjects[objectID];
	else
		return NULL;
}

const C_XtAnimationChannel*	C_XtAnimationClip::GetObjectChannel(S32 objectID, U32 channelID) const
{
	if(objectID >= 0 && objectID < m_AnimationObjects.count)
		return m_AnimationObjects[objectID].GetAnimationChannel(channelID);
	else
		return NULL;
}

const bool	C_XtAnimationClip::GetObjectTranslation(S32 objectID, C_AnimTValue tValue, Maths::C_Vertex &translation) const
{
	if(objectID >= 0 && objectID < m_AnimationObjects.count)
		return m_AnimationObjects[objectID].GetTranslationAtTValue(tValue,translation);
	else
		return false;	
}
const bool	C_XtAnimationClip::GetObjectRotation(S32 objectID,  C_AnimTValue tValue,Maths::C_Quaternion &rotation) const
{
	if(objectID >= 0 && objectID < m_AnimationObjects.count)
		return m_AnimationObjects[objectID].GetRotationAtTValue(tValue, rotation);
	else
		return false;	

}

const bool	C_XtAnimationClip::GetCameraFOV(S32 objectID,  C_AnimTValue tValue,F32 &fovValue) const
{
	if(objectID >= 0 && objectID < m_AnimationObjects.count)
		return m_AnimationObjects[objectID].GetCameraFOVAtTValue(tValue,fovValue);
	else
		return false;	

}

const bool	C_XtAnimationClip::GetNamedFloatCurveValue(S32 objectID,  C_AnimTValue tValue, const S32 channelID, F32 &fValue) const
{
	if(objectID >= 0 && objectID < m_AnimationObjects.count)
	{
		Level::C_XtNamedFloatAnimationChannel *pNamedFloatChannel = (Level::C_XtNamedFloatAnimationChannel *)m_AnimationObjects[objectID].GetNamedFloatAnimationChannel(channelID);

		if(pNamedFloatChannel)
		{
			pNamedFloatChannel->m_FloatCurve.GetValueFromT(tValue);
			return true;
		}
		else
			return false;
	}
	else
		return false;	
}

const bool	C_XtAnimationClip::GetObjectSlaveTvalue(S32 objectID,  C_AnimTValue tValueIn,C_AnimTValue &tValueOut) const
{
	if(objectID >= 0 && objectID < m_AnimationObjects.count)
		return m_AnimationObjects[objectID].GetSlaveTValueAtTValue(tValueIn, tValueOut);
	else
		return false;	

}

void C_XtAnimationClip::DebugRender(DbUtil::I_DebugRender* debugRenderPtr, const HMat43& actorMatrix) const
{
	C_Vertex oldPos, pos;

	for (S32 objectNumber = 0; objectNumber < m_AnimationObjects.count; objectNumber++)
	{
		const C_XtAnimationObject& object = m_AnimationObjects[objectNumber];

		for (F32 i = 0.f; i < m_ClipLength + 0.01f; i+= 0.02f)
		{
			object.GetTranslationAtTValue(i, pos);
			pos = pos * actorMatrix;
			
			if (i > 0.f)
				debugRenderPtr->Draw3dLine(oldPos, pos, RGBA(0xffe0e0e0));

			oldPos = pos;
		}

		// Draw snapshots:
		for (F32 i = 0.0f; i < m_ClipLength + 0.05f; i+= 0.1f)
		{
			C_Quaternion orient;
			object.GetTranslationAtTValue(i, pos);
			object.GetRotationAtTValue(i, orient);

			HMat33 rot(orient);
			HMat43 mat(rot, pos);
			mat = mat * actorMatrix;
			debugRenderPtr->Draw3dLine(HVec3(-0.5f, -0.2f, -1.f) * mat, HVec3(-0.5f, -0.2f,  1.f) * mat, RGBA(0xffc0c0c0));
			debugRenderPtr->Draw3dLine(HVec3( 0.5f, -0.2f, -1.f) * mat, HVec3( 0.5f, -0.2f,  1.f) * mat, RGBA(0xffc0c0c0));
			debugRenderPtr->Draw3dLine(HVec3(-0.5f, -0.2f, -1.f) * mat, HVec3( 0.5f, -0.2f, -1.f) * mat, RGBA(0xffc0c0c0));
			debugRenderPtr->Draw3dLine(HVec3(-0.5f, -0.2f,  1.f) * mat, HVec3( 0.5f, -0.2f,  1.f) * mat, RGBA(0xffc0c0c0));
			debugRenderPtr->Draw3dLine(HVec3(-0.5f,  0.2f, -1.f) * mat, HVec3(-0.5f,  0.2f,  1.f) * mat, RGBA(0xffc0c0c0));
			debugRenderPtr->Draw3dLine(HVec3( 0.5f,  0.2f, -1.f) * mat, HVec3( 0.5f,  0.2f,  1.f) * mat, RGBA(0xffc0c0c0));
			debugRenderPtr->Draw3dLine(HVec3(-0.5f,  0.2f, -1.f) * mat, HVec3( 0.5f,  0.2f, -1.f) * mat, RGBA(0xffc0c0c0));
			debugRenderPtr->Draw3dLine(HVec3(-0.5f,  0.2f,  1.f) * mat, HVec3( 0.5f,  0.2f,  1.f) * mat, RGBA(0xffc0c0c0));

			oldPos = pos;
		}
	}
}

// temp method
const C_XtAnimationMessageChannel *	C_XtAnimationClip::GetMessageChannel(void) const
{
	for (S32 objectNumber = 0; objectNumber < m_AnimationObjects.count; objectNumber++)
	{
		const C_XtAnimationMessageChannel * pMessageChannel = (const C_XtAnimationMessageChannel * )m_AnimationObjects[objectNumber].GetAnimationChannel(E_AnimationChannelID_Message);
		if(pMessageChannel)
			return pMessageChannel;
	}

	return NULL;
}


// Animation Object Methods
const C_XtAnimationChannel *	C_XtAnimationObject::GetAnimationChannel(const U32 channelID) const
{
	for(XtI32 channelNo = 0;channelNo < m_AnimationChannels.count; channelNo++)
	{
		const C_XtAnimationChannel *pChannel = m_AnimationChannels[channelNo];

		if(pChannel && pChannel->m_ChannelID == channelID)
			return pChannel;
	}

	return NULL;
}

const U32 C_XtAnimationObject::GetNumNamedFloatAnimationChannels(void) const
{
	if(m_NameFloatChannelIndices.Size() == 0)
	{
		for(XtI32 channelNo = 0;channelNo < m_AnimationChannels.count; channelNo++)
		{
			const C_XtAnimationChannel *pChannel = m_AnimationChannels[channelNo];

			if(pChannel && pChannel->m_ChannelID == E_AnimationChannelID_NamedFloat)
			{
				m_NameFloatChannelIndices.PushBack((U32)channelNo);
			}
		}
	}

	return m_NameFloatChannelIndices.Size();
}

const C_XtNamedFloatAnimationChannel *	C_XtAnimationObject::GetNamedFloatAnimationChannelFromIndex(U32 index) const
{
	U32 channelIndex = m_NameFloatChannelIndices[index];
	const C_XtAnimationChannel *pChannel = m_AnimationChannels[channelIndex];

	if(pChannel && pChannel->m_ChannelID == E_AnimationChannelID_NamedFloat)
		return (C_XtNamedFloatAnimationChannel *)pChannel;

	return NULL;
}

const S32 C_XtAnimationObject::GetNamedFloatAnimationChannelID(const Util::C_String &name) const 
{
	for(XtI32 channelNo = 0;channelNo < m_AnimationChannels.count; channelNo++)
	{
		const C_XtAnimationChannel *pChannel = m_AnimationChannels[channelNo];

		if(pChannel && pChannel->m_ChannelID == E_AnimationChannelID_NamedFloat)
		{
			const C_XtNamedFloatAnimationChannel *pNamedFloatChannel = (const C_XtNamedFloatAnimationChannel *)pChannel;

			if(Util::C_String(pNamedFloatChannel->m_Name) == name)
				return (S32)channelNo;
		}
	}

	return -1;
}

const C_XtNamedFloatAnimationChannel *	C_XtAnimationObject::GetNamedFloatAnimationChannel(const Util::C_String &name) const 
{
	S32 channelID = GetNamedFloatAnimationChannelID(name);
	return GetNamedFloatAnimationChannel(channelID);
}

const C_XtNamedFloatAnimationChannel *	C_XtAnimationObject::GetNamedFloatAnimationChannel(const S32 channelID) const 
{
	if(channelID == -1)
		return NULL;

	const C_XtAnimationChannel *pChannel = m_AnimationChannels[channelID];

	if(pChannel)
		return (const C_XtNamedFloatAnimationChannel *)pChannel;

	return NULL;
}

const bool C_XtAnimationObject::GetTranslationAtTValue(const C_AnimTValue tValue, Maths::C_Vertex &translation) const
{
	const C_XtTranslationAnimationChannel *pTranslationChannel = (const C_XtTranslationAnimationChannel *)GetAnimationChannel(E_AnimationChannelID_Translation);

	if(pTranslationChannel)
	{
		translation = pTranslationChannel->GetTranslationFromTValue(tValue);
		return true;
	}
	return false;
}

const bool C_XtAnimationObject::GetRotationAtTValue(const C_AnimTValue tValue, Maths::C_Quaternion &rotation) const
{
	const C_XtRotationAnimationChannel *pRotationChannel = (const C_XtRotationAnimationChannel *)GetAnimationChannel(E_AnimationChannelID_Rotation);

	if(pRotationChannel)
	{
		rotation = pRotationChannel->GetRotationFromTValue(tValue);
		return true;
	}
	return false;
}

const bool C_XtAnimationObject::GetCameraFOVAtTValue(const C_AnimTValue tValue, F32 &fValue) const
{
	const C_XtFovAnimationChannel *pFovChannel = (const C_XtFovAnimationChannel *)GetAnimationChannel(E_AnimationChannelID_CameraFOV);

	if(pFovChannel)
	{
		fValue = pFovChannel->GetFOVFromTValue(tValue);
		return true;
	}
	return false;
}

const bool C_XtAnimationObject::GetNamedFloatValueAtTValue(const C_AnimTValue tValue,const S32 channelID, F32 &fValue) const
{
	const C_XtNamedFloatAnimationChannel *pNameFloatChannel = (const C_XtNamedFloatAnimationChannel *)GetNamedFloatAnimationChannel(channelID);

	if(pNameFloatChannel)
	{
		fValue = pNameFloatChannel->m_FloatCurve.GetValueFromT(tValue);
		return true;
	}
	return false;
}

// Get a Slave Tvalue from the Slave Tvalue channel for a given Tvalue
const bool C_XtAnimationObject::GetSlaveTValueAtTValue(const C_AnimTValue tValueIn, C_AnimTValue &tValueOut) const
{
	const C_XtSlaveTValueAnimationChannel *pTValueChannel = (const C_XtSlaveTValueAnimationChannel *)GetAnimationChannel(E_AnimationChannelID_SlaveTValue);

	if(pTValueChannel)
	{
		tValueOut = pTValueChannel->m_TValueCurve.GetValueFromT(tValueIn);
		return true;
	}
	return false;
}


}

XT_TYPE(Level::C_XtAnimationClip, "AnimationClip")
	XT_BASE(Level::C_GameData)
	XT_FIELD(m_ClipLength, "ClipLength")
	XT_FIELD(m_AnimationObjects, "Objects")
XT_END_TYPE

XT_TYPE(Level::C_XtAnimationMessageItem, "AnimationMessage")
	XT_FIELD(m_TValue,	"tvalue")
	XT_FIELD(m_Message,	"message")
XT_END_TYPE

XT_TYPE(Level::C_XtAnimationObject, "AnimationObject")
	XT_FIELD(m_Actor, "actor")
	XT_FIELD(m_AnimationMessages, "messages")
	XT_FIELD(m_AnimationChannels, "channels")	
XT_END_TYPE


XT_TYPE(Level::C_XtAnimationCurve, "AnimationCurve")
	XT_FIELD(m_Curve, "curve")
XT_END_TYPE

XT_TYPE(Level::C_XtAnimationCurveKey, "AnimationCurveKey")
	XT_FIELD(m_Value, "value")
	XT_FIELD(m_Position, "position")
	XT_FIELD(m_TangentIn, "tangentIn")
	XT_FIELD(m_TangentOut, "tangentOut")
XT_END_TYPE

// Animation Channel Base class
XT_ABSTRACT_TYPE(Level::C_XtAnimationChannel, "AnimationChannel")
XT_END_TYPE

XT_TYPE(Level::C_XtTranslationAnimationChannel, "TranslationAnimationChannel")
	XT_BASE(Level::C_XtAnimationChannel)
	XT_FIELD(m_XPosition, "X")
	XT_FIELD(m_YPosition, "Y")
	XT_FIELD(m_ZPosition, "Z")
XT_END_TYPE


XT_TYPE( Level::C_XtRotationAnimationChannel::E_RotationOrder, "AnimationRotationOrder" )
	XT_OPTION(Level::C_XtRotationAnimationChannel::E_RotationOrder_XYZ, "XYZ")
	XT_OPTION(Level::C_XtRotationAnimationChannel::E_RotationOrder_XZY, "XZY")
	XT_OPTION(Level::C_XtRotationAnimationChannel::E_RotationOrder_YXZ, "YXZ")
	XT_OPTION(Level::C_XtRotationAnimationChannel::E_RotationOrder_YZX, "YZX")
	XT_OPTION(Level::C_XtRotationAnimationChannel::E_RotationOrder_ZXY, "ZXY")
	XT_OPTION(Level::C_XtRotationAnimationChannel::E_RotationOrder_ZYX, "ZYX")
XT_END_TYPE

XT_TYPE(Level::C_XtRotationAnimationChannel, "RotationAnimationChannel")
	XT_BASE(Level::C_XtAnimationChannel)
	XT_FIELD(m_RotationOrder, "RotationOrder")
	XT_FIELD(m_XRotation, "X")
	XT_FIELD(m_YRotation, "Y")
	XT_FIELD(m_ZRotation, "Z")
XT_END_TYPE

XT_TYPE(Level::C_XtFovAnimationChannel, "FovAnimationChannel")
	XT_BASE(Level::C_XtAnimationChannel)
	XT_FIELD(m_FieldOfView,"FieldOfView")
XT_END_TYPE

XT_TYPE(Level::C_XtNamedFloatAnimationChannel, "NamedFloatAnimationChannel")
	XT_BASE(Level::C_XtAnimationChannel)
	XT_FIELD(m_Name,"Name")
	XT_FIELD(m_FloatCurve,"FloatCurve")
XT_END_TYPE

XT_TYPE(Level::C_XtSlaveTValueAnimationChannel, "SlaveTValueAnimationChannel")
XT_BASE(Level::C_XtAnimationChannel)
XT_FIELD(m_TValueCurve,"TValueCurve")
XT_END_TYPE

XT_TYPE(Level::C_XtAnimationMessageChannelItem, "AnimationMessageChannelItem")
	XT_FIELD(m_TValue,"tvalue")
	XT_FIELD(m_Message,"message")
XT_END_TYPE

XT_TYPE(Level::C_XtAnimationMessageChannel, "AnimationMessageChannel")
	XT_BASE(Level::C_XtAnimationChannel)
	XT_FIELD(m_Messages, "messages")
XT_END_TYPE

XT_LIBRARY(LevelAnimation)
XT_REGISTER_TYPE(Level::C_XtAnimationClip)
XT_REGISTER_TYPE(Level::C_XtAnimationMessageItem)
XT_REGISTER_TYPE(Level::C_XtAnimationObject)
XT_REGISTER_TYPE(Level::C_XtAnimationCurve)
XT_REGISTER_TYPE(Level::C_XtAnimationCurveKey)
XT_REGISTER_TYPE(Level::C_XtAnimationChannel)
XT_REGISTER_TYPE(Level::C_XtTranslationAnimationChannel)
XT_REGISTER_TYPE(Level::C_XtRotationAnimationChannel::E_RotationOrder)
XT_REGISTER_TYPE(Level::C_XtRotationAnimationChannel)
XT_REGISTER_TYPE(Level::C_XtFovAnimationChannel)
XT_REGISTER_TYPE(Level::C_XtNamedFloatAnimationChannel)
XT_REGISTER_TYPE(Level::C_XtSlaveTValueAnimationChannel)
XT_REGISTER_TYPE(Level::C_XtAnimationMessageChannelItem)
XT_REGISTER_TYPE(Level::C_XtAnimationMessageChannel)
XT_END_LIBRARY
