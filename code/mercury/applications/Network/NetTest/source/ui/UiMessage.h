#ifndef UIMESSAGE_H
#define UIMESSAGE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <bdCore/bdCore.h>

namespace UI
{
	enum 
	{
		UI_MESSAGE_ID_UNKNOWN = -1,
	};

	enum 
	{
		PARAM_TYPE_NOT_SET,
		PARAM_TYPE_CHUNK,
		PARAM_TYPE_STRING,
		PARAM_TYPE_BOOLEAN,
		PARAM_TYPE_REAL,
		PARAM_TYPE_INTEGER,
	};


	// message sent from lua scripts and handled in the game
	BD_REFERENCE(Message);
	class Message: public bdReferencable
	{
	public:
		Message(int messageId = UI_MESSAGE_ID_UNKNOWN, bool confirmed=false)
		: m_messageId(messageId)
		, m_confirmed(confirmed)
		{}
		virtual ~Message()
		{}

		static UI::MessageRef create(const char* messageId, bool confirmed=false);
	
		int						messageId()			const { return m_messageId; }
		virtual const char*		messageType()		const=0;
		virtual void			logOutput()			const=0;
		virtual int				count	()			const=0;
		virtual int				length	(int index) const=0;
		virtual int				type	(int index) const=0;
		virtual int				integer	(int index) const=0;
		virtual bool			boolean	(int index) const=0;
		virtual float			real	(int index) const=0;
		virtual const char*		string	(int index) const=0;
		virtual const void*		chunk	(int index) const=0;
		virtual UI::MessageRef	message	(int index) const=0;
				
		virtual void pushMessage	(UI::MessageRef msg)=0;
		virtual void pushBool		(const bool value)=0;
		virtual void pushInteger	(const int value)=0;
		virtual void pushReal		(const float value)=0;
		virtual void pushChunk		(const void* ptr, const int len)=0;
		virtual void pushString		(const char* string)=0;
		
		// make a copy of the message
		virtual UI::MessageRef	copy() const=0;

	public:
		const int m_messageId;
		bool	  m_confirmed; // is this message been confirmed by a popup

	private:
		friend class MessageImp;
		virtual const class MessageParam* param(int index) const=0;
		virtual void pushParam(class MessageParam* param)=0;
	};

};
#endif //UIMESSAGE_H