#ifndef XT_LEVEL_ANIMATION_H
#define XT_LEVEL_ANIMATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xt/XtGame.h>
#include <misc_shared/UID/FourCC.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtActorMessages.h>

// forward decs to external types
namespace Actors
{
	class C_GameActorCreate;
};
namespace ActorMsg
{
	class C_XtActorMessageBase;
}

namespace Curve
{
	class C_Hermite1DData;
	class C_Hermite1D;
}

namespace DbUtil
{
	class I_DebugRender;
};

namespace Level
{
typedef F32 C_AnimTValue;

// forward decs
class C_XtAnimationCurveKey;
class C_XtAnimationObject;
class C_XtAnimationChannel;
class C_XtNamedFloatAnimationChannel;
class C_XtAnimationMessageChannel;

class C_XtAnimationClip : public Level::C_GameData
{
public:
	const C_XtAnimationObject *GetAnimationObject(S32 objectID) const;
	const C_XtAnimationChannel*	GetObjectChannel(S32 objectID, U32 channelID) const;
	const bool	GetObjectTranslation(S32 objectID, C_AnimTValue tValue, Maths::C_Vertex &translation) const;
	const bool	GetObjectRotation(S32 objectID,  C_AnimTValue tValue,Maths::C_Quaternion &rotation) const;
	const bool	GetCameraFOV(S32 objectID,  C_AnimTValue tValue,F32 &fValue) const;
	const bool	GetNamedFloatCurveValue(S32 objectID,  C_AnimTValue tValue, const S32 channelID, F32 &fValue) const;
	const bool	GetObjectSlaveTvalue(S32 objectID,  C_AnimTValue tValueIn,C_AnimTValue &tValueOut) const;
	void DebugRender(DbUtil::I_DebugRender* debugRenderPtr, const Maths::HMat43& actorMatrix) const;

	// temp functions
	const C_XtAnimationMessageChannel *	GetMessageChannel(void) const;
public:
	F32								m_ClipLength;

	XtArray<C_XtAnimationObject>	m_AnimationObjects;
};

// New Message Type 
// ---------------

struct C_XtAnimationMessageItem
{
	F32									m_TValue;
	ActorMsg::C_XtActorMessageBase *	m_Message;
};

class C_XtAnimationObject
{
public:
	const C_XtAnimationChannel *	GetAnimationChannel(const U32 channelID) const;

	const U32								GetNumNamedFloatAnimationChannels(void) const;
	const C_XtNamedFloatAnimationChannel *	GetNamedFloatAnimationChannelFromIndex(U32 index) const;

	const S32						GetNamedFloatAnimationChannelID(const Util::C_String &name) const;
	const C_XtNamedFloatAnimationChannel *	GetNamedFloatAnimationChannel(const Util::C_String &name) const;
	const C_XtNamedFloatAnimationChannel *	GetNamedFloatAnimationChannel(const S32 channelID) const;

	const bool			GetTranslationAtTValue(const C_AnimTValue tValue, Maths::C_Vertex &translation) const;
	const bool			GetRotationAtTValue(const C_AnimTValue tValue, Maths::C_Quaternion &rotation) const;
	const bool			GetCameraFOVAtTValue(const C_AnimTValue tValue, F32 &fValue) const;
	const bool			GetNamedFloatValueAtTValue(const C_AnimTValue tValue,const S32 channelID, F32 &fValue) const;
	const bool			GetSlaveTValueAtTValue(const C_AnimTValue tValueIn, C_AnimTValue &tValueOut) const;
public:
	Actors::C_GameActorCreate*		m_Actor;
	XtArray<C_XtAnimationMessageItem>	m_AnimationMessages;
	XtArray<C_XtAnimationChannel *>	m_AnimationChannels;	

	mutable Btl::Vector<U32>	m_NameFloatChannelIndices;
};

class C_XtAnimationCurve
{
public:
	C_XtAnimationCurve();
	virtual ~C_XtAnimationCurve();
	const F32		GetValueFromT(C_AnimTValue TValue) const;

public:
	XtArray<C_XtAnimationCurveKey>	m_Curve;

private:
	Curve::C_Hermite1DData* m_CurveData;
	mutable bool m_Imported;
};

class C_XtAnimationCurveKey
{
public:

public:
	F32		m_Value;
	F32		m_Position;
	F32		m_TangentIn;
	F32		m_TangentOut;
};

enum E_AnimationChannelID
{
	E_AnimationChannelID_None,
	E_AnimationChannelID_Translation = FOURCC_HI20('LACI'),
	E_AnimationChannelID_Rotation,
	E_AnimationChannelID_CameraFOV,
	E_AnimationChannelID_NamedFloat,
	E_AnimationChannelID_SlaveTValue,
	E_AnimationChannelID_Message,
};

// Animation Channel Base class
class C_XtAnimationChannel
{
public:
	C_XtAnimationChannel():m_ChannelID(E_AnimationChannelID_None){}

	U32 m_ChannelID; // 'POS_'  GetId<C_XtPositionAnimationChannel>()
};

class C_XtTranslationAnimationChannel : public C_XtAnimationChannel
{
public:

	C_XtTranslationAnimationChannel()
	{
		m_ChannelID = E_AnimationChannelID_Translation;
	}
	const Maths::HVec3	GetTranslationFromTValue(C_AnimTValue TValue) const;
public:
	C_XtAnimationCurve	m_XPosition;
	C_XtAnimationCurve	m_YPosition;
	C_XtAnimationCurve	m_ZPosition;
};



class C_XtRotationAnimationChannel : public C_XtAnimationChannel
{
public:
	enum E_RotationOrder
	{
		E_RotationOrder_XYZ = 0,
		E_RotationOrder_XZY,
		E_RotationOrder_YXZ,
		E_RotationOrder_YZX,
		E_RotationOrder_ZXY,
		E_RotationOrder_ZYX,
	};

	C_XtRotationAnimationChannel()
	{
		m_ChannelID = E_AnimationChannelID_Rotation;
	}
	const Maths::Quaternion	GetRotationFromTValue(C_AnimTValue TValue) const;
public:
	E_RotationOrder		m_RotationOrder;

	C_XtAnimationCurve	m_XRotation;
	C_XtAnimationCurve	m_YRotation;
	C_XtAnimationCurve	m_ZRotation;

};

class C_XtFovAnimationChannel : public C_XtAnimationChannel
{
public:
	C_XtFovAnimationChannel()
	{
		m_ChannelID = E_AnimationChannelID_CameraFOV;
	}

	const F32		GetFOVFromTValue(C_AnimTValue tValue) const {return m_FieldOfView.GetValueFromT(tValue);}
public:
	C_XtAnimationCurve	m_FieldOfView;

};

class C_XtNamedFloatAnimationChannel : public C_XtAnimationChannel
{
public:
	C_XtNamedFloatAnimationChannel()
	{
		m_ChannelID = E_AnimationChannelID_NamedFloat;
	}
public:
	XtSz8				m_Name;
	C_XtAnimationCurve	m_FloatCurve;

};

// Channel type for animating TValues that are sent to slave animation controllers
class C_XtSlaveTValueAnimationChannel : public C_XtAnimationChannel
{
public:
	C_XtSlaveTValueAnimationChannel()
	{
		m_ChannelID = E_AnimationChannelID_SlaveTValue;
	}
public:
	C_XtAnimationCurve	m_TValueCurve;

};

// Message Channel 
// ---------------

struct C_XtAnimationMessageChannelItem
{
	F32							m_TValue;
	ActorMsg::C_XtSendMessage	m_Message;
};

class C_XtAnimationMessageChannel : public C_XtAnimationChannel
{
public:
	C_XtAnimationMessageChannel()
	{
		m_ChannelID = E_AnimationChannelID_Message;
	}
public:
	XtArray<C_XtAnimationMessageChannelItem>	m_Messages;

};

}//namespace Level

#endif