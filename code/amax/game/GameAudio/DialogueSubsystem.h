
#ifndef DialogueSUBSYSTEM_H
#define DialogueSUBSYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "AppInterfaceLayer.h"

#define  mMaxNumDialogueEntites 64
	  
class DialogueSubsystem : public Audio::Subsystem,  public Messaging::C_MessageHandler
{
public:
		enum Message
	{
		E_Msg_HiBits = FOURCC_HI20('DASS'),

		eMoveActionIn,
		eToggleActionIn,
		eForwardsActionIn,
		eBackActionIn,
		eIn_horizontalActionIn,
		eIn_verticalActionIn,
		eOutActionIn,
		eLinkActionIn,
		eStartActionIn,
		eStopActionIn,
		eLoadEntites
	};

	enum DialogueSubsystemMessage
	{
		eAudioStopped = 0,
	

		TotalNumDialogueMessages
	};

	struct EntityInfo
	{

		AudioEntityHandle m_entityHandle;
		Audio::XtEntityProxy* m_entityProxy;
		bool					m_isActive;

	};

	DialogueSubsystem();
	~DialogueSubsystem() { }

	bool		ProcessSystemMessage(const U32 u32MessageId);
	bool		ProcessCustomMessage(const U32 u32MessageId, U32 u32Data = 0, void* pData = 0);

	bool		MessageHandler(const Messaging::C_MessageBase *pMessage );

	void		LoadEntity(int bankIndex);

	bool		SendStateTransition(const U32 u32OutgoingState, const U32 u32IncomingState)
	{
		return true;
	}
	void		Update();

	F32			GetVOPosition(){return m_VOPosition;}

private:
	AudioEntityHandle m_hEntity;
	//static const int mMaxNumDialogueEntites = 64;
	Btl::Vector< EntityInfo, Btl::VectorParameters_Array< mMaxNumDialogueEntites > > m_DialogueEntityPool;
	S32 m_listSize;
	U32 m_bankID;
	U32 m_sampleIndex;
	U32 m_delayFrames;
	bool m_playDialogue;
	U32 m_currentBankID;

	Audio::EntityDataMemberHandle m_VOPosParamOutHandle;
	Audio::ReturnDataHandle m_VOPosReturnDataHandle;

	F32 m_VOPosition;
	bool	m_VOActive;
};

#endif // DialogueSUBSYSTEM_H
