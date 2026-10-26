#ifndef ANIMATION_CONTROLLER_H
#define ANIMATION_CONTROLLER_H

#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/Components/Interfaces/I_AnimationController.h>
#include <shared_actors/Components/Creation/AnimationControllerCreate.h>

#include <shared_actors/XtLevelAnimation.h>

namespace Components
{


class C_AnimationControllerMessageItemBase
{
public:
	virtual bool Valid(void) const = 0;
	virtual bool Send(const Actor::C_Actor *pActorContext) const = 0;
	virtual F32	GetTValue(void) const = 0;

	bool operator<(const C_AnimationControllerMessageItemBase& other) const
	{
		if(Valid() && other.Valid())
			return GetTValue() < other.GetTValue(); 
		else
			return false;
	}
};

class C_AnimationControllerMessageChannelItem : public C_AnimationControllerMessageItemBase
{
public:
	C_AnimationControllerMessageChannelItem():m_pMessageItem(NULL){}
	C_AnimationControllerMessageChannelItem(const Level::C_XtAnimationMessageChannelItem	*pMessageItem):m_pMessageItem(pMessageItem){}

	virtual bool Valid(void) const { return m_pMessageItem != NULL;}
	virtual bool Send(const Actor::C_Actor *pActorContext) const
	{
		return m_pMessageItem->m_Message.Send(pActorContext);
	}
	virtual F32	GetTValue(void) const {return m_pMessageItem->m_TValue; }


	//const Level::C_XtAnimationMessageChannelItem	*MessageItem() const {return m_pMessageItem;}
private:
	const Level::C_XtAnimationMessageChannelItem	*m_pMessageItem;
};

// One for C_XtAnimationMessageItem
class C_AnimationControllerMessageItem : public C_AnimationControllerMessageItemBase
{
public:
	C_AnimationControllerMessageItem():m_pMessageItem(NULL){}
	C_AnimationControllerMessageItem(const Level::C_XtAnimationMessageItem	*pMessageItem, const Actors::C_GameActorCreate * dest):m_pMessageItem(pMessageItem),m_DestActor(dest){}

	virtual bool Valid(void) const { return m_pMessageItem != NULL;}
	virtual bool Send(const Actor::C_Actor *pActorContext) const;
	virtual F32	GetTValue(void) const {return m_pMessageItem->m_TValue; }


private:
	const Level::C_XtAnimationMessageItem	*m_pMessageItem;
	const Actors::C_GameActorCreate*		m_DestActor;
};

class C_AnimationMessageListItem
{
public:
	C_AnimationMessageListItem(C_AnimationControllerMessageItemBase *pItem):m_pItem(pItem){}
	//~C_AnimationMessageListItem(){SAFE_DELETE_PTR(m_pItem);}

	bool operator<(const C_AnimationMessageListItem& other) const
	{
		if(m_pItem && other.m_pItem)
		{
			if(m_pItem->Valid() && other.m_pItem->Valid())
				return m_pItem->GetTValue() < other.m_pItem->GetTValue(); 
			else
				return false;
		}
		return false;
	}
	bool Send(const Actor::C_Actor *pActorContext) const { return m_pItem->Send(pActorContext);}
	F32	GetTValue(void) const {return m_pItem->GetTValue(); }


public:
	C_AnimationControllerMessageItemBase *	m_pItem;

};

class C_AnimationController : public I_AnimationController
{
public:
	C_AnimationController();
	virtual ~C_AnimationController() {}


	static Component::C_ComponentTypeId		ClassType() {return Components::E_AnimationController;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_AnimationController;} // from Component::I_GameComponent

	const I_AnimationControllerCreator	*GetCreationData(void) const {return static_cast<const I_AnimationControllerCreator *>(GetCreator().GetData());}

	const Level::C_XtAnimationClip *	GetAnimationClip(void) const {return GetCreationData()->m_AnimationClip;}
	bool				OnMessage(const Messaging::C_MessageBase* pMessage);
#ifndef CFG_GOLD
	void				DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const OVERRIDE;
#endif

	bool				SendSlaveObjectsTValues(bool bMoveTo);

	//Serialisation
	virtual void		SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void		DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void		SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void		DeserialiseForSync(Sys::C_ContextualStream& stream);


	// Interface implementation
	virtual F32	GetAnimationLength() const;

	virtual const Level::C_AnimTValue	GetCurrentTValue(void) const {return m_TValue;}
	//virtual const Level::C_XtAnimationChannel*	GetObjectChannel(S32 objectID, U32 channelID) const;
	virtual const bool	GetObjectTranslation(const S32 objectID, Maths::C_Vertex &translation) const;
	virtual const bool	GetObjectLocalTranslation(const S32 objectID, Maths::C_Vertex &translation) const;
	virtual const bool	GetObjectRotation(const S32 objectID, Maths::C_Quaternion &rotation) const;
	virtual const bool	GetObjectLocalRotation(const S32 objectID, Maths::C_Quaternion &rotation) const;

	virtual const bool	GetCameraFOV(const S32 objectID, F32 &fValue) const;

	virtual const S32	GetNamedFloatCurveChannelID(const S32 objectID, const Util::C_String &name) const;
	virtual const bool	GetNamedFloatCurveValue(const S32 objectID, const S32 channelID, F32 &fValue) const;

	virtual const U32	GetNumberOfNamedCurves(const S32 objectID) const;
	virtual const char*	GetNamedCurveName(const S32 objectID, U32 index) const;

protected:
	virtual bool		CreateImpl(); // from Component::I_GameComponent
	virtual void		KillImpl(); // from Component::I_GameComponent

private:
	U32					GetMessageIndexFromTValue(F32 tValue) const;

	Level::C_AnimTValue		m_TValue;
	// for message items
	U32						m_CurrentMsgIndex;
	Btl::Vector<C_AnimationControllerMessageChannelItem>	m_MessageItems;
	Btl::Vector<C_AnimationMessageListItem>			m_NewMessageItems;
};

}//namespace Components

#endif